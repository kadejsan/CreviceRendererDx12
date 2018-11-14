
#include "stdafx.h"
#include "MeshGenerator.h"
#include "GraphicsDevice.h"

Mesh MeshGenerator::CreateBox( GraphicsTypes::GraphicsDevice& graphicsDevice )
{
	Mesh meshData;

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

	meshData.CreateVertexBuffers(graphicsDevice, vertices.data(), vbByteSize, sizeof(Vertex));
	meshData.CreateIndexBuffers(graphicsDevice, indices.data(), vbByteSize);

	Submesh submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	meshData.m_drawArgs["box"] = submesh;

	return meshData;
}

