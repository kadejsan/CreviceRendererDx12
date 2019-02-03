#include "stdafx.h"
#include "AmbientOcclusion.h"
#include "MathHelper.h"
#include "Renderer.h"
#include "UIContext.h"

/// Sample kernel for ambient occlusion. The requirements are that:
/// - Sample positions fall within the unit hemisphere oriented
///   toward positive z axis.
/// - Sample positions are more densely clustered towards the origin.
///   This effectively attenuates the occlusion contribution
///   according to distance from the sample kernel centre (samples closer
///   to a point occlude it more than samples further away).
///
/// @param sampleKernelSize Size of the sample kernel to generate
/// @param sampleKernel Output sample kernel list
///
void GenerateSampleKernel(const std::uint32_t sampleKernelSize, std::vector<float3>& sampleKernel)
{
	sampleKernel.reserve(sampleKernelSize);

	const float sampleKernelSizeFloat = static_cast<float>(sampleKernelSize);

	XMVECTOR vec;

	for (std::uint32_t i = 0U; i < sampleKernelSize; ++i)
	{
		const float x = MathHelper::RandF(-1.0f, 1.0f);
		const float y = MathHelper::RandF(-1.0f, 1.0f);
		const float z = MathHelper::RandF(0.0f, 1.0f);

		sampleKernel.push_back(float3(x, y, z));
		float3& currentSample = sampleKernel.back();
		vec = XMLoadFloat3(&currentSample);
		vec = XMVector3Normalize(vec);
		XMStoreFloat3(&currentSample, vec);

		// Accelerating interpolation function to falloff 
		// from the distance from the origin.
		float scale = i / sampleKernelSizeFloat;
		scale = MathHelper::Lerp(0.1f, 1.0f, scale * scale);
		vec = XMVectorScale(vec, scale);
		XMStoreFloat3(&currentSample, vec);
	}
}

/// Generate a set of random values used to rotate the sample kernel,
/// which will effectively increase the sample count and minimize 
/// the 'banding' artifacts.
///
/// @param numSamples Number of samples to generate
/// @param noiseVector Output noise vector
void GenerateNoise(const std::uint32_t numSamples, std::vector<float3>& noiseVector)
{
	noiseVector.reserve(numSamples);

	XMVECTOR vec;

	for (std::uint32_t i = 0U; i < numSamples; ++i) 
	{
		const float x = MathHelper::RandF(-1.0f, 1.0f);
		const float y = MathHelper::RandF(-1.0f, 1.0f);

		// The z component must zero. Since our kernel is oriented along the z-axis, 
		// we want the random rotation to occur around that axis.
		const float z = 0.0f;
		noiseVector.push_back(float3(x, y, z));
		float3& currentSample = noiseVector.back();
		vec = XMLoadFloat3(&currentSample);
		vec = XMVector3Normalize(vec);
		XMStoreFloat3(&currentSample, vec);

		// Map from [-1.0f, 1.0f] to [0.0f, 1.0f] because
		// this is going to be stored in a texture
		currentSample.x = currentSample.x * 0.5f + 0.5f;
		currentSample.y = currentSample.y * 0.5f + 0.5f;
		currentSample.z = currentSample.z * 0.5f + 0.5f;
	}
}

AmbientOcclusion::AmbientOcclusion()
	: m_sampleKernelSize(32U)
	, m_noiseTextureDimension(64U)
	, m_sampleKernel(nullptr)
	, m_noiseTexture(nullptr)
	, m_vsCB(nullptr)
	, m_psCB(nullptr)
{

}

AmbientOcclusion::~AmbientOcclusion()
{
	delete m_sampleKernel;
	delete m_noiseTexture;
	delete m_vsCB;
	delete m_psCB;
}

void AmbientOcclusion::Initialize(UINT width, UINT height)
{
	m_ao.Initialize(width, height, false, Renderer::RTFormat_AO);

	if (m_sampleKernel == nullptr)
	{
		m_sampleKernel = new Graphics::GPUBuffer();

		std::vector<float3> sampleKernel;
		GenerateSampleKernel(m_sampleKernelSize, sampleKernel);

		GPUBufferDesc bd;
		bd.BindFlags = BIND_SHADER_RESOURCE;
		bd.Usage = USAGE_IMMUTABLE;
		bd.MiscFlags = RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(float3) * (UINT)sampleKernel.size();
		bd.StructureByteStride = sizeof(float3);
		SubresourceData initData;
		initData.SysMem = sampleKernel.data();
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_sampleKernel);
		Renderer::GetDevice()->TransitionBarrier(m_sampleKernel, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	if (m_noiseTexture == nullptr)
	{
		m_noiseTexture = new Graphics::Texture2D();

		std::vector<float3> noiseData;
		GenerateNoise(m_noiseTextureDimension * m_noiseTextureDimension, noiseData);

		TextureDesc desc;
		desc.Width = m_noiseTextureDimension; desc.Height = m_noiseTextureDimension; desc.Depth = 1;
		desc.ArraySize = 1;
		desc.Format = FORMAT_R32G32B32_FLOAT;
		desc.BindFlags = BIND_SHADER_RESOURCE;
		desc.MipLevels = 0;
		desc.MiscFlags = 0;
		SubresourceData initData;
		initData.SysMem = noiseData.data();
		initData.SysMemPitch = sizeof(float3) * m_noiseTextureDimension;
		initData.SysMemSlicePitch = sizeof(float3) * (UINT)noiseData.size();
		Renderer::GetDevice()->CreateTexture2D(desc, &initData, &m_noiseTexture);
		Renderer::GetDevice()->TransitionBarrier(m_noiseTexture, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	if (m_vsCB == nullptr)
	{
		m_vsCB = new Graphics::GPUBuffer();

		AoVSConstants aoCB;
		ZeroMemory(&aoCB, sizeof(aoCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(AoVSConstants);
		SubresourceData initData;
		initData.SysMem = &aoCB;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_vsCB);
		Renderer::GetDevice()->TransitionBarrier(m_vsCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (m_psCB == nullptr)
	{
		m_psCB = new Graphics::GPUBuffer();

		AoPSConstants aoCB;
		ZeroMemory(&aoCB, sizeof(aoCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(AoPSConstants);
		SubresourceData initData;
		initData.SysMem = &aoCB;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_psCB);
		Renderer::GetDevice()->TransitionBarrier(m_psCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void AmbientOcclusion::UpdateConstants(Renderer* renderer, const Camera& camera)
{
	{
		AoVSConstants vsCB;
		XMMATRIX viewM = XMLoadFloat4x4(&camera.m_view);
		XMMATRIX projM = XMLoadFloat4x4(&camera.m_proj);
		XMMATRIX invProjM = XMLoadFloat4x4(&camera.m_invProj);
		XMStoreFloat4x4(&vsCB.View, XMMatrixTranspose(viewM));
		XMStoreFloat4x4(&vsCB.Proj, XMMatrixTranspose(projM));
		XMStoreFloat4x4(&vsCB.InvProj, XMMatrixTranspose(invProjM));
		renderer->GetDevice()->UpdateBuffer(m_vsCB, &vsCB, sizeof(AoVSConstants));
	}

	{
		AoPSConstants psCB;
		psCB.NoiseTextureDimension = (float)m_noiseTextureDimension;
		psCB.SampleKernelSize = m_sampleKernelSize;
		psCB.OcclusionRadius = UIContext::OcclusionRadius;
		psCB.OcclusionPower = UIContext::OcclusionPower;
		psCB.OcclusionFalloff = UIContext::OcclusionFalloff;
		psCB.OcclusionDarkness = UIContext::OcclusionDarkness;
		psCB.ScreenWidth = (float)m_ao.GetDesc().Width;
		psCB.ScreenHeight = (float)m_ao.GetDesc().Height;
		psCB.CamerNear = camera.m_nearZ;
		psCB.CameraFar = camera.m_farZ;
		psCB.CameraNearInv = 1.0f / camera.m_nearZ;
		psCB.CameraFarInv = 1.0f / camera.m_farZ;
		renderer->GetDevice()->UpdateBuffer(m_psCB, &psCB, sizeof(AoPSConstants));
	}

	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::VS, m_vsCB, 0);
	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::PS, m_vsCB, 0);
	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::PS, m_psCB, 1);
}

void AmbientOcclusion::ComputeAO(Renderer* renderer)
{
	m_ao.Activate();
	
	renderer->GetDevice()->BindResource(SHADERSTAGE::PS, m_sampleKernel, 2);
	renderer->GetDevice()->BindResource(SHADERSTAGE::PS, m_noiseTexture, 3);
	renderer->GetDevice()->BindGraphicsPSO(renderer->GetPSO(eGPSO::AmbientOcclusionPass));
	renderer->GetDevice()->BindSampler(SHADERSTAGE::PS, renderer->GetSamplerState(eSamplerState::LinearClamp), 0);
	renderer->GetDevice()->BindSampler(SHADERSTAGE::PS, renderer->GetSamplerState(eSamplerState::PointClamp), 1);
	renderer->GetDevice()->DrawInstanced(3, 1, 0, 0);

	m_ao.Deactivate();
}