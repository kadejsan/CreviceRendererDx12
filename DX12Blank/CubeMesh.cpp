
#include "stdafx.h"
#include "CubeMesh.h"
#include "GraphicsDevice.h"

CubeMesh::CubeMesh( GraphicsTypes::GraphicsDevice* graphicsDevice )
{
	CreateBoxMesh(graphicsDevice);
}

CubeMesh::~CubeMesh()
{

}

void CubeMesh::CreateBoxMesh( GraphicsTypes::GraphicsDevice* graphicsDevice )
{
	struct Vertex
	{
		float3 Pos;
		float4 Color;
	};

	std::array<Vertex, 8> vertices =
	{
		Vertex({ float3(-1.0f, -1.0f, -1.0f), float4(Colors::White) }),
		Vertex({ float3(-1.0f, +1.0f, -1.0f), float4(Colors::Black) }),
		Vertex({ float3(+1.0f, +1.0f, -1.0f), float4(Colors::Red) }),
		Vertex({ float3(+1.0f, -1.0f, -1.0f), float4(Colors::Green) }),
		Vertex({ float3(-1.0f, -1.0f, +1.0f), float4(Colors::Blue) }),
		Vertex({ float3(-1.0f, +1.0f, +1.0f), float4(Colors::Yellow) }),
		Vertex({ float3(+1.0f, +1.0f, +1.0f), float4(Colors::Cyan) }),
		Vertex({ float3(+1.0f, -1.0f, +1.0f), float4(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT16);

	graphicsDevice->CreateBlob(vbByteSize, &m_vertexBufferCPU);
	CopyMemory(m_vertexBufferCPU.m_blob->GetBufferPointer(), vertices.data(), vbByteSize);
	graphicsDevice->CreateBlob(ibByteSize, &m_indexBufferCPU);
	CopyMemory(m_indexBufferCPU.m_blob->GetBufferPointer(), indices.data(), ibByteSize);

	// vertex buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = vbByteSize;
		bd.BindFlags = BIND_VERTEX_BUFFER;
		bd.CpuAccessFlags = 0;
		bd.StructureByteStride = sizeof(Vertex);

		m_vertexBufferGPU.m_desc = bd;

		SubresourceData initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.SysMem = vertices.data();

		graphicsDevice->CreateBuffer(bd, &initData, &m_vertexBufferGPU);

		graphicsDevice->TransitionBarrier(&m_vertexBufferGPU, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	// index buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = ibByteSize;
		bd.BindFlags = BIND_INDEX_BUFFER;
		bd.CpuAccessFlags = 0;
		bd.Format = FORMAT_R16_UINT;

		m_indexBufferGPU.m_desc = bd;

		SubresourceData initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.SysMem = indices.data();

		graphicsDevice->CreateBuffer(bd, &initData, &m_indexBufferGPU);

		graphicsDevice->TransitionBarrier(&m_indexBufferGPU, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_GENERIC_READ);
	}

	Submesh submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_drawArgs["box"] = submesh;
}

