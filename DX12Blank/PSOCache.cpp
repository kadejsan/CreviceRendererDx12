#include "stdafx.h"
#include "PSOCache.h"
#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"
#include "Renderer.h"

using namespace Graphics;

PSOCache::PSOCache()
{

}

PSOCache::~PSOCache()
{
}

void PSOCache::Initialize(Graphics::GraphicsDevice& device)
{
	InitializeGraphics(device);
	InitializeCompute(device);
	if(device.SupportRayTracing()) InitializeRayTrace(device);
}


void PSOCache::Clean()
{
	m_cacheGraphics.clear();
	m_cacheCompute.clear();
}

void PSOCache::InitializeGraphics(Graphics::GraphicsDevice& device)
{
	m_cacheGraphics.resize(GPSO_MAX);

	// SimpleColorSolid
	GraphicsPSODesc psoDesc = {};
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\Grid.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\Grid.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[2] =
		{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
		};
		psoDesc.IL = new VertexLayout();
		device.CreateInputLayout(layoutDesc, 2, psoDesc.IL);
	}
	psoDesc.RS = new RasterizerState();
	psoDesc.BS = new BlendState();
	psoDesc.DSS = new DepthStencilState();
	psoDesc.DSS->m_desc.DepthEnable = true;
	psoDesc.DSS->m_desc.DepthWriteMask = DEPTH_WRITE_MASK_ALL;
	psoDesc.DSS->m_desc.DepthFunc = COMPARISON_LESS;
	psoDesc.PT = PRIMITIVETOPOLOGY::LINELIST;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.NumRTs = 3;
	psoDesc.RTFormats[0] = Renderer::RTFormat_GBuffer0;
	psoDesc.RTFormats[1] = Renderer::RTFormat_GBuffer1;
	psoDesc.RTFormats[2] = Renderer::RTFormat_GBuffer2;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSFormat = Renderer::DSFormat_Full;

	m_cacheGraphics[GridSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[GridSolid].get());

	// PBR
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\PBRMaterial.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\PBRMaterial.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[5] =
		{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 36, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 48, INPUT_PER_VERTEX_DATA, 0 }
		};
		psoDesc.IL = new VertexLayout();
		device.CreateInputLayout(layoutDesc, 5, psoDesc.IL);
	}
	psoDesc.PT = PRIMITIVETOPOLOGY::TRIANGLELIST;
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	psoDesc.NumRTs = 3;
	psoDesc.RTFormats[0] = Renderer::RTFormat_GBuffer0;
	psoDesc.RTFormats[1] = Renderer::RTFormat_GBuffer1;
	psoDesc.RTFormats[2] = Renderer::RTFormat_GBuffer2;
	m_cacheGraphics[PBRSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[PBRWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRWireframe].get());

	// PBR Simple
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\PBRMaterialSimple.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\PBRMaterialSimple.hlsl", psoDesc.PS);
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	m_cacheGraphics[PBRSimpleSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRSimpleSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[PBRSimpleWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRSimpleWireframe].get());

	// Simple Depth
	psoDesc.VS = new VertexShader();
	psoDesc.PS = nullptr;
	device.CreateShader(L"Shaders\\SimpleDepth.hlsl", psoDesc.VS);
	psoDesc.DSS->m_desc.DepthEnable = true;
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	psoDesc.DSS->m_desc.DepthFunc = COMPARISON_GREATER;
	m_cacheGraphics[SimpleDepthSelection] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleDepthSelection].get());

	psoDesc.DSS->m_desc.DepthFunc = COMPARISON_LESS;
	m_cacheGraphics[SimpleDepthShadow] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleDepthShadow].get());

	// Skybox
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\Skybox.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\Skybox.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[5] =
		{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
		};
		psoDesc.IL = new VertexLayout();
		device.CreateInputLayout(layoutDesc, 1, psoDesc.IL);
	}
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	m_cacheGraphics[SkyboxSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SkyboxSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[SkyboxWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SkyboxWireframe].get());

	// Tonemapping
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\Tonemapping.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\Tonemapping.hlsl", psoDesc.PS);
	psoDesc.IL = nullptr;
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	psoDesc.RS->m_desc.FrontCounterClockwise = true;
	psoDesc.RTFormats[0] = device.GetBackBufferFormat();
	psoDesc.DSS->m_desc.DepthEnable = false;
	psoDesc.DSFormat = FORMAT_UNKNOWN;
	m_cacheGraphics[TonemappingReinhard] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[TonemappingReinhard].get());

	// Linearize Depth
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\LinearizeDepth.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\LinearizeDepth.hlsl", psoDesc.PS);
	psoDesc.RTFormats[0] = Renderer::RTFormat_LinearDepth;
	m_cacheGraphics[LinearizeDepth] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[LinearizeDepth].get());

	// Background
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\Background.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\Background.hlsl", psoDesc.PS);
	psoDesc.DSS->m_desc.DepthEnable = false;
	psoDesc.DSS->m_desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	psoDesc.NumRTs = 1;
	psoDesc.RTFormats[0] = Renderer::RTFormat_HDR;
	m_cacheGraphics[Background] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[Background].get());

	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\LightingPass.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\LightingPass.hlsl", psoDesc.PS);
	m_cacheGraphics[LightingPass] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[LightingPass].get());

	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\SobelFilter.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\SobelFilter.hlsl", psoDesc.PS);
	psoDesc.RTFormats[0] = Renderer::RTFormat_LDR;
	m_cacheGraphics[SobelFilter] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SobelFilter].get());

	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\AmbientOcclusion.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\AmbientOcclusion.hlsl", psoDesc.PS);
	psoDesc.RTFormats[0] = Renderer::RTFormat_AO;
	m_cacheGraphics[AmbientOcclusionPass] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[AmbientOcclusionPass].get());

	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\Blur.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\Blur.hlsl", psoDesc.PS);
	psoDesc.RTFormats[0] = Renderer::RTFormat_AO;
	m_cacheGraphics[Blur] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[Blur].get());

	// Simple Color
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[5] =
		{
		{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 36, INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 48, INPUT_PER_VERTEX_DATA, 0 }
		};
		psoDesc.IL = new VertexLayout();
		device.CreateInputLayout(layoutDesc, 5, psoDesc.IL);
	}
	psoDesc.NumRTs = 1;
	psoDesc.RTFormats[0] = Renderer::RTFormat_HDR;
	psoDesc.RS->m_desc.FillMode = FILL_SOLID;
	psoDesc.DSS->m_desc.DepthEnable = true;
	psoDesc.DSS->m_desc.DepthFunc = COMPARISON_LESS;
	psoDesc.DSFormat = Renderer::DSFormat_Full;
	psoDesc.RS->m_desc.FrontCounterClockwise = true;
	m_cacheGraphics[GizmoSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[GizmoSolid].get());
}

void PSOCache::InitializeCompute(Graphics::GraphicsDevice& device)
{
	m_cacheCompute.resize(CPSO_MAX);

	ComputePSODesc psoDesc = {};
	psoDesc.CS = new ComputeShader();
	device.CreateShader(L"Shaders\\Equirect2Cube.hlsl", psoDesc.CS);
	m_cacheCompute[Equirect2Cube] = std::make_unique<ComputePSO>();
	device.CreateComputePSO(&psoDesc, m_cacheCompute[Equirect2Cube].get());

	psoDesc.CS = new ComputeShader();
	device.CreateShader(L"Shaders\\Spmap.hlsl", psoDesc.CS);
	m_cacheCompute[SpecularEnvironmentMap] = std::make_unique<ComputePSO>();
	device.CreateComputePSO(&psoDesc, m_cacheCompute[SpecularEnvironmentMap].get());

	psoDesc.CS = new ComputeShader();
	device.CreateShader(L"Shaders\\Irrmap.hlsl", psoDesc.CS);
	m_cacheCompute[IrradianceMap] = std::make_unique<ComputePSO>();
	device.CreateComputePSO(&psoDesc, m_cacheCompute[IrradianceMap].get());

	psoDesc.CS = new ComputeShader();
	device.CreateShader(L"Shaders\\Spbrdf.hlsl", psoDesc.CS);
	m_cacheCompute[SpecularBRDFLut] = std::make_unique<ComputePSO>();
	device.CreateComputePSO(&psoDesc, m_cacheCompute[SpecularBRDFLut].get());
}

void PSOCache::InitializeRayTrace(Graphics::GraphicsDevice& device)
{
	m_cacheRayTrace.resize(RTPSO_MAX);

	RayTracePSODesc psoDesc = {};
	psoDesc.RGS = new RayGenerationShader();
	psoDesc.MS = new RayMissShader();
	psoDesc.CHS = new RayClosestHitShader();
	psoDesc.HitGroup = new HitGroup();
	psoDesc.HitGroup->m_desc.HitGroupName = L"HitGroup";
	psoDesc.HitGroup->m_desc.ClosestHitSymbol = L"ClosestHit";
	psoDesc.MaxPayloadSize = 10 * sizeof(float); // RGB and HitT, normal, roughness, metalness, ID
	psoDesc.MaxAttributeSize = 2 * sizeof(float); // barycentric coordinates
	psoDesc.MaxRecursionDepth = 1;
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialSimpleRT.hlsl", psoDesc.RGS);
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialSimpleRT.hlsl", psoDesc.MS);
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialSimpleRT.hlsl", psoDesc.CHS);
	m_cacheRayTrace[PBRSimpleSolidRT] = std::make_unique<RayTracePSO>();
	device.CreateRayTracePSO(&psoDesc, m_cacheRayTrace[PBRSimpleSolidRT].get());

	psoDesc.RGS = new RayGenerationShader();
	psoDesc.MS = new RayMissShader();
	psoDesc.CHS = new RayClosestHitShader();
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialRT.hlsl", psoDesc.RGS);
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialRT.hlsl", psoDesc.MS);
	device.CreateShader(L"Shaders\\RayTracing\\PBRMaterialRT.hlsl", psoDesc.CHS);
	m_cacheRayTrace[PBRSolidRT] = std::make_unique<RayTracePSO>();
	device.CreateRayTracePSO(&psoDesc, m_cacheRayTrace[PBRSolidRT].get());
}
