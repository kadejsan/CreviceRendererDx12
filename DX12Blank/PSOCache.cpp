#include "stdafx.h"
#include "PSOCache.h"
#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

using namespace GraphicsTypes;

PSOCache::PSOCache()
{

}

PSOCache::~PSOCache()
{

}

void PSOCache::Initialize(GraphicsTypes::GraphicsDevice& device)
{
	m_cache.resize(PSO_MAX);

	// SimpleColorSolid
	GraphicsPSODesc psoDesc = {};
	psoDesc.VS = new VertexShader();
	psoDesc.PS = new PixelShader();
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.VS);
	device.CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.PS);
	{
		VertexInputLayoutDesc layoutDesc[2] =
		{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, FORMAT_R32G32B32A32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 }
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
}
