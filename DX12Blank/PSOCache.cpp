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
	m_cache.resize(PSO_MAX);

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

	m_cache[SimpleColorSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cache[SimpleColorSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cache[SimpleColorWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cache[SimpleColorWireframe].get());

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
	m_cache[PBRSolid] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cache[PBRSolid].get());

	psoDesc.RS->m_desc.FillMode = FILL_WIREFRAME;
	m_cache[PBRWireframe] = std::make_unique<GraphicsPSO>();
	device.CreateGraphicsPSO(&psoDesc, m_cache[PBRWireframe].get());
}
