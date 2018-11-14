#include "stdafx.h"
#include "PSOCache.h"
#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

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
	psoDesc.RTFormats[0] = device.GetBackBufferFormat();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSFormat = device.GetDepthStencilFormat();

	m_cacheGraphics[SimpleColorSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleColorSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[SimpleColorWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[SimpleColorWireframe].get());

	// PBR
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\PBR.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\PBR.hlsl", psoDesc.PS);
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
	m_cacheGraphics[PBRSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cacheGraphics[PBRWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cacheGraphics[PBRWireframe].get());

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
}

void PSOCache::InitializeCompute(Graphics::GraphicsDevice& device)
{
	m_cacheCompute.resize(CPSO_MAX);

	ComputePSODesc psoDesc = {};
	psoDesc.CS = new ComputeShader();
	device.CreateShader(L"Shaders\\Equirect2Cube.hlsl", psoDesc.CS);
	m_cacheCompute[Equirect2Cube] = std::make_unique<ComputePSO>();
	device.CreateComputePSO(&psoDesc, m_cacheCompute[Equirect2Cube].get());
}