#pragma once

#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

using namespace GraphicsTypes;

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct Submesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};

struct Mesh
{
	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	CPUBuffer	m_vertexBufferCPU;
	CPUBuffer	m_indexBufferCPU;

	GPUBuffer	m_vertexBufferGPU;
	GPUBuffer	m_indexBufferGPU;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, Submesh> m_drawArgs;
};