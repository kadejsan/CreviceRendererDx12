#include "stdafx.h"
#include "SamplerCache.h"
#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

using namespace Graphics;

SamplerCache::SamplerCache()
{

}

SamplerCache::~SamplerCache()
{

}

void SamplerCache::Initialize(Graphics::GraphicsDevice& device)
{
	m_cache.resize(SS_MAX);

	SamplerDesc samplerDesc;
	samplerDesc.Filter = FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = FLOAT32_MAX;
	m_cache[LinearClamp] = std::make_unique<Sampler>();
	device.CreateSamplerState(&samplerDesc, m_cache[LinearClamp].get());
	
	samplerDesc.Filter = FILTER_ANISOTROPIC;
	m_cache[AnisotropicClamp] = std::make_unique<Sampler>();
	device.CreateSamplerState(&samplerDesc, m_cache[AnisotropicClamp].get());

	samplerDesc.Filter = FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = TEXTURE_ADDRESS_WRAP;
	m_cache[LinearWrap] = std::make_unique<Sampler>();
	device.CreateSamplerState(&samplerDesc, m_cache[LinearWrap].get());

	samplerDesc.Filter = FILTER_ANISOTROPIC;
	m_cache[AnisotropicWrap] = std::make_unique<Sampler>();
	device.CreateSamplerState(&samplerDesc, m_cache[AnisotropicWrap].get());
}
