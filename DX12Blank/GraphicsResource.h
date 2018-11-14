#pragma once

using namespace Microsoft::WRL;

class GPUResource
{
public:
	ComPtr< ID3D12Resource > m_resourceDX12;

	GPUResource();
	virtual ~GPUResource();
};

// - - - - - - - - - - - - - - - - - - - - - - - -

class GPUBuffer : public GPUResource
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE * m_cbvDX12;

public:
	GPUBuffer();
	virtual ~GPUBuffer();
};

// - - - - - - - - - - - - - - - - - - - - - - - -

class Texture : public GPUResource
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE * m_rtvDX12;

public:
	Texture();
	virtual ~Texture();
};