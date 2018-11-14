#include "stdafx.h"
#include "GraphicsResource.h"

GPUResource::GPUResource()
{
}

GPUResource::~GPUResource()
{
	m_resourceDX12.Reset();
}

// - - - - - - - - - - - - - - - - - - - -

GPUBuffer::GPUBuffer() 
	: GPUResource()
{
	SAFE_INIT(m_cbvDX12);
}
GPUBuffer::~GPUBuffer()
{
	SAFE_DELETE(m_cbvDX12);
}

// - - - - - - - - - - - - - - - - - - - -

Texture::Texture() 
	: GPUResource()
{
	SAFE_INIT(m_rtvDX12);
}
Texture::~Texture()
{
	SAFE_DELETE(m_rtvDX12);
}