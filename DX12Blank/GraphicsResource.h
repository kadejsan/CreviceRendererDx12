#pragma once

#include "Include_DX12.h"
#include "GraphicsDescriptors.h"

using namespace Microsoft::WRL;

namespace GraphicsTypes
{
	class GPUResource
	{
	public:
		ComPtr< ID3D12Resource >		m_resource;

		D3D12_CPU_DESCRIPTOR_HANDLE*	m_srv;
		D3D12_CPU_DESCRIPTOR_HANDLE*	m_uav;

		GPUResource();
		virtual ~GPUResource();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class GPUBuffer : public GPUResource
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE*	m_cbv;

	public:
		GPUBufferDesc					m_desc;

	public:
		GPUBuffer();
		virtual ~GPUBuffer();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class Texture : public GPUResource
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE * m_rtv;

	public:
		Texture();
		virtual ~Texture();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class CPUResource
	{
	public:
		CPUResource() {};
		virtual ~CPUResource() {};
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class CPUBuffer : public CPUResource
	{
	public:
		ComPtr<ID3DBlob> m_blob;

	public:
		CPUBuffer() {};
		virtual ~CPUBuffer() {};
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class BaseShader
	{
	public:
		ComPtr<ID3DBlob> m_blob;

		virtual SHADERSTAGE GetShaderStage() const = 0;

	public:
		inline BYTE* GetBufferPtr() { return reinterpret_cast<BYTE*>(m_blob->GetBufferPointer()); }
		inline UINT64 GetSize() const { return m_blob->GetBufferSize(); }
	};

	class VertexShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::VS; }
	};

	class PixelShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::PS; }
	};

	class GeometryShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::GS; }
	};

	class HullShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::HS; }
	};

	class DomainShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::DS; }
	};

	class ComputeShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::CS; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class VertexLayout
	{
	public:
		std::vector<VertexInputLayoutDesc> m_desc;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class BlendState
	{
	public:
		BlendStateDesc m_desc;

	public:
		BlendState() {};
		~BlendState() {};

		BlendStateDesc GetDesc() { return m_desc; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class DepthStencilState
	{
	public:
		DepthStencilStateDesc m_desc;

	public:
		DepthStencilState() {};
		~DepthStencilState() {};

		DepthStencilStateDesc GetDesc() { return m_desc; }
	};

	class RasterizerState
	{
	public:
		RasterizerStateDesc m_desc;

	public:
		RasterizerState() {};
		~RasterizerState() {};

		RasterizerStateDesc GetDesc() { return m_desc; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class GraphicsPSO
	{
	public:
		ComPtr< ID3D12PipelineState >	m_pso;
		GraphicsPSODesc					m_desc;

	public:
		const GraphicsPSODesc& GetDesc() const { return m_desc; }

		GraphicsPSO() {};
		~GraphicsPSO() {};
	};

	class ComputePSO
	{
	public:
		ComPtr< ID3D12PipelineState >	m_pso;
		ComputePSODesc					m_desc;

	public:
		const ComputePSODesc& GetDesc() const { return m_desc; }

		ComputePSO() {};
		~ComputePSO() {};
	};
}