
#include "stdafx.h"

#include "CreviceWindow.h"
#include "DXHelper.h"
#include "GeometryGenerator.h"
#include "Material.h"
#include "Camera.h"
#include "UIContext.h"
#include "Renderer.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#define PBR_MODEL 0

CreviceWindow::CreviceWindow(std::string name)
	: BaseWindow(name)
{
}

void CreviceWindow::OnInit()
{
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

	m_engineTimer.Reset();

	m_renderer = new Renderer(DirectX12, this);

	GetDevice().PresentBegin();
	GetDevice().SetBackBuffer();
	InitializeConstantBuffers();
	InitializeTextures();
	InitializeMesh();
	InitializeRenderObjects();
	GetDevice().PresentEnd();

	Renderer::GGraphicsDevice->Flush();
}

void CreviceWindow::OnUpdate()
{
	BaseWindow::OnUpdate();
}

void CreviceWindow::OnRender()
{
	UIContext::DrawUI();

	GetDevice().PresentBegin();

	UpdateHDRSkybox();

	UpdateGlobalConstantBuffer();

	{
		{
			ScopedTimer perf("Rendering Objects", Renderer::GGraphicsDevice);

			GetRenderer().SetFrameBuffer(true);
			GetRenderer().BindIBL();

			// render objects
			GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_objCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_shadingCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 1);

#if PBR_MODEL
			GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(UIContext::PBRModel == 0 ? eGPSO::PBRSimpleSolid : eGPSO::PBRSolid, UIContext::Wireframe));

			GPUResource* textures[] = {
				m_textures[ETT_Albedo],
				m_textures[ETT_Normal],
				m_textures[ETT_Roughness],
				m_textures[ETT_Metalness],
			};
			GetDevice().BindResources(PS, textures, 0, 4);
			GetDevice().BindSampler(SHADERSTAGE::PS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0);

			const RenderObject& model = m_model[UIContext::PBRModel];
			UpdateObjectConstantBuffer(model);
			model.m_mesh->Draw(GetDevice(), model.m_world, GetCamera()->m_frustum);
#else
			GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(UIContext::PBRModel == 0 ? eGPSO::PBRSimpleSolid : eGPSO::PBRSolid, UIContext::Wireframe));
			GetDevice().BindSampler(SHADERSTAGE::PS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0);

			for (auto o : m_renderObjects)
			{
				//Update buffer
				UpdateObjectConstantBuffer(o);
				o.m_mesh->Draw(GetDevice(), o.m_world, GetCamera()->m_frustum);
			}
#endif
		}

		{
			ScopedTimer perf("Rendering Background", Renderer::GGraphicsDevice);

			// render background
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 0);
			GetRenderer().RenderBackground();
		}

		GetRenderer().SetFrameBuffer(false);

		GetDevice().SetBackBuffer();

		{
			ScopedTimer perf("Rendering PostProcess", Renderer::GGraphicsDevice);

			GetRenderer().DoPostProcess();
		}
	}

	ScopedTimer::RenderPerfCounters();
	GetDevice().FlushUI();

	GetDevice().PresentEnd();
}

void CreviceWindow::OnDestroy()
{
	// PBR Model
	for (int i = 0; i < ETT_Max; ++i)
		delete m_textures[i];
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

void CreviceWindow::UpdateHDRSkybox()
{
	if (UIContext::HDRSkybox != m_skyboxID)
	{
		std::string name;
		switch (UIContext::HDRSkybox)
		{
		case 0:
			name = "environment";
			break;
		case 1:
			name = "rooftop";
			break;
		case 2:
			name = "cape_hill";
			break;
		case 3:
			name = "venice_sunset";
			break;
		case 4:
			name = "newport_loft";
			break;
		}

		GetRenderer().InitializeIBLTextures(name);
		m_skyboxID = UIContext::HDRSkybox;
	}
}

void CreviceWindow::InitializeRenderObjects()
{
	static const UINT32 NumObjects = 64;
	for (UINT32 i = 0; i < NumObjects; ++i)
	{
		RenderObject ro;
		ro.m_mesh.reset(new Mesh());
		{
			UINT32 j = 0;// i % 4;
			GeometryGenerator::MeshData obj;
			switch (j)
			{
			case 0:
				obj = GeometryGenerator::CreateSphere(1.0f, 16, 16);
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
}

void CreviceWindow::InitializeMesh()
{
	m_model[EMT_Sphere].m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/sphere.obj");
	m_model[EMT_Sphere].m_world = MathHelper::Identity4x4();
	m_model[EMT_Sphere].m_world._11 *= 0.1f;
	m_model[EMT_Sphere].m_world._22 *= 0.1f;
	m_model[EMT_Sphere].m_world._33 *= 0.1f;

	m_model[EMT_Cerberus].m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/cerberus.fbx");
	m_model[EMT_Cerberus].m_world = m_model[EMT_Sphere].m_world;

	m_skybox.m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/skybox.obj");
	m_skybox.m_world = MathHelper::Identity4x4();
	m_skybox.m_world._11 *= 100.0f;
	m_skybox.m_world._22 *= 100.0f;
	m_skybox.m_world._33 *= 100.0f;
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

	if (m_backgroundCB == nullptr)
	{
		m_backgroundCB = new Graphics::GPUBuffer();

		BackgroundConstants backgrounsCB;
		ZeroMemory(&backgrounsCB, sizeof(backgrounsCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(BackgroundConstants);
		SubresourceData initData;
		initData.SysMem = &backgrounsCB;
		GetDevice().CreateBuffer(bd, &initData, m_backgroundCB);
		GetDevice().TransitionBarrier(m_backgroundCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void CreviceWindow::UpdateGlobalConstantBuffer()
{
	if (m_shadingCB != nullptr)
	{
		const Camera* cam = GetCamera();

		static Light lights[MaxLights] = {
			{ float3(0, 0, -1), 1.0f },
		{ float3(1, 0, 0), 1.0f },
		{ float3(0, -1, 0), 1.0f }
		};
		static UINT lightsCount = 0;

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

		shadingCB.Color = float3(UIContext::Color);
		shadingCB.Roughness = UIContext::Roughness;
		shadingCB.Metalness = UIContext::Metalness;

		GetDevice().UpdateBuffer(m_shadingCB, &shadingCB, sizeof(ShadingConstants));
	}

	if (m_backgroundCB != nullptr)
	{
		BackgroundConstants backgroundCB;
		ZeroMemory(&backgroundCB, sizeof(backgroundCB));

		const Camera* cam = GetCamera();

		XMMATRIX view = XMLoadFloat4x4(&cam->m_view);
		XMMATRIX proj = XMLoadFloat4x4(&cam->m_proj);
		XMMATRIX worldToScreen = view * proj;
		XMMATRIX screenToWorld = XMMatrixInverse(&XMMatrixDeterminant(worldToScreen), worldToScreen);
		XMStoreFloat4x4(&backgroundCB.ScreenToWorld, XMMatrixTranspose(screenToWorld));

		XMMATRIX cubeRotation = XMMatrixRotationRollPitchYaw(0.f, MathHelper::Deg2Rad((float)UIContext::CubemapRotation), 0.f);
		XMStoreFloat4x4(&backgroundCB.CubemapRotation, XMMatrixTranspose(cubeRotation));

		backgroundCB.ScreenDimensions = float4((float)GetWidth(), (float)GetHeight(), tanf(0.5f * MathHelper::Deg2Rad(cam->m_fov)), (float)GetHeight() / (float)GetWidth());

		GetDevice().UpdateBuffer(m_backgroundCB, &backgroundCB, sizeof(BackgroundConstants));
	}
}

void CreviceWindow::UpdateObjectConstantBuffer(const RenderObject& renderObject)
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
		XMStoreFloat4x4(&objCB.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objCB.Scene, XMMatrixTranspose(view));
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));
		GetDevice().UpdateBuffer(m_objCB, &objCB, sizeof(ObjectConstants));
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 