
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
	GetDevice().PresentBegin();

	UIContext::DrawUI(this);
	{
		if (m_hitProxyID >= st_objectsOffset)
		{
			UINT id = m_hitProxyID - st_objectsOffset;

			RenderObject& ro = m_renderObjects[id];
			UIContext::ShowObjectSettings(ro);
			ro.SetWorld();
		}
	}

	UpdateHDRSkybox();

	UpdateGlobalConstantBuffer();

	{
		{
			ScopedTimer perf("Rendering Shadowmap", Renderer::GGraphicsDevice);

			GetRenderer().SetShadowMapDepth();
			GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(eGPSO::SimpleDepthShadow));

			GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_objVsCB, 0);

			int i = st_objectsOffset;
			GlobalLightCamera lightCamera(m_globalLight.GetSunDirection(UIContext::Time));
			const Frustum& frustum = lightCamera.m_frustum;
			for (auto o : m_renderObjects)
			{
				//Update buffer
				if (o.IsEnabled() && frustum.CheckBox(o.m_mesh->GetBoundingBox(o.GetWorld())))
				{
					// TODO: light view matrix, ortho projection matrix
					UpdateObjectConstantBufferShadows(o, lightCamera);
					o.m_mesh->Draw(GetDevice());
				}
			}
		}

		{
			ScopedTimer perf("Rendering Objects", Renderer::GGraphicsDevice);

			GetRenderer().SetGBuffer(true);
			GetRenderer().BindIBL();

			// render objects
			GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_objVsCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_objPsCB, 0);

			// Render debug grid
			if (UIContext::DebugGrid)
			{
				RenderObject ro;
				UpdateObjectConstantBuffer(ro, 0);
				GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(eGPSO::GridSolid));
				m_grid.Render();
			}

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
			UpdateObjectConstantBuffer(model, 1);
			model.m_mesh->Draw(GetDevice());
#else
			GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(UIContext::PBRModel == 0 ? eGPSO::PBRSimpleSolid : eGPSO::PBRSolid, UIContext::Wireframe));
			GetDevice().BindSampler(SHADERSTAGE::PS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0);

			int i = st_objectsOffset;
			const Frustum& frustum = GetCamera()->m_frustum;
			for (auto o : m_renderObjects)
			{
				//Update buffer
				if (o.IsEnabled() && frustum.CheckBox(o.m_mesh->GetBoundingBox(o.GetWorld())))
				{
					UpdateObjectConstantBuffer(o, i++);
					o.m_mesh->Draw(GetDevice());
				}
			}
#endif

			GetRenderer().SetGBuffer(false);
		}

		GetRenderer().SetFrameBuffer(true);

		{
			ScopedTimer perf("Lighting Pass", Renderer::GGraphicsDevice);

			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_shadingCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 1);

			GetRenderer().RenderLighting();
		}

		if(UIContext::HDRSkybox > 0)
		{
			ScopedTimer perf("Rendering Background", Renderer::GGraphicsDevice);

			// render background
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 0);
			GetRenderer().RenderBackground();
		}

		// Render Gizmo
		if (m_gizmo.IsActive())
		{
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_objPsCB, 0);

			GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(eGPSO::GizmoSolid));

			for (int i = eAxis::X; i <= eAxis::Z; ++i)
			{
				switch (m_gizmo.GetType())
				{
				case Translator:
					UpdateObjectConstantBuffer(m_gizmo.GetAxis(i).m_translator);
					break;
				case Rotator:
					UpdateObjectConstantBuffer(m_gizmo.GetAxis(i).m_rotator);
					break;
				case Scaler:
					UpdateObjectConstantBuffer(m_gizmo.GetAxis(i).m_scaler);
					break;
				}
				m_gizmo.Render(i);
			}
		}

		GetRenderer().SetFrameBuffer(false);

#pragma region HitProxy
		Renderer::HitProxyData hitProxyData = GetRenderer().ReadBackHitProxy();
		m_gizmo.UpdateFocus(m_lastMousePos.x, m_lastMousePos.y, GetWidth(), GetHeight(), m_camera);

		if (m_readHitProxy)
		{
			if (hitProxyData.HitProxyID != -1)
			{
				// set selection
				m_readHitProxy = false;
				
				if (!m_gizmo.IsGizmoActive())
				{
					if (hitProxyData.HitProxyID >= st_objectsOffset)
					{
						m_hitProxyID = hitProxyData.HitProxyID;

						RenderObject& ro = m_renderObjects[m_hitProxyID - st_objectsOffset];

						m_gizmo.SetActive(true, ro.GetTranslation());
					}
					else if (hitProxyData.HitProxyID < Gizmo::st_gizmoOffset)
					{
						m_hitProxyID = hitProxyData.HitProxyID;

						m_gizmo.SetActive(false);
					}
				}
			}
		}

		if (m_hitProxyID != -1)
		{
			ScopedTimer perf("Render Selection", Renderer::GGraphicsDevice);

			// Render to custom depth (selection)
			GetRenderer().SetSelectionDepth();

			if (m_hitProxyID >= (int)st_objectsOffset)
			{
				RenderObject& ro = m_renderObjects[m_hitProxyID - st_objectsOffset];
				if (ro.IsEnabled())
				{
					float d = m_camera->DistanceTo(float3(ro.GetX(), ro.GetY(), ro.GetZ()));
					m_gizmo.SetScale(d / 15.0f);
					m_gizmo.SetTransform(ro.GetTransform());

					GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(eGPSO::SimpleDepthSelection));
					UpdateObjectConstantBuffer(ro, 0);
					ro.m_mesh->Draw(GetDevice());
				}
			}

			// sobel-filter edge detection post process
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 0);
			GetRenderer().EdgeDetection();
		}
#pragma endregion HitProxy

		GetDevice().SetBackBuffer();

		{
			ScopedTimer perf("Rendering PostProcess", Renderer::GGraphicsDevice);

			GetRenderer().DoPostProcess();
		}
	}

	{
		ScopedTimer::RenderPerfCounters();
		GetDevice().FlushUI();
	}
	GetDevice().PresentEnd();
}

void CreviceWindow::OnDestroy()
{
	// PBR Model
	for (int i = 0; i < ETT_Max; ++i)
		delete m_textures[i];

	// CB
	delete m_objVsCB;
	delete m_objPsCB;
	delete m_shadingCB;
	delete m_backgroundCB;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CreviceWindow::OnKeyDown(UINT8 key)
{
	BaseWindow::OnKeyDown(key);

	if (!ImGui::IsAnyItemActive())
	{
		if (key == '1')
		{
			m_gizmo.SetType(Translator);
		}
		else if (key == '2')
		{
			m_gizmo.SetType(Rotator);
		}
		else if (key == '3')
		{
			m_gizmo.SetType(Scaler);
		}
	}
}
void CreviceWindow::OnKeyUp(UINT8 key)
{
	BaseWindow::OnKeyUp(key);
}

void CreviceWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseDown(btnState, x, y);

	if (!ImGui::IsAnyWindowHovered())
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			m_readHitProxy = true;
			m_gizmo.SetDrag(true);
		}
	}
}

void CreviceWindow::OnMouseUp(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseUp(btnState, x, y);

	m_gizmo.SetDrag(false);
}

void CreviceWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0 && m_hitProxyID >= (int)st_objectsOffset)
	{
		RenderObject& ro = m_renderObjects[m_hitProxyID - st_objectsOffset];

		m_gizmo.EditTransform(x, y, GetWidth(), GetHeight(), m_camera, ro.m_transform);
		ro.SetWorld();
		m_gizmo.SetTransform(ro.GetTransform());
	}

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
			name = "black.dds";
			break;
		case 1:
			name = "environment.hdr";
			break;
		case 2:
			name = "rooftop.hdr";
			break;
		case 3:
			name = "cape_hill.hdr";
			break;
		case 4:
			name = "venice_sunset.hdr";
			break;
		case 5:
			name = "newport_loft.hdr";
			break;
		}

		if(UIContext::HDRSkybox >= 0) 
			GetRenderer().InitializeIBLTextures(name);
		m_skyboxID = UIContext::HDRSkybox;
	}
}

void CreviceWindow::AddObject(eObject id, float a, float b, float c, float d, float e)
{
	RenderObject ro;
	ro.m_mesh.reset(new Mesh());
	{
		GeometryGenerator::MeshData obj;
		switch (id)
		{
		case eObject::Plane:
			obj = GeometryGenerator::CreateGrid(a, b, (UINT32)c, (UINT32)d);
			break;
		case eObject::Box:
			obj = GeometryGenerator::CreateBox(a, b, c, (UINT32)d);
			break;
		case eObject::Sphere:
			obj = GeometryGenerator::CreateSphere(a, (UINT32)b, (UINT32)c);
			break;
		case eObject::Cone:
			obj = GeometryGenerator::CreateCylinder(a, 0.0f, b, (UINT32)c, (UINT32)d);
			break;
		case eObject::Cylinder:
			obj = GeometryGenerator::CreateCylinder(a, a, b, (UINT32)c, (UINT32)d);
			break;
		}

		const UINT vbByteSize = (UINT)obj.Vertices.size() * sizeof(GeometryGenerator::Vertex);
		const UINT ibByteSize = (UINT)obj.Indices32.size() * sizeof(UINT16);

		ro.m_mesh->CreateVertexBuffers(GetDevice(), obj.Vertices.data(), vbByteSize, sizeof(GeometryGenerator::Vertex));
		ro.m_mesh->CreateIndexBuffers(GetDevice(), obj.GetIndices16().data(), ibByteSize, FORMAT_R16_UINT);

		Submesh submesh;
		submesh.IndexCount = (UINT)obj.Indices32.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;
		submesh.Bounds = obj.BBox;

		ro.m_mesh->m_drawArgs.reserve(1);
		ro.m_mesh->m_drawArgs.push_back(submesh);
		ro.m_color = float3(1, 1, 1);
	}
	m_renderObjects.push_back(ro);
}

void CreviceWindow::ClearScene()
{
	m_renderObjects.clear();
}

void CreviceWindow::InitializeRenderObjects()
{
	m_grid.Initialize();
	m_gizmo.Initialize();

	// Scene
	AddObject(eObject::Box, 20.0f, 1.0f, 20.0f, 1);
	m_renderObjects[0].SetTranslation(0.0f, -0.5f, 0.0f);
	AddObject(eObject::Sphere, 1.0f, 16, 16);
	m_renderObjects[1].SetTranslation(-5, 1, 0);
	AddObject(eObject::Box, 2.0f, 2.0f, 2.0f, 1);
	m_renderObjects[2].SetTranslation(-2, 1, 0);
	m_renderObjects[2].SetColor(1, 0, 0);
	AddObject(eObject::Cone, 1.0f, 3.0f, 8.0f, 8.0f);
	m_renderObjects[3].SetTranslation(1, 1.5f, 0);
	m_renderObjects[3].SetColor(0, 1, 0);
	AddObject(eObject::Cylinder, 1.0f, 3.0f, 8.0f, 8.0f);
	m_renderObjects[4].SetTranslation(4, 1.5f, 0);
	m_renderObjects[4].SetColor(0, 0, 1);
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
	m_model[EMT_Sphere].SetScale(0.1f, 0.1f, 0.1f);

	m_model[EMT_Cerberus].m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/cerberus.fbx");
	m_model[EMT_Cerberus].SetScale(0.1f, 0.1f, 0.1f);

	m_skybox.m_mesh = Mesh::FromFile(GetDevice(), "Data/Meshes/skybox.obj");
	m_skybox.SetScale(100.0f, 100.0f, 100.0f);
}

void CreviceWindow::InitializeConstantBuffers()
{
	if (m_objVsCB == nullptr)
	{
		m_objVsCB = new Graphics::GPUBuffer();

		ObjectConstantsVS objCB;
		ZeroMemory(&objCB, sizeof(objCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(ObjectConstantsVS);
		SubresourceData initData;
		initData.SysMem = &objCB;
		GetDevice().CreateBuffer(bd, &initData, m_objVsCB);
		GetDevice().TransitionBarrier(m_objVsCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (m_objPsCB == nullptr)
	{
		m_objPsCB = new Graphics::GPUBuffer();

		ObjectConstantsPS objCB;
		ZeroMemory(&objCB, sizeof(objCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(ObjectConstantsPS);
		SubresourceData initData;
		initData.SysMem = &objCB;
		GetDevice().CreateBuffer(bd, &initData, m_objPsCB);
		GetDevice().TransitionBarrier(m_objPsCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
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

		XMVECTOR eyePos = XMLoadFloat3(&cam->m_eyePos);

		ShadingConstants shadingCB;
		ZeroMemory(&shadingCB, sizeof(shadingCB));
		XMStoreFloat4(&shadingCB.EyePosition, eyePos);

		Light globalLight;
		float3 sunDir = m_globalLight.GetSunDirection(UIContext::Time);
		globalLight.Direction = float4(sunDir.x, sunDir.y, sunDir.z, 0.0f);
		shadingCB.GlobalLight.Direction = globalLight.Direction;
		float3 sunColor = UIContext::LightColor;
		globalLight.Color = float4(sunColor.x * UIContext::LightIntensity, sunColor.y * UIContext::LightIntensity, sunColor.z * UIContext::LightIntensity, 1.0f);
		shadingCB.GlobalLight.Color = globalLight.Color;
		
		shadingCB.MousePos = float2((float)m_lastMousePos.x, (float)m_lastMousePos.y);

		GetDevice().UpdateBuffer(m_shadingCB, &shadingCB, sizeof(ShadingConstants));
	}

	if (m_backgroundCB != nullptr)
	{
		BackgroundConstants backgroundCB;
		ZeroMemory(&backgroundCB, sizeof(backgroundCB));

		{
			const Camera* cam = GetCamera();

			XMMATRIX view = XMLoadFloat4x4(&cam->m_view);
			XMMATRIX proj = XMLoadFloat4x4(&cam->m_proj);
			XMMATRIX worldToScreen = view * proj;
			XMMATRIX screenToWorld = XMMatrixInverse(&XMMatrixDeterminant(worldToScreen), worldToScreen);
			XMStoreFloat4x4(&backgroundCB.ScreenToWorld, XMMatrixTranspose(screenToWorld));
			XMStoreFloat4x4(&backgroundCB.ViewProj, XMMatrixTranspose(worldToScreen));

			backgroundCB.ScreenDimensions = float4((float)GetWidth(), (float)GetHeight(), tanf(0.5f * MathHelper::Deg2Rad(cam->m_fov)), (float)GetHeight() / (float)GetWidth());
		}

		{
			GlobalLightCamera lightCamera(m_globalLight.GetSunDirection(UIContext::Time));

			XMMATRIX view = lightCamera.m_view;
			XMMATRIX proj = lightCamera.m_proj;

			XMMATRIX lightViewProj = view * proj;
			XMStoreFloat4x4(&backgroundCB.LightViewProj, XMMatrixTranspose(lightViewProj));
		}

		{
			XMMATRIX cubeRotation = XMMatrixRotationRollPitchYaw(0.f, MathHelper::Deg2Rad((float)UIContext::CubemapRotation), 0.f);
			XMStoreFloat4x4(&backgroundCB.CubemapRotation, XMMatrixTranspose(cubeRotation));
		}

		GetDevice().UpdateBuffer(m_backgroundCB, &backgroundCB, sizeof(BackgroundConstants));
	}
}

void CreviceWindow::UpdateObjectConstantBuffer(const RenderObject& renderObject, UINT objectID)
{
	if (m_objVsCB != nullptr)
	{
		const Camera* cam = GetCamera();

		XMMATRIX world = XMLoadFloat4x4(&renderObject.GetWorld());
		XMMATRIX view = XMLoadFloat4x4(&cam->m_view);
		XMMATRIX proj = XMLoadFloat4x4(&cam->m_proj);
		XMMATRIX worldViewProj = world * view * proj;
		XMVECTOR eyePos = XMLoadFloat3(&cam->m_eyePos);

		ObjectConstantsVS objCB;
		ZeroMemory(&objCB, sizeof(objCB));
		// Update constant buffer with the latest worldViewProj matrix
		XMStoreFloat4x4(&objCB.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objCB.Scene, XMMatrixTranspose(view));
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));
		GetDevice().UpdateBuffer(m_objVsCB, &objCB, sizeof(ObjectConstantsVS));
	}

	if (m_objPsCB != nullptr)
	{
		ObjectConstantsPS objCB;
		ZeroMemory(&objCB, sizeof(objCB));

		objCB.Color = renderObject.m_color;
		objCB.Roughness = renderObject.m_roughness;
		objCB.Metalness = renderObject.m_metalness;
		objCB.ObjectID = objectID;

		GetDevice().UpdateBuffer(m_objPsCB, &objCB, sizeof(ObjectConstantsPS));
	}
}

void CreviceWindow::UpdateObjectConstantBufferShadows(const RenderObject& renderObject, const GlobalLightCamera& lightCamera)
{
	if (m_objVsCB != nullptr)
	{
		const Camera* cam = GetCamera();

		XMMATRIX world = XMLoadFloat4x4(&renderObject.GetWorld());

		XMMATRIX worldViewProj = world * lightCamera.m_view * lightCamera.m_proj;

		ObjectConstantsVS objCB;
		ZeroMemory(&objCB, sizeof(objCB));
		// Update constant buffer with the latest worldViewProj matrix
		XMStoreFloat4x4(&objCB.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objCB.Scene, XMMatrixTranspose(lightCamera.m_view));
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));
		GetDevice().UpdateBuffer(m_objVsCB, &objCB, sizeof(ObjectConstantsVS));
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 