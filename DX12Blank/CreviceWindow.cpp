
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

CreviceWindow::CreviceWindow(std::string name)
	: BaseWindow(name)
	, m_modelID(EModelType::EMT_Sphere)
	, m_variableShadingRate(0)
	, m_scene(eScene::PBRModel)
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
	if (GetDevice().SupportRayTracing())
		InitializeRayTracingAccelerationStructure();
	GetDevice().PresentEnd();

	Renderer::GGraphicsDevice->Flush();

	UIContext::IsRayTracingSupported = GetDevice().SupportRayTracing();
	UIContext::IsVRSSupported = GetDevice().SupportVariableRateShadingTier2();
}

void CreviceWindow::OnUpdate()
{
	BaseWindow::OnUpdate();

	GetDevice().EnableRayTracing(UIContext::UseRayTracing);

	UIContext::IsRayTracingSupported = m_scene == eScene::PBRModel;
	UIContext::ShowPBRModelCombo = m_scene == eScene::PBRModel;

	m_scene = (eScene)UIContext::Scene;
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

	if (GetDevice().UseRayTracing())
	{
		if (m_modelID != UIContext::PBRModel)
		{
			GetDevice().Flush();

			m_modelID = (EModelType)UIContext::PBRModel;
			delete m_rtAccelerationStructure;
			InitializeRayTracingAccelerationStructure();
		}
	}

	if (GetDevice().SupportVariableRateShadingTier2())
	{
		if (m_variableShadingRate != UIContext::VariableRateShading)
		{
			GetRenderer().InitializeVRSImage(m_width, m_height, ConvertToVariableShadingRate(UIContext::VariableRateShading));
			m_variableShadingRate = UIContext::VariableRateShading;
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
				if (o.IsEnabled() && o.CastsShadows() && frustum.CheckBox(o.m_mesh->GetBoundingBox(o.GetWorld())))
				{
					// TODO: light view matrix, ortho projection matrix
					UpdateObjectConstantBufferShadows(o, lightCamera);
					o.m_mesh->Draw(GetDevice());
				}
			}
		}

		if (GetDevice().UseRayTracing() && m_scene == eScene::PBRModel)
		{
			ScopedTimer perf("Rendering Objects Ray Tracing", Renderer::GGraphicsDevice);

			if (m_scene == eScene::PBRModel)
			{
				GetDevice().BindResource(SHADERSTAGE::RGS, m_rtAccelerationStructure->m_TLASResult, 0, -1, RT_PASS_GBUFFER);
				GetDevice().BindResource(SHADERSTAGE::RGS, &m_model[UIContext::PBRModel].m_mesh->m_vertexBufferGPU, 1, -1, RT_PASS_GBUFFER);
				GetDevice().BindResource(SHADERSTAGE::RGS, &m_model[UIContext::PBRModel].m_mesh->m_indexBufferGPU, 2, -1, RT_PASS_GBUFFER);
				GetRenderer().BindGBufferUAV();
				GetDevice().BindConstantBuffer(SHADERSTAGE::RGS, m_rayTracedGBufferCB, 0, RT_PASS_GBUFFER);
				GetDevice().BindConstantBuffer(SHADERSTAGE::RGS, m_objPsCB, 1, RT_PASS_GBUFFER);
				GetDevice().BindSampler(SHADERSTAGE::RGS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0, RT_PASS_GBUFFER);

				const RenderObject& model = m_model[UIContext::PBRModel];
				UpdateObjectConstantBuffer(model, 1);

				GPUResource* textures[] = {
					m_textures[ETT_Albedo],
					m_textures[ETT_Normal],
					m_textures[ETT_Roughness],
					m_textures[ETT_Metalness],
				};
				GetDevice().BindResources(SHADERSTAGE::RGS, textures, 3, 4, RT_PASS_GBUFFER);

				eRTPSO pso = UIContext::PBRModel == 0 ? eRTPSO::PBRSimpleSolidRT : eRTPSO::PBRSolidRT;
				RayTracePSO* rtpso = GetRenderer().GetPSO(pso);
				GetDevice().BindRayTracePSO(rtpso);

				GetRenderer().RenderRayTracedObjects(pso);

				GetRenderer().BindGBufferUAV(false);
			}
			else if(m_scene == eScene::SimpleSolids)
			{
				// TODO: implement pbr objects
			}
		}
		else
		{
			ScopedTimer perf("Rendering Objects", Renderer::GGraphicsDevice);

			GetRenderer().SetGBuffer(true);

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

			if (m_scene == eScene::PBRModel)
			{
				GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(UIContext::PBRModel == 0 ? eGPSO::PBRSimpleSolid : eGPSO::PBRSolid, UIContext::Wireframe));

				GPUResource* textures[] = {
					m_textures[ETT_Albedo],
					m_textures[ETT_Normal],
					m_textures[ETT_Roughness],
					m_textures[ETT_Metalness],
				};
				GetDevice().BindResources(PS, textures, 0, 4);
				GetDevice().BindSampler(SHADERSTAGE::PS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0);

				VARIABLE_SHADING_RATE rate = ConvertToVariableShadingRate(m_variableShadingRate);
				if (GetDevice().SupportVariableRateShadingTier2() && rate > VRS_1x1)
				{
					GetDevice().SetVariableShadingRateImage(GetRenderer().GetVRSImage(), VRS_COMBINER_OVERRIDE);
				}

				const RenderObject& model = m_model[UIContext::PBRModel];
				UpdateObjectConstantBuffer(model, 1);
				model.m_mesh->Draw(GetDevice());

				if (GetDevice().SupportVariableRateShadingTier2() && rate > VRS_1x1)
				{
					GetDevice().SetVariableShadingRateImage(nullptr, VRS_COMBINER_OVERRIDE);
				}
			}
			else if (m_scene == SimpleSolids)
			{
				GetDevice().BindGraphicsPSO(GetRenderer().GetPSO(eGPSO::PBRSimpleSolid, UIContext::Wireframe));
				GetDevice().BindSampler(SHADERSTAGE::PS, GetRenderer().GetSamplerState(eSamplerState::AnisotropicWrap), 0);

				VARIABLE_SHADING_RATE rate = ConvertToVariableShadingRate(m_variableShadingRate);
				if (GetDevice().SupportVariableRateShadingTier2() && rate > VRS_1x1)
				{
					GetDevice().SetVariableShadingRateImage(GetRenderer().GetVRSImage(), VRS_COMBINER_OVERRIDE);
				}

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

				if (GetDevice().SupportVariableRateShadingTier2() && rate > VRS_1x1)
				{
					GetDevice().SetVariableShadingRateImage(nullptr, VRS_COMBINER_OVERRIDE);
				}
			}

			GetRenderer().SetGBuffer(false);
		}

		{
			ScopedTimer perf("Linearize Depth", Renderer::GGraphicsDevice);

			GetRenderer().LinearizeDepth(*m_camera);
		}

		{
			ScopedTimer perf("Ambient Occlusion", Renderer::GGraphicsDevice);

			if (GetDevice().UseRayTracing())
			{
				GetDevice().BindResource(SHADERSTAGE::RGS, m_rtAccelerationStructure->m_TLASResult, 0, -1, RT_PASS_AMBIENT_OCCLUSION);

				GetRenderer().RenderRayTracedAmbientOcclusion(*m_camera);
			}
			else
			{
				GetRenderer().RenderAmbientOcclusion();
			}
		}

		GetRenderer().SetFrameBuffer(true);

		{
			ScopedTimer perf("Lighting Pass", Renderer::GGraphicsDevice);

			GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_objVsCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_shadingCB, 0);
			GetDevice().BindConstantBuffer(SHADERSTAGE::PS, m_backgroundCB, 1);

			GetRenderer().BindIBL();

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
	Renderer::GGraphicsDevice->Flush();

	// PBR Model
	for (int i = 0; i < ETT_Max; ++i)
		delete m_textures[i];

	// CB
	delete m_objVsCB;
	delete m_objPsCB;
	delete m_shadingCB;
	delete m_backgroundCB;
	delete m_rayTracedGBufferCB;
	if(m_rtAccelerationStructure)
		delete m_rtAccelerationStructure;
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
		else if (key == '0')
		{
			m_renderer->InitializePSO();
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

		const UINT vertexCount = (UINT)obj.Vertices.size();
		const UINT indexCount = (UINT)obj.Indices32.size();

		ro.m_mesh->CreateVertexBuffers(GetDevice(), obj.Vertices.data(), vertexCount, sizeof(GeometryGenerator::Vertex), FORMAT_R32G32B32_FLOAT);
		ro.m_mesh->CreateIndexBuffers(GetDevice(), obj.GetIndices16().data(), indexCount, INDEXFORMAT_16BIT);

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
	m_renderObjects[0].SetCastsShadows(false);
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
	m_model[EMT_Sphere].SetColor(0.5f, 0.5f, 0.5f);
	m_model[EMT_Sphere].SetRoughness(0.0f);
	m_model[EMT_Sphere].SetMetalness(1.0f);

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

	if (m_rayTracedGBufferCB == nullptr)
	{
		m_rayTracedGBufferCB = new Graphics::GPUBuffer();

		RayTracedGBufferCB rayTracedGBufferCB;
		ZeroMemory(&rayTracedGBufferCB, sizeof(rayTracedGBufferCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(RayTracedGBufferCB);
		SubresourceData initData;
		initData.SysMem = &rayTracedGBufferCB;
		GetDevice().CreateBuffer(bd, &initData, m_rayTracedGBufferCB);
		GetDevice().TransitionBarrier(m_rayTracedGBufferCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_GENERIC_READ);
	}
}

void CreviceWindow::InitializeRayTracingAccelerationStructure()
{
	m_rtAccelerationStructure = new RayTracingAccelerationStructure();

	if (m_scene == eScene::PBRModel)
	{
		const RenderObject& ro = m_model[UIContext::PBRModel];
		Mesh* mesh = ro.m_mesh.get();

		RayTracingAccelerationStructureDesc desc;
		desc.Flags = AS_BUILD_FLAG_PREFER_FAST_TRACE;

		// Geometry Desc
		RayTracingAccelerationStructureDesc::BottomLevelAccelerationStructure::Geometry& geometry = desc.BottomLevelAS.Geometries.emplace_back();
		geometry.Type = AS_BOTTOM_LEVEL_GEOMETRY_TYPE_TRIANGLES;
		geometry.Flags = AS_BOTTOM_LEVEL_GEOMETRY_FLAG_OPAQUE;
		geometry.Triangles.VertexBufferGPUVirtualAddress = mesh->m_vertexBufferGPU.m_resource->GetGPUVirtualAddress();
		geometry.Triangles.VertexStride = mesh->m_vertexStride;
		geometry.Triangles.VertexCount = mesh->m_vertexCount;
		geometry.Triangles.VertexFormat = mesh->m_vertexFormat;
		geometry.Triangles.IndexBufferGPUVirtualAddress = mesh->m_indexBufferGPU.m_resource->GetGPUVirtualAddress();
		geometry.Triangles.IndexCount = mesh->m_indexCount;
		geometry.Triangles.IndexFormat = mesh->m_indexFormat;

		// Bottom Level
		desc.Type = AS_TYPE_BOTTOMLEVEL;
		GetDevice().TransitionBarrier(&mesh->m_vertexBufferGPU, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, RESOURCE_STATE_GENERIC_READ);
		GetDevice().CreateRaytracingAccelerationStructure(desc, m_rtAccelerationStructure);

		// Instance Desc
		RayTracingAccelerationStructureDesc::TopLevelAccelerationStructure::Instance& instance = desc.TopLevelAS.Instances.emplace_back();
		const float4x4& world = ro.GetWorld();
		instance.Transform = XMFLOAT3X4(
			world._11, world._21, world._31, world._41,
			world._12, world._22, world._32, world._42,
			world._13, world._23, world._33, world._43
		);
		instance.InstanceID = 0;
		instance.InstanceMask = 0xFF;
		instance.InstanceContributionToHitGroupIndex = 0;

		// Top Level
		desc.Type = AS_TYPE_TOPLEVEL;
		GetDevice().CreateRaytracingAccelerationStructure(desc, m_rtAccelerationStructure);
	}
	else if (m_scene == eScene::SimpleSolids)
	{
		// Todo: implement pbr simple solids
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
			backgroundCB.EnableSSAO = UIContext::EnableSSAO;
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

	if (m_rayTracedGBufferCB != nullptr)
	{
		RayTracedGBufferCB rayTracedGBufferCB;
		ZeroMemory(&rayTracedGBufferCB, sizeof(rayTracedGBufferCB));

		{
			const Camera* cam = GetCamera();

			const RenderObject& model = m_model[UIContext::PBRModel];
			XMMATRIX world = XMLoadFloat4x4(&model.GetWorld());
			XMStoreFloat4x4(&rayTracedGBufferCB.World, world);

			XMMATRIX view = XMLoadFloat4x4(&cam->m_view);
			XMStoreFloat4x4(&rayTracedGBufferCB.View, view);

			XMVECTOR eyePos = XMLoadFloat3(&cam->m_eyePos);
			XMStoreFloat4(&rayTracedGBufferCB.EyePos, eyePos);

			rayTracedGBufferCB.ResolutionTanHalfFovYAndAspectRatio = float4((float)GetWidth(), (float)GetHeight(), tanf(0.5f * cam->m_fov), m_aspectRatio);
			rayTracedGBufferCB.CameraNearFar = float2(cam->m_nearZ, cam->m_farZ);
		}

		GetDevice().UpdateBuffer(m_rayTracedGBufferCB, &rayTracedGBufferCB, sizeof(RayTracedGBufferCB));
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

Graphics::VARIABLE_SHADING_RATE CreviceWindow::ConvertToVariableShadingRate(int uiRate)
{
	switch (uiRate)
	{
	case 0: return VRS_1x1;
	case 1: return VRS_1x2;
	case 2: return VRS_2x1;
	case 3: return VRS_2x2;
	case 4: return VRS_2x4;
	case 5: return VRS_4x2;
	case 6: return VRS_4x4;
	default: return VRS_1x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 