#include "stdafx.h"
#include "GraphicsResource.h"

namespace GraphicsTypes
{
	GPUResource::GPUResource()
	{
		SAFE_INIT(m_srv);
		SAFE_INIT(m_uav);
	}

	GPUResource::~GPUResource()
	{
		m_resource.Reset();
		SAFE_DELETE(m_srv);
		SAFE_DELETE(m_uav);
	}

	// - - - - - - - - - - - - - - - - - - - -

	GPUBuffer::GPUBuffer()
		: GPUResource()
	{
		SAFE_INIT(m_cbv);
	}
	GPUBuffer::~GPUBuffer()
	{
		SAFE_DELETE(m_cbv);
	}

	// - - - - - - - - - - - - - - - - - - - -

	Texture::Texture()
		: GPUResource()
	{
		SAFE_INIT(m_rtv);
	}
	Texture::~Texture()
	{
		SAFE_DELETE(m_rtv);
	}

	// - - - - - - - - - - - - - - - - - - - -
}