
#include "stdafx.h"
#include "Grid.h"
#include "Mesh.h"
#include "Renderer.h"

Grid::Grid()
{

}

Grid::~Grid()
{
	delete m_vb;
	delete m_cb;
}

void Grid::Initialize()
{
	float col = 1.0f;
	m_gridVertexCount = 0;

	const float h = 0.01f; // avoid z-fight with zero plane
	const int a = 20;
	XMFLOAT3 verts[((a + 1) * 2 + (a + 1) * 2) * 2];

	int count = 0;
	for (int i = 0; i <= a; ++i)
	{
		verts[count++] = XMFLOAT3(i - a * 0.5f, h, -a * 0.5f);
		verts[count++] = (i == a / 2 ? XMFLOAT3(0, 0, 1) : XMFLOAT3(col, col, col));

		verts[count++] = XMFLOAT3(i - a * 0.5f, h, +a * 0.5f);
		verts[count++] = (i == a / 2 ? XMFLOAT3(0, 0, 1) : XMFLOAT3(col, col, col));
	}
	for (int j = 0; j <= a; ++j)
	{
		verts[count++] = XMFLOAT3(-a * 0.5f, h, j - a * 0.5f);
		verts[count++] = (j == a / 2 ? XMFLOAT3(1, 0, 0) : XMFLOAT3(col, col, col));

		verts[count++] = XMFLOAT3(+a * 0.5f, h, j - a * 0.5f);
		verts[count++] = (j == a / 2 ? XMFLOAT3(1, 0, 0) : XMFLOAT3(col, col, col));
	}

	m_gridVertexCount = ARRAYSIZE(verts) / 2;

	// VB
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(verts);
		bd.BindFlags = BIND_VERTEX_BUFFER;
		bd.CpuAccessFlags = 0;
		SubresourceData InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.SysMem = verts;
		m_vb = new GPUBuffer;
		(*Renderer::GetDevice()).CreateBuffer(bd, &InitData, m_vb);
	}

	// CB
	{
		m_cb = new Graphics::GPUBuffer();

		GridCB objCB;
		ZeroMemory(&objCB, sizeof(objCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(GridCB);
		SubresourceData initData;
		initData.SysMem = &objCB;
		(*Renderer::GetDevice()).CreateBuffer(bd, &initData, m_cb);
		(*Renderer::GetDevice()).TransitionBarrier(m_cb, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void Grid::Render()
{
	GPUBuffer* vbs[] = {
		m_vb,
	};
	const UINT strides[] = {
		sizeof(XMFLOAT3) + sizeof(XMFLOAT3),
	};
	(*Renderer::GetDevice()).BindVertexBuffers(vbs, 0, ARRAYSIZE(vbs), strides, nullptr);
	(*Renderer::GetDevice()).Draw(m_gridVertexCount, 0);
}