
#include "stdafx.h"
#include "GraphicsDevice_DX12.h"
#include "CreviceWindow.h"
#include "DXHelper.h"
#include "GeometryGenerator.h"
#include "Material.h"
#include "Camera.h"
#include "TextRenderer.h"

#define PBR_MODEL

CreviceWindow::CreviceWindow( std::string name )
	: BaseWindow( name )
{
}

void CreviceWindow::OnInit()
{
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
	
	m_engineTimer.Reset();

	Renderer::GGraphicsDevice = new Graphics::GraphicsDevice_DX12();
	Renderer::GGraphicsDevice->Initialize( this );

	m_psoCache.Initialize(*Renderer::GGraphicsDevice);
	m_samplerCache.Initialize(*Renderer::GGraphicsDevice);
	
	GetDevice().PresentBegin();
	InitializeTextures();
	InitializeMesh();
	InitializeRenderObjects();
	InitializeConstantBuffers();
	TextRenderer::Font::Initialize(GetWidth(), GetHeight());
	GetDevice().PresentEnd();

	Renderer::GGraphicsDevice->Flush();
}

void CreviceWindow::OnUpdate()
{
	BaseWindow::OnUpdate();
}

void CreviceWindow::OnRender()
{
	GetDevice().PresentBegin();

	GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_objCB, 0);
	GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_shadingCB, 0);

	{
		ScopedTimer perf("Rendering Objects", Renderer::GGraphicsDevice);

#ifdef PBR_MODEL
		if (m_wireframe)
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::PBRWireframe));
		else
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::PBRSolid));

		Renderer::GGraphicsDevice->BindResource(PS, m_textures[ETT_Albedo], 0);
		Renderer::GGraphicsDevice->BindResource(PS, m_textures[ETT_Normal], 1);
		Renderer::GGraphicsDevice->BindResource(PS, m_textures[ETT_Roughness], 2);
		Renderer::GGraphicsDevice->BindResource(PS, m_textures[ETT_Metalness], 3);
		GetDevice().BindSampler(SHADERSTAGE::PS, m_samplerCache.GetSamplerState(eSamplerState::AnisotropicWrap), 0);

		UpdateConstantBuffer(m_model);
		m_model.m_mesh->Draw(GetDevice(), m_model.m_world, GetCamera()->m_frustum);

		if (m_wireframe)
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::SkyboxWireframe));
		else
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::SkyboxSolid));

		UpdateConstantBuffer(m_skybox);
		Renderer::GGraphicsDevice->BindResource(PS, m_envTextureUnfiltered, 0);
		m_skybox.m_mesh->Draw(GetDevice(), m_skybox.m_world, GetCamera()->m_frustum);
#else
		if (m_wireframe)
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::SimpleColorWireframe));
		else
			GetDevice().BindGraphicsPSO(m_psoCache.GetPSO(eGPSO::SimpleColorSolid));

		GetDevice().BindSampler(SHADERSTAGE::PS, m_samplerCache.GetSamplerState(eSamplerState::LinearClamp), 0);

 		for (auto o : m_renderObjects)
 		{
 			//Update buffer
 			UpdateConstantBuffer(o);
 			o.m_mesh->Draw(GetDevice(), o.m_world, GetCamera()->m_frustum);
 		}
#endif
	}

	ScopedTimer::RenderPerfCounters();

	GetDevice().PresentEnd();
}

void CreviceWindow::OnDestroy()
{
	TextRenderer::Font::CleanUpStatic();
	delete Renderer::GGraphicsDevice;

	// PBR Model
	for (int i = 0; i < ETT_Max; ++i)
		delete m_textures[i];

	delete m_envTexture;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CreviceWindow::OnKeyDown(UINT8 key)
{
	BaseWindow::OnKeyDown(key);
}
void CreviceWindow::OnKeyUp(UINT8 key)
{
	BaseWindow::OnKeyUp(key);
}

void CreviceWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseDown(btnState, x, y);
}

void CreviceWindow::OnMouseUp(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseUp(btnState, x, y);
}

void CreviceWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseMove(btnState, x, y);
}

void CreviceWindow::InitializeRenderObjects()
{
	static const UINT32 NumObjects = 64;
	for (UINT32 i = 0; i < NumObjects; ++i)
	{
		RenderObject ro;
		ro.m_mesh.reset(new Mesh());
		{
			UINT32 j = i % 4;
			GeometryGenerator::MeshData obj;
			switch (j)
			{
			case 0:
				obj = GeometryGenerator::CreateSphere(1.0f, 8, 8);
				break;
			case 1:
				obj = GeometryGenerator::CreateBox(2.0f, 2.0f, 2.0f, 3);
				break;
			case 2:
				obj = GeometryGenerator::CreateCylinder(1.0f, 0.0f, 2.0f, 8, 8);
				break;
			case 3:
				obj = GeometryGenerator::CreateCylinder(1.0f, 1.0f, 2.0f, 8, 8);
			}

			const UINT vbByteSize = (UINT)obj.Vertices.size() * sizeof(GeometryGenerator::Vertex);
			const UINT ibByteSize = (UINT)obj.Indices32.size() * sizeof(UINT16);

			ro.m_mesh->CreateVertexBuffers(GetDevice(), obj.Vertices.data(), vbByteSize, sizeof(GeometryGenerator::Vertex));
			ro.m_mesh->CreateIndexBuffers(GetDevice(), obj.GetIndices16().data(), ibByteSize, FORMAT_R16_UINT);

			XMMATRIX worldMtx = XMMatrixTranslation(-8.0f + 2.0f * (i / 8), 0, -8.0f + 2.0f* (i % 8));
			XMStoreFloat4x4(&ro.m_world, worldMtx);

			Submesh submesh;
			submesh.IndexCount = (UINT)obj.Indices32.size();
			submesh.StartIndexLocation = 0;
			submesh.BaseVertexLocation = 0;
			submesh.Bounds = obj.BBox;

			ro.m_mesh->m_drawArgs.reserve(1);
			ro.m_mesh->m_drawArgs.push_back(submesh);
		}
		m_renderObjects.push_back(std::move(ro));
	}
}

void CreviceWindow::InitializeTextures()
{
	GetDevice().CreateTextureFromFile("Data/Textures/cerberus_A.png", &m_textures[ETT_Albedo], true);
	GetDevice().CreateTextureFromFile("Data/Textures/cerberus_N.png", &m_textures[ETT_Normal], true);
	GetDevice().CreateTextureFromFile("Data/Textures/cerberus_R.png", &m_textures[ETT_Roughness], true);
	GetDevice().CreateTextureFromFile("Data/Textures/cerberus_M.png", &m_textures[ETT_Metalness], true);

	TextureDesc desc;
	desc.Width = 1024; desc.Height = 1024;
	desc.ArraySize = 6;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.BindFlags = BIND_SHADER_RESOURCE;
	GetDevice().CreateTexture2D(desc, nullptr, &m_envTexture);
	{
		// Unfiltered environment cube map (temporary).
		desc.BindFlags |= BIND_UNORDERED_ACCESS;
		desc.MiscFlags = RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 0;
		GetDevice().CreateTexture2D(desc, nullptr, &m_envTextureUnfiltered);

		// Load & convert equirectangular environment map to cubemap texture
		{
			GetDevice().CreateTextureFromFile("Data/Environments/environment.hdr", &m_envTextureEquirect, false);

			GetDevice().BindComputePSO(m_psoCache.GetPSO(Equirect2Cube));

			GetDevice().BindSampler(SHADERSTAGE::CS, m_samplerCache.GetSamplerState(eSamplerState::LinearWrap), 0);
			GetDevice().TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_COMMON, RESOURCE_STATE_UNORDERED_ACCESS);
			GetDevice().BindResource(SHADERSTAGE::CS, m_envTextureEquirect, 0);
			GetDevice().BindUnorderedAccessResource(m_envTextureUnfiltered, 0);
			GetDevice().Dispatch(m_envTexture->m_desc.Width / 32, m_envTexture->m_desc.Height / 32, 6);
			GetDevice().TransitionBarrier(m_envTextureUnfiltered, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_COMMON);

			GetDevice().GenerateMipmaps(m_envTextureUnfiltered);
		}
	}
}

void CreviceWindow::InitializeMesh()
{
	m_model.m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/cerberus.fbx");
	m_model.m_world = MathHelper::Identity4x4();
	m_model.m_world._11 *= 0.1f;
	m_model.m_world._22 *= 0.1f;
	m_model.m_world._33 *= 0.1f;

	m_skybox.m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/skybox.obj");
	m_skybox.m_world = MathHelper::Identity4x4();
	m_skybox.m_world._11 *= 10.0f;
	m_skybox.m_world._22 *= 10.0f;
	m_skybox.m_world._33 *= 10.0f;
}

void CreviceWindow::InitializeConstantBuffers()
{
	if (m_objCB == nullptr)
	{
		m_objCB = new Graphics::GPUBuffer();

		ObjectConstants objCB;
		ZeroMemory(&objCB, sizeof(objCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(ObjectConstants);
		SubresourceData initData;
		initData.SysMem = &objCB;
		GetDevice().CreateBuffer(bd, &initData, m_objCB);
		GetDevice().TransitionBarrier(m_objCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (m_shadingCB == nullptr)
	{
		m_shadingCB = new Graphics::GPUBuffer();

		ShadingConstants shadingCB;
		ZeroMemory(&shadingCB, sizeof(shadingCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(ShadingConstants);
		SubresourceData initData;
		initData.SysMem = &shadingCB;
		GetDevice().CreateBuffer(bd, &initData, m_shadingCB);
		GetDevice().TransitionBarrier(m_shadingCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void CreviceWindow::UpdateConstantBuffer(const RenderObject& renderObject)
{
	if (m_objCB != nullptr)
	{
		const Camera* cam = GetCamera();

		XMMATRIX world = XMLoadFloat4x4(&renderObject.m_world);
		XMMATRIX view = XMLoadFloat4x4(&cam->m_view);
		XMMATRIX proj = XMLoadFloat4x4(&cam->m_proj);
		XMMATRIX worldViewProj = world * view * proj;
		XMVECTOR eyePos = XMLoadFloat3(&cam->m_eyePos);

		ObjectConstants objCB;
		ZeroMemory(&objCB, sizeof(objCB));
		// Update constant buffer with the latest worldViewProj matrix
		XMStoreFloat4x4(&objCB.Scene, XMMatrixTranspose(view));
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));
		GetDevice().UpdateBuffer(m_objCB, &objCB, sizeof(ObjectConstants));
	}

	if (m_shadingCB != nullptr)
	{
		const Camera* cam = GetCamera();

		static Light lights[MaxLights] = {
			{ float3(0, 0, -1), 1.0f },
			{ float3(1, 0, 0), 1.0f },
			{ float3(0, -1, 0), 1.0f }
		};
		static UINT lightsCount = 3;

		XMVECTOR eyePos = XMLoadFloat3(&cam->m_eyePos);

		ShadingConstants shadingCB;
		ZeroMemory(&shadingCB, sizeof(shadingCB));
		XMStoreFloat4(&shadingCB.EyePosition, eyePos);

		for (UINT i = 0; i < lightsCount; ++i)
		{
			XMVECTOR lightDirV = XMLoadFloat3(&lights[i].LightDirection);
			XMStoreFloat3(&shadingCB.Lights[i].LightDirection, lightDirV);
			shadingCB.Lights[i].LightRadiance = lights[i].LightRadiance;
		}
		shadingCB.LightsCount = lightsCount;

		GetDevice().UpdateBuffer(m_shadingCB, &shadingCB, sizeof(ShadingConstants));
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 