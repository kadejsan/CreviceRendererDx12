#pragma once

#include "Include_DX12.h"
#include "GraphicsDescriptors.h"

using namespace Microsoft::WRL;

namespace Graphics
{
	class GPUResource
	{
	public:
		ComPtr< ID3D12Resource >				  m_resource;

		D3D12_CPU_DESCRIPTOR_HANDLE*			  m_srv;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE*> m_additionalSRVs;
		
		D3D12_CPU_DESCRIPTOR_HANDLE*			  m_uav;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE*> m_additionalUAVs;

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

	class GPURingBuffer : public GPUBuffer
	{
	public:
		size_t	m_byteOffset;
		UINT64	m_residentFrame;
	public:
		GPURingBuffer() 
			: m_byteOffset(0)
			, m_residentFrame(0) 
		{}

		virtual ~GPURingBuffer() 
		{}

		// The next appending to buffer will start at this offset
		size_t GetByteOffset() const { return m_byteOffset; }
		UINT64 GetResidentFrame() const { return m_residentFrame; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class GPUReadbackBuffer : public GPUBuffer
	{
	public:
		bool	m_isLocked;

	public:
		GPUReadbackBuffer()
			: m_isLocked(false)
		{}

		virtual ~GPUReadbackBuffer()
		{}

		bool IsLocked() const { return m_isLocked; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class Texture : public GPUResource
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE*			  m_rtv;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE*> m_additionalRTVs;

	public:
		TextureDesc						m_desc;

	public:
		bool							m_independentRTVArraySlices;
		bool							m_independentRTVCubemapFaces;
		bool							m_independentSRVArraySlices;
		bool							m_independentSRVMIPs;
		bool							m_independentUAVMIPs;

	public:
		Texture();
		virtual ~Texture();

		// if true, then each array slice will get a unique rendertarget
		void RequestIndependentRenderTargetArraySlices(bool value);
		// if true, then each face of the cubemap will get a unique rendertarget
		void RequestIndependentRenderTargetCubemapFaces(bool value);
		// if true, then each array slice will get a unique shader resource
		void RequestIndependentShaderResourceArraySlices(bool value);
		// if true, then each miplevel will get unique shader resource
		void RequestIndependentShaderResourcesForMIPs(bool value);
		// if true, then each miplevel will get unique unordered access resource
		void RequestIndependentUnorderedAccessResourcesForMIPs(bool value);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class Texture2D : public Texture
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE*			  m_dsv;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE*> m_additionalDSVs;

	public:
		Texture2D();
		virtual ~Texture2D();
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

	class RayGenerationShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::RGS; }
	};

	class RayMissShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::MS; }
	};

	class RayClosestHitShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::CHS; }
	};

	class RayAnyHitShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::AHS; }
	};

	class RayIntersectionShader : public BaseShader
	{
		virtual SHADERSTAGE GetShaderStage() const { return SHADERSTAGE::IS; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class Sampler
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE*	m_resource;

		SamplerDesc						m_desc;
	public:
		Sampler();
		~Sampler();

		bool IsValid() { return m_resource != nullptr; }
		SamplerDesc GetDesc() { return m_desc; }
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

	class HitGroup
	{
	public:
		HitGroupDesc m_desc;

	public:
		HitGroup() {};
		~HitGroup() {};

		HitGroupDesc& GetDesc() { return m_desc; }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class RayTracingAccelerationStructure
	{
	public:
		GPUBuffer*		m_BLASResult;
		GPUBuffer*		m_BLASScratch;
		GPUBuffer*		m_TLASResult;
		GPUBuffer*		m_TLASScratch;
		GPUBuffer*		m_instanceDesc;

		RayTracingAccelerationStructureDesc m_desc;
	public:

		const RayTracingAccelerationStructureDesc& GetDesc() const { return m_desc; }

		RayTracingAccelerationStructure()
			: m_BLASResult(nullptr)
			, m_BLASScratch(nullptr)
			, m_TLASResult(nullptr)
			, m_TLASScratch(nullptr)
			, m_instanceDesc(nullptr)
		{}

		~RayTracingAccelerationStructure() 
		{
			delete m_BLASResult;
			delete m_BLASScratch;
			delete m_TLASResult;
			delete m_TLASScratch;
			delete m_instanceDesc;
		}
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

	class RayTracePSO
	{
	public:
		ComPtr<ID3D12StateObject>			m_pso;
		ComPtr<ID3D12StateObjectProperties> m_rtpsoInfo;
		RayTracePSODesc						m_desc;

	public:
		const RayTracePSODesc& GetDesc() const { return m_desc; }
		RayTracePSO() {};
		~RayTracePSO() {};
	};

	// - - - - - - - - - - - - - - - - - - - - - - - -

	class ShaderTable
	{
	public:
		ComPtr<ID3D12Resource>	m_shaderTable;
		UINT					m_shaderTableRecordSize;

	public:
		ShaderTable()
			: m_shaderTableRecordSize(0)
		{}
	};
}