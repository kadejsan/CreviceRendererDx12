#include "stdafx.h"
#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

const unsigned int ImportFlags =
	aiProcess_CalcTangentSpace |
	aiProcess_Triangulate |
	aiProcess_SortByPType |
	aiProcess_PreTransformVertices |
	aiProcess_GenNormals |
	aiProcess_GenUVCoords |
	aiProcess_OptimizeMeshes |
	aiProcess_Debone |
	aiProcess_ValidateDataStructure;

void Mesh::Draw(Graphics::GraphicsDevice& device, const float4x4& world)
{
	GPUBuffer* vertexBufs[] = { &m_vertexBufferGPU };
	const UINT strides[] = { m_vertexBufferGPU.m_desc.StructureByteStride };
	device.BindVertexBuffers(vertexBufs, 0, 1, strides);
	device.BindIndexBuffer(&m_indexBufferGPU, m_indexBufferGPU.m_desc.Format, 0);

	for (auto& submesh : m_drawArgs)
	{
		BoundingBox bbox;
		submesh.Bounds.Transform(bbox, XMLoadFloat4x4(&world));
		device.DrawIndexed(submesh.IndexCount, submesh.StartIndexLocation, submesh.BaseVertexLocation);
	}
}

void Mesh::Draw(Graphics::GraphicsDevice& device, const float4x4& world, const Frustum& frustum)
{
	GPUBuffer* vertexBufs[] = { &m_vertexBufferGPU };
	const UINT strides[] = { m_vertexBufferGPU.m_desc.StructureByteStride };
	device.BindVertexBuffers(vertexBufs, 0, 1, strides);
	device.BindIndexBuffer(&m_indexBufferGPU, m_indexBufferGPU.m_desc.Format, 0);

	for( auto& submesh : m_drawArgs )
	{
		BoundingBox bbox;
		submesh.Bounds.Transform(bbox, XMLoadFloat4x4(&world));
		if( frustum.CheckBox(bbox) )
		{
			device.DrawIndexed(submesh.IndexCount, submesh.StartIndexLocation, submesh.BaseVertexLocation);
		}
	}
}

void Mesh::CreateVertexBuffers(Graphics::GraphicsDevice& device, void* data, UINT64 size, UINT stride)
{
	device.CreateBlob(size, &m_vertexBufferCPU);
	CopyMemory(m_vertexBufferCPU.m_blob->GetBufferPointer(), data, size);

	// vertex buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = (UINT)size;
		bd.BindFlags = BIND_VERTEX_BUFFER;
		bd.CpuAccessFlags = 0;
		bd.StructureByteStride = stride;

		m_vertexBufferGPU.m_desc = bd;

		SubresourceData initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.SysMem = data;

		device.CreateBuffer(bd, &initData, &m_vertexBufferGPU);

		device.TransitionBarrier(&m_vertexBufferGPU, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void Mesh::CreateIndexBuffers(Graphics::GraphicsDevice& device, void* data, UINT64 size, FORMAT format)
{
	device.CreateBlob(size, &m_indexBufferCPU);
	CopyMemory(m_indexBufferCPU.m_blob->GetBufferPointer(), data, size);

	// index buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = (UINT)size;
		bd.BindFlags = BIND_INDEX_BUFFER;
		bd.CpuAccessFlags = 0;
		bd.Format = format;
		bd.StructureByteStride = sizeof(UINT16);

		m_indexBufferGPU.m_desc = bd;

		SubresourceData initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.SysMem = data;

		device.CreateBuffer(bd, &initData, &m_indexBufferGPU);

		device.TransitionBarrier(&m_indexBufferGPU, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_GENERIC_READ);
	}
}

std::shared_ptr<Mesh> Mesh::FromFile(Graphics::GraphicsDevice& device, const std::string& filename)
{
	std::shared_ptr<Mesh> mesh;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename, ImportFlags);
	if (scene && scene->HasMeshes())
	{
		mesh = std::shared_ptr<Mesh>(new Mesh(device, scene->mMeshes[0]));
	}

	assert(mesh != nullptr);
	return mesh;
}

Mesh::Mesh(Graphics::GraphicsDevice& device, const aiMesh* mesh)
{
	assert(mesh->HasPositions());
	assert(mesh->HasNormals());

	struct Vertex
	{
		float3 Position;
		float3 Normal;
		float3 Tangent;
		float3 Bitangent;
		float2 Texcoord;
	};

	std::vector<Vertex> vertices;
	vertices.reserve(mesh->mNumVertices);

	for (UINT i = 0; i < vertices.capacity(); ++i)
	{
		Vertex vertex;
		vertex.Position = float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Normal = float3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if (mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent = float3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex.Bitangent = float3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
		if (mesh->HasTextureCoords(0))
		{
			vertex.Texcoord = float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		vertices.push_back(vertex);
	}

	UINT stride = sizeof(Vertex);
	CreateVertexBuffers(device, vertices.data(), vertices.size()*(UINT64)stride, stride);

	std::vector<UINT> indices;
	indices.reserve(mesh->mNumFaces * 3);

	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		assert(mesh->mFaces[i].mNumIndices == 3);
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	CreateIndexBuffers(device, indices.data(), indices.size() * (UINT64)sizeof(UINT), FORMAT_R32_UINT);

	Submesh submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.Bounds = BoundingBox();

	m_drawArgs.reserve(1);
	m_drawArgs.push_back(submesh);
}