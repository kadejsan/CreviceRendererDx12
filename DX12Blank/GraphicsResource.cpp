#include "stdafx.h"
#include "GraphicsResource.h"

namespace Graphics
{
	GPUResource::GPUResource()
	{
		SAFE_INIT(m_srv);
		SAFE_INIT(m_uav);
	}

	GPUResource::~GPUResource()
	{
		SAFE_RELEASE(m_resource);
		SAFE_DELETE(m_srv);
		for (auto& x : m_additionalSRVs)
		{
			SAFE_DELETE(x);
		}
		SAFE_DELETE(m_uav);
		for (auto& x : m_additionalUAVs)
		{
			SAFE_DELETE(x);
		}
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
		, m_independentRTVArraySlices(false)
		, m_independentRTVCubemapFaces(false)
		, m_independentSRVArraySlices(false)
		, m_independentSRVMIPs(false)
		, m_independentUAVMIPs(false)
	{
		SAFE_INIT(m_rtv);
	}
	Texture::~Texture()
	{
		SAFE_DELETE(m_rtv);
		for (auto& x : m_additionalRTVs)
		{
			SAFE_DELETE(x);
		}
	}

	void Texture::RequestIndependentRenderTargetArraySlices(bool value)
	{
		m_independentRTVArraySlices = value;
	}
	void Texture::RequestIndependentRenderTargetCubemapFaces(bool value)
	{
		m_independentRTVCubemapFaces = value;
	}
	void Texture::RequestIndependentShaderResourceArraySlices(bool value)
	{
		m_independentSRVArraySlices = value;
	}
	void Texture::RequestIndependentShaderResourcesForMIPs(bool value)
	{
		m_independentSRVMIPs = value;
	}
	void Texture::RequestIndependentUnorderedAccessResourcesForMIPs(bool value)
	{
		m_independentUAVMIPs = value;
	}

	// - - - - - - - - - - - - - - - - - - - -

	Texture2D::Texture2D()
	{
		SAFE_INIT(m_dsv);
	}

	Texture2D::~Texture2D()
	{
		SAFE_DELETE(m_dsv);
		for (auto& x : m_additionalDSVs)
		{
			SAFE_DELETE(x);
		}
	}

	// - - - - - - - - - - - - - - - - - - - -

	Sampler::Sampler()
	{
		SAFE_INIT(m_resource);
	}

	Sampler::~Sampler()
	{
		SAFE_DELETE(m_resource);
	}

}