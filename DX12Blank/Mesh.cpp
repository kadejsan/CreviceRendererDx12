#include "stdafx.h"
#include "Mesh.h"

void Mesh::Draw(GraphicsTypes::GraphicsDevice& device)
{
	GPUBuffer* vertexBufs[] = { &m_vertexBufferGPU };
	const UINT strides[] = { m_vertexBufferGPU.m_desc.StructureByteStride };
	device.BindVertexBuffers(vertexBufs, 0, 1, strides);
	device.BindIndexBuffer(&m_indexBufferGPU, m_indexBufferGPU.m_desc.Format, 0);

	for( auto& submesh : m_drawArgs )
		device.DrawIndexed(submesh.IndexCount, submesh.StartIndexLocation, submesh.BaseVertexLocation);
}

void Mesh::CreateVertexBuffers(GraphicsTypes::GraphicsDevice& device, void* data, UINT size, UINT stride)
{
	device.CreateBlob(size, &m_vertexBufferCPU);
	CopyMemory(m_vertexBufferCPU.m_blob->GetBufferPointer(), data, size);

	// vertex buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = size;
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

void Mesh::CreateIndexBuffers(GraphicsTypes::GraphicsDevice& device, void* data, UINT size)
{
	device.CreateBlob(size, &m_indexBufferCPU);
	CopyMemory(m_indexBufferCPU.m_blob->GetBufferPointer(), data, size);

	// index buffer
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DEFAULT;
		bd.ByteWidth = size;
		bd.BindFlags = BIND_INDEX_BUFFER;
		bd.CpuAccessFlags = 0;
		bd.Format = FORMAT_R16_UINT;
		bd.StructureByteStride = sizeof(UINT16);

		m_indexBufferGPU.m_desc = bd;

		SubresourceData initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.SysMem = data;

		device.CreateBuffer(bd, &initData, &m_indexBufferGPU);

		device.TransitionBarrier(&m_indexBufferGPU, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_GENERIC_READ);
	}
}
