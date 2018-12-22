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
}

void PSOCache::InitializeGraphics(Graphics::GraphicsDevice& device)
{
	m_cacheGraphics.resize(GPSO_MAX);

	// SimpleColorSolid
	GraphicsPSODesc psoDesc = {};
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[4] =
		{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 36, INPUT_PER_VERTEX_DATA, 0 }
		};
		psoDesc.IL = new VertexLayout();
		device.CreateInputLayout(layoutDesc, 4, psoDesc.IL);
	}
	psoDesc.RS = new RasterizerState();
	psoDesc.BS = new BlendState();
	psoDesc.DSS = new DepthStencilState();
	psoDesc.DSS->m_desc.DepthEnable = true;
	psoDesc.DSS->m_desc.DepthWriteMask = DEPTH_WRITE_MASK_ALL;
	psoDesc.DSS->m_desc.DepthFunc = COMPARISON_LESS;
	psoDesc.PT = PRIMITIVETOPOLOGY::TRIANGLELIST;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.NumRTs = 1;
	psoDesc.RTFormats[0] = Renderer::RTFormat_HDR;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSFormat = Renderer::DSFormat_Full;

	m_cacheGraphics[SimpleColorSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleColorSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[SimpleColorWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleColorWireframe].get());

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