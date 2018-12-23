#include "stdafx.h"
#include "Renderer.h"
#include "BaseWindow.h"
#include "TextRenderer.h"

#include "GraphicsDevice_DX12.h"

Graphics::GraphicsDevice* Renderer::GGraphicsDevice = nullptr;

Renderer::Renderer(GpuAPI gpuApi, BaseWindow* window)
{
	switch (gpuApi)
	{
	case DirectX11:
		assert("not implemented yet");
		break;
	case DirectX12:
		GGraphicsDevice = new Graphics::GraphicsDevice_DX12();
		break;
	case Vulkan:
		assert("not implemented yet");
		break;
	default:
		assert(false);
		break;
	}
	
	GGraphicsDevice->Initialize(window);

	m_psoCache.Initialize(*Renderer::GGraphicsDevice);
	m_samplerCache.Initialize(*Renderer::GGraphicsDevice);

	GGraphicsDevice->PresentBegin();
	{
		m_gbuffer.Initialize(window->GetWidth(), window->GetHeight(), true, RTFormat_GBuffer0);
		m_gbuffer.Add(RTFormat_GBuffer1);
		m_gbuffer.Add(RTFormat_GBuffer2);

		m_frameBuffer.Initialize(window->GetWidth(), window->GetHeight(), true, Renderer::RTFormat_HDR);

		GGraphicsDevice->SetBackBuffer();
		InitializeConstantBuffers();
		InitializeIBLTextures("environment");
		InitializeHitProxyBuffers();
		TextRenderer::Font::Initialize(window->GetWidth(), window->GetHeight());
	}
	GGraphicsDevice->PresentEnd();

	GGraphicsDevice->Flush();
}

Renderer::~Renderer()
{
	delete GGraphicsDevice;

	TextRenderer::Font::CleanUpStatic();

	delete m_envTexture;
	delete m_envTextureEquirect;
	delete m_envTextureUnfiltered;
	delete m_irradianceMap;
	delete m_spBRDFLut;

	delete m_specularMapFilterCB;

	delete m_hitProxy;
	delete m_hitProxyReadback;
}

void Renderer::InitializeConstantBuffers()
{
	if (m_specularMapFilterCB == nullptr)
	{
		m_specularMapFilterCB = new Graphics::GPUBuffer();

		SpecularMapFilterConstants spmapCB;
		ZeroMemory(&spmapCB, sizeof(spmapCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(SpecularMapFilterConstants);
		SubresourceData initData;
		initData.SysMem = &spmapCB;
		GGraphicsDevice->CreateBuffer(bd, &initData, m_specularMapFilterCB);
		GGraphicsDevice->TransitionBarrier(m_specularMapFilterCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void Renderer::InitializeHitProxyBuffers()
{
	GPUBufferDesc bd;
	bd.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
	bd.Usage = USAGE_DEFAULT;
	bd.CpuAccessFlags = 0;
	bd.ByteWidth = sizeof(int);
	bd.StructureByteStride = 4;
	bd.Format = FORMAT_R32_UINT;

	m_hitProxy = new Graphics::GPUBuffer();
	GGraphicsDevice->CreateBuffer(bd, nullptr, m_hitProxy);

	bd.BindFlags = BIND_RESOURCE_NONE;
	bd.Usage = USAGE_STAGING;
	bd.CpuAccessFlags = CPU_ACCESS_READ;
	bd.Format = FORMAT_UNKNOWN;

	m_hitProxyReadback = new Graphics::GPUReadbackBuffer();
	GGraphicsDevice->CreateBuffer(bd, nullptr, m_hitProxyReadback);
}

void Renderer::InitializeIBLTextures(const std::string& name)
{
	TextureDesc desc;
	desc.Width = 1024; desc.Height = 1024;
	desc.ArraySize = 6;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
	desc.MipLevels = 0;
	desc.MiscFlags = RESOURCE_MISC_TEXTURECUBE;
	m_envTexture = new Texture2D();
	m_envTexture->RequestIndependentUnorderedAccessResourcesForMIPs(true);
	GGraphicsDevice->CreateTexture2D(desc, nullptr, &m_envTexture);
	{
		// Unfiltered environment cube map (temporary).
		m_envTexture->RequestIndependentUnorderedAccessResourcesForMIPs(false);
		GGraphicsDevice->CreateTexture2D(desc, nullptr, &m_envTextureUnfiltered);

		// Load & convert equirectangular environment map to cubemap texture
		{
			GGraphicsDevice->CreateTextureFromFile("Data/Environments/" + name + ".hdr", &m_envTextureEquirect, false);

			GGraphicsDevice->BindComputePSO(m_psoCache.GetPSO(Equirect2Cube));

			GGraphicsDevice->BindSampler(SHADERSTAGE::CS, m_samplerCache.GetSamplerState(eSamplerState::LinearWrap), 0);
			GGraphicsDevice->TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_COMMON, RESOURCE_STATE_UNORDERED_ACCESS);
			GGraphicsDevice->BindResource(SHADERSTAGE::CS, m_envTextureEquirect, 0);
			GGraphicsDevice->BindUnorderedAccessResource(SHADERSTAGE::CS, m_envTextureUnfiltered, 0);
			GGraphicsDevice->Dispatch(m_envTexture->m_desc.Width / 32, m_envTexture->m_desc.Height / 32, 6);
			GGraphicsDevice->TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_COMMON);

			GGraphicsDevice->GenerateMipmaps(m_envTextureUnfiltered);
		}

		// Compute pre-filtered specular environment map
		{
			// Copy 0th mipmap level into destination environment map.
			GGraphicsDevice->TransitionBarrier(m_envTexture, RESOURCE_STATE_COMMON, RESOURCE_STATE_COPY_DEST);
			GGraphicsDevice->TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_COMMON, RESOURCE_STATE_COPY_SOURCE);
			for (UINT arraySlice = 0; arraySlice < 6; ++arraySlice)
			{
				GGraphicsDevice->CopyTextureRegion(m_envTexture, 0, 0, 0, 0, m_envTextureUnfiltered, 0, arraySlice);
			}
			GGraphicsDevice->TransitionBarrier(m_envTexture, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_UNORDERED_ACCESS);
			GGraphicsDevice->TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_COPY_SOURCE, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			// Pre-filter rest of the mip chain.
			GGraphicsDevice->BindComputePSO(m_psoCache.GetPSO(SpecularEnvironmentMap));
			GGraphicsDevice->BindConstantBuffer(SHADERSTAGE::CS, m_specularMapFilterCB, 0);
			GGraphicsDevice->BindResource(SHADERSTAGE::CS, m_envTextureUnfiltered, 0);

			const float deltaRoughness = 1.0f / std::max(float(m_envTexture->m_desc.MipLevels - 1), 1.0f);
			for (UINT level = 1, size = 512; level < m_envTexture->m_desc.MipLevels; ++level, size /= 2)
			{
				const UINT numGroups = std::max<UINT>(1, size / 32);
				const float spmapRoughness = level * deltaRoughness;

				SpecularMapFilterConstants spmapCB;
				spmapCB.Roughness = spmapRoughness;

				GGraphicsDevice->UpdateBuffer(m_specularMapFilterCB, &spmapCB, sizeof(SpecularMapFilterConstants));
				GGraphicsDevice->BindUnorderedAccessResource(SHADERSTAGE::CS, m_envTexture, 0, level);

				GGraphicsDevice->Dispatch(numGroups, numGroups, 6);
			}

			GGraphicsDevice->TransitionBarrier(m_envTexture, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_COMMON);
		}

		// Compute diffuse irradiance cubemap.
		desc.Width = 32; desc.Height = 32;
		desc.ArraySize = 6;
		desc.MipLevels = 1;
		GGraphicsDevice->CreateTexture2D(desc, nullptr, &m_irradianceMap);
		{
			GGraphicsDevice->BindComputePSO(m_psoCache.GetPSO(IrradianceMap));

			GGraphicsDevice->TransitionBarrier(m_irradianceMap, RESOURCE_STATE_COMMON, RESOURCE_STATE_UNORDERED_ACCESS);

			GGraphicsDevice->BindResource(SHADERSTAGE::CS, m_envTexture, 0);
			GGraphicsDevice->BindUnorderedAccessResource(SHADERSTAGE::CS, m_irradianceMap, 0);

			GGraphicsDevice->Dispatch(m_irradianceMap->m_desc.Width / 32, m_irradianceMap->m_desc.Height / 32, 6);

			GGraphicsDevice->TransitionBarrier(m_irradianceMap, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_COMMON);
		}

		// Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.
		desc.Width = 256; desc.Height = 256;
		desc.ArraySize = 1;
		desc.Format = FORMAT_R16G16_FLOAT;
		GGraphicsDevice->CreateTexture2D(desc, nullptr, &m_spBRDFLut);
		{
			GGraphicsDevice->BindComputePSO(m_psoCache.GetPSO(SpecularBRDFLut));
			GGraphicsDevice->BindSampler(SHADERSTAGE::CS, m_samplerCache.GetSamplerState(eSamplerState::LinearClamp), 1);

			GGraphicsDevice->TransitionBarrier(m_spBRDFLut, RESOURCE_STATE_COMMON, RESOURCE_STATE_UNORDERED_ACCESS);

			GGraphicsDevice->BindUnorderedAccessResource(SHADERSTAGE::CS, m_spBRDFLut, 0);

			GGraphicsDevice->Dispatch(m_spBRDFLut->m_desc.Width / 32, m_spBRDFLut->m_desc.Height / 32, 1);

			GGraphicsDevice->TransitionBarrier(m_spBRDFLut, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_COMMON);
		}
	}
}

void Renderer::BindIBL()
{
	GPUResource* textures[] = {

		m_envTexture,
		m_irradianceMap,
		m_spBRDFLut
	};
	GGraphicsDevice->BindResources(PS, textures, 4, 3);
}

void Renderer::BindGBuffer()
{
	GPUResource* textures[] = {

		m_gbuffer.GetTexture(0),
		m_gbuffer.GetTexture(1),
		m_gbuffer.GetTexture(2),
		m_gbuffer.GetDepthTexture()
	};
	GGraphicsDevice->BindResources(PS, textures, 0, 4);
}

void Renderer::BindEnvTexture(SHADERSTAGE stage, int slot)
{
	GGraphicsDevice->BindResource(stage, m_envTexture, slot);
}

void Renderer::RenderLighting()
{
	BindGBuffer();

	GGraphicsDevice->BindUnorderedAccessResource(PS, m_hitProxy, 0);

	GGraphicsDevice->BindGraphicsPSO(GetPSO(eGPSO::LightingPass));
	GGraphicsDevice->BindSampler(SHADERSTAGE::PS, GetSamplerState(eSamplerState::LinearClamp), 0);
	GGraphicsDevice->DrawInstanced(3, 1, 0, 0);
}

void Renderer::RenderBackground()
{
	BindEnvTexture(PS, 0);
	GGraphicsDevice->BindResource(PS, m_gbuffer.GetDepthTexture(), 1);

	GGraphicsDevice->BindGraphicsPSO(GetPSO(eGPSO::Background));
	GGraphicsDevice->BindSampler(SHADERSTAGE::PS, GetSamplerState(eSamplerState::LinearClamp), 0);
	GGraphicsDevice->DrawInstanced(3, 1, 0, 0);
}

void Renderer::DoPostProcess()
{
	// Reinhard Tonemapping
	GGraphicsDevice->BindResource(PS, m_frameBuffer.GetTexture(), 0);
	GGraphicsDevice->BindSampler(SHADERSTAGE::PS, GetSamplerState(eSamplerState::LinearClamp), 0);
	GGraphicsDevice->BindGraphicsPSO(GetPSO(eGPSO::TonemappingReinhard));
	GGraphicsDevice->DrawInstanced(3, 1, 0, 0);
}

UINT Renderer::ReadBackHitProxy()
{
	if (m_hitProxyReadback->IsLocked())
	{
		void* ptr = GGraphicsDevice->Map(m_hitProxyReadback);
		if (ptr)
		{
			GGraphicsDevice->Unmap(m_hitProxyReadback);
			m_hitProxyReadback->m_isLocked = false;
			UINT* ptrU = (UINT*)ptr;
			return *ptrU;
		}
	}

	if (!m_hitProxyReadback->IsLocked())
	{
		GGraphicsDevice->CopyBuffer(m_hitProxyReadback, m_hitProxy);
		m_hitProxyReadback->m_isLocked = true;
	}

	return -1;
}
