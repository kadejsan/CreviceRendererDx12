#include "stdafx.h"
#include "DXHelper.h"
#include "D3DUtils.h"
#include "MathHelper.h"
#include "BaseWindow.h"
#include "GraphicsDevice_DX12.h"
#include "GraphicsResource.h"

namespace GraphicsTypes
{
	// Local Helpers:

	inline size_t Align(size_t uLocation, size_t uAlign)
	{
		if ((0 == uAlign) || (uAlign & (uAlign - 1)))
		{
			assert(0);
		}

		return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
	}

	inline UINT ParseColorWriteMask(UINT value)
	{
		UINT _flag = 0;

		if (value == D3D12_COLOR_WRITE_ENABLE_ALL)
		{
			return D3D12_COLOR_WRITE_ENABLE_ALL;
		}
		else
		{
			if (value & COLOR_WRITE_ENABLE_RED)
				_flag |= D3D12_COLOR_WRITE_ENABLE_RED;
			if (value & COLOR_WRITE_ENABLE_GREEN)
				_flag |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			if (value & COLOR_WRITE_ENABLE_BLUE)
				_flag |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			if (value & COLOR_WRITE_ENABLE_ALPHA)
				_flag |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}

		return _flag;
	}
	inline D3D12_RESOURCE_STATES ConvertResourceStates(RESOURCE_STATES value)
	{
		return static_cast<D3D12_RESOURCE_STATES>(value);
	}
	inline DXGI_FORMAT ConvertFormat(FORMAT value)
	{
		switch (value)
		{
		case FORMAT_UNKNOWN:
			return DXGI_FORMAT_UNKNOWN;
			break;
		case FORMAT_R32G32B32A32_TYPELESS:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;
			break;
		case FORMAT_R32G32B32A32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case FORMAT_R32G32B32A32_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;
			break;
		case FORMAT_R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		case FORMAT_R32G32B32_TYPELESS:
			return DXGI_FORMAT_R32G32B32_TYPELESS;
			break;
		case FORMAT_R32G32B32_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case FORMAT_R32G32B32_UINT:
			return DXGI_FORMAT_R32G32B32_UINT;
			break;
		case FORMAT_R32G32B32_SINT:
			return DXGI_FORMAT_R32G32B32_SINT;
			break;
		case FORMAT_R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
			break;
		case FORMAT_R16G16B16A16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case FORMAT_R16G16B16A16_UNORM:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
			break;
		case FORMAT_R16G16B16A16_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;
			break;
		case FORMAT_R16G16B16A16_SNORM:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		case FORMAT_R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;
			break;
		case FORMAT_R32G32_TYPELESS:
			return DXGI_FORMAT_R32G32_TYPELESS;
			break;
		case FORMAT_R32G32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
			break;
		case FORMAT_R32G32_UINT:
			return DXGI_FORMAT_R32G32_UINT;
			break;
		case FORMAT_R32G32_SINT:
			return DXGI_FORMAT_R32G32_SINT;
			break;
		case FORMAT_R32G8X24_TYPELESS:
			return DXGI_FORMAT_R32G8X24_TYPELESS;
			break;
		case FORMAT_D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			break;
		case FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
			break;
		case FORMAT_R10G10B10A2_TYPELESS:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
			break;
		case FORMAT_R10G10B10A2_UNORM:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
			break;
		case FORMAT_R10G10B10A2_UINT:
			return DXGI_FORMAT_R10G10B10A2_UINT;
			break;
		case FORMAT_R11G11B10_FLOAT:
			return DXGI_FORMAT_R11G11B10_FLOAT;
			break;
		case FORMAT_R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
			break;
		case FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			break;
		case FORMAT_R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		case FORMAT_R8G8B8A8_SNORM:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
			break;
		case FORMAT_R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;
			break;
		case FORMAT_R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_TYPELESS;
			break;
		case FORMAT_R16G16_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
			break;
		case FORMAT_R16G16_UNORM:
			return DXGI_FORMAT_R16G16_UNORM;
			break;
		case FORMAT_R16G16_UINT:
			return DXGI_FORMAT_R16G16_UINT;
			break;
		case FORMAT_R16G16_SNORM:
			return DXGI_FORMAT_R16G16_SNORM;
			break;
		case FORMAT_R16G16_SINT:
			return DXGI_FORMAT_R16G16_SINT;
			break;
		case FORMAT_R32_TYPELESS:
			return DXGI_FORMAT_R32_TYPELESS;
			break;
		case FORMAT_D32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
			break;
		case FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
			break;
		case FORMAT_R32_UINT:
			return DXGI_FORMAT_R32_UINT;
			break;
		case FORMAT_R32_SINT:
			return DXGI_FORMAT_R32_SINT;
			break;
		case FORMAT_R24G8_TYPELESS:
			return DXGI_FORMAT_R24G8_TYPELESS;
			break;
		case FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FORMAT_R24_UNORM_X8_TYPELESS:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			break;
		case FORMAT_R8G8_TYPELESS:
			return DXGI_FORMAT_R8G8_TYPELESS;
			break;
		case FORMAT_R8G8_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;
			break;
		case FORMAT_R8G8_UINT:
			return DXGI_FORMAT_R8G8_UINT;
			break;
		case FORMAT_R8G8_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;
			break;
		case FORMAT_R8G8_SINT:
			return DXGI_FORMAT_R8G8_SINT;
			break;
		case FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_TYPELESS;
			break;
		case FORMAT_R16_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
			break;
		case FORMAT_D16_UNORM:
			return DXGI_FORMAT_D16_UNORM;
			break;
		case FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
			break;
		case FORMAT_R16_UINT:
			return DXGI_FORMAT_R16_UINT;
			break;
		case FORMAT_R16_SNORM:
			return DXGI_FORMAT_R16_SNORM;
			break;
		case FORMAT_R16_SINT:
			return DXGI_FORMAT_R16_SINT;
			break;
		case FORMAT_R8_TYPELESS:
			return DXGI_FORMAT_R8_TYPELESS;
			break;
		case FORMAT_R8_UNORM:
			return DXGI_FORMAT_R8_UNORM;
			break;
		case FORMAT_R8_UINT:
			return DXGI_FORMAT_R8_UINT;
			break;
		case FORMAT_R8_SNORM:
			return DXGI_FORMAT_R8_SNORM;
			break;
		case FORMAT_R8_SINT:
			return DXGI_FORMAT_R8_SINT;
			break;
		case FORMAT_A8_UNORM:
			return DXGI_FORMAT_A8_UNORM;
			break;
		case FORMAT_R1_UNORM:
			return DXGI_FORMAT_R1_UNORM;
			break;
		case FORMAT_R9G9B9E5_SHAREDEXP:
			return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
			break;
		case FORMAT_R8G8_B8G8_UNORM:
			return DXGI_FORMAT_R8G8_B8G8_UNORM;
			break;
		case FORMAT_G8R8_G8B8_UNORM:
			return DXGI_FORMAT_G8R8_G8B8_UNORM;
			break;
		case FORMAT_BC1_TYPELESS:
			return DXGI_FORMAT_BC1_TYPELESS;
			break;
		case FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM;
			break;
		case FORMAT_BC1_UNORM_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
			break;
		case FORMAT_BC2_TYPELESS:
			return DXGI_FORMAT_BC2_TYPELESS;
			break;
		case FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM;
			break;
		case FORMAT_BC2_UNORM_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
			break;
		case FORMAT_BC3_TYPELESS:
			return DXGI_FORMAT_BC3_TYPELESS;
			break;
		case FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM;
			break;
		case FORMAT_BC3_UNORM_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;
			break;
		case FORMAT_BC4_TYPELESS:
			return DXGI_FORMAT_BC4_TYPELESS;
			break;
		case FORMAT_BC4_UNORM:
			return DXGI_FORMAT_BC4_UNORM;
			break;
		case FORMAT_BC4_SNORM:
			return DXGI_FORMAT_BC4_SNORM;
			break;
		case FORMAT_BC5_TYPELESS:
			return DXGI_FORMAT_BC5_TYPELESS;
			break;
		case FORMAT_BC5_UNORM:
			return DXGI_FORMAT_BC5_UNORM;
			break;
		case FORMAT_BC5_SNORM:
			return DXGI_FORMAT_BC5_SNORM;
			break;
		case FORMAT_B5G6R5_UNORM:
			return DXGI_FORMAT_B5G6R5_UNORM;
			break;
		case FORMAT_B5G5R5A1_UNORM:
			return DXGI_FORMAT_B5G5R5A1_UNORM;
			break;
		case FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		case FORMAT_B8G8R8X8_UNORM:
			return DXGI_FORMAT_B8G8R8X8_UNORM;
			break;
		case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
			break;
		case FORMAT_B8G8R8A8_TYPELESS:
			return DXGI_FORMAT_B8G8R8A8_TYPELESS;
			break;
		case FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			break;
		case FORMAT_B8G8R8X8_TYPELESS:
			return DXGI_FORMAT_B8G8R8X8_TYPELESS;
			break;
		case FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
			break;
		case FORMAT_BC6H_TYPELESS:
			return DXGI_FORMAT_BC6H_TYPELESS;
			break;
		case FORMAT_BC6H_UF16:
			return DXGI_FORMAT_BC6H_UF16;
			break;
		case FORMAT_BC6H_SF16:
			return DXGI_FORMAT_BC6H_SF16;
			break;
		case FORMAT_BC7_TYPELESS:
			return DXGI_FORMAT_BC7_TYPELESS;
			break;
		case FORMAT_BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM;
			break;
		case FORMAT_BC7_UNORM_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
			break;
		case FORMAT_AYUV:
			return DXGI_FORMAT_AYUV;
			break;
		case FORMAT_Y410:
			return DXGI_FORMAT_Y410;
			break;
		case FORMAT_Y416:
			return DXGI_FORMAT_Y416;
			break;
		case FORMAT_NV12:
			return DXGI_FORMAT_NV12;
			break;
		case FORMAT_P010:
			return DXGI_FORMAT_P010;
			break;
		case FORMAT_P016:
			return DXGI_FORMAT_P016;
			break;
		case FORMAT_420_OPAQUE:
			return DXGI_FORMAT_420_OPAQUE;
			break;
		case FORMAT_YUY2:
			return DXGI_FORMAT_YUY2;
			break;
		case FORMAT_Y210:
			return DXGI_FORMAT_Y210;
			break;
		case FORMAT_Y216:
			return DXGI_FORMAT_Y216;
			break;
		case FORMAT_NV11:
			return DXGI_FORMAT_NV11;
			break;
		case FORMAT_AI44:
			return DXGI_FORMAT_AI44;
			break;
		case FORMAT_IA44:
			return DXGI_FORMAT_IA44;
			break;
		case FORMAT_P8:
			return DXGI_FORMAT_P8;
			break;
		case FORMAT_A8P8:
			return DXGI_FORMAT_A8P8;
			break;
		case FORMAT_B4G4R4A4_UNORM:
			return DXGI_FORMAT_B4G4R4A4_UNORM;
			break;
		case FORMAT_FORCE_UINT:
			return DXGI_FORMAT_FORCE_UINT;
			break;
		default:
			break;
		}
		return DXGI_FORMAT_UNKNOWN;
	}
	inline D3D12_FILL_MODE ConvertFillMode(FILL_MODE value)
	{
		switch (value)
		{
		case FILL_WIREFRAME:
			return D3D12_FILL_MODE_WIREFRAME;
			break;
		case FILL_SOLID:
			return D3D12_FILL_MODE_SOLID;
			break;
		default:
			break;
		}
		return D3D12_FILL_MODE_WIREFRAME;
	}
	inline D3D12_CULL_MODE ConvertCullMode(CULL_MODE value)
	{
		switch (value)
		{
		case CULL_NONE:
			return D3D12_CULL_MODE_NONE;
			break;
		case CULL_FRONT:
			return D3D12_CULL_MODE_FRONT;
			break;
		case CULL_BACK:
			return D3D12_CULL_MODE_BACK;
			break;
		default:
			break;
		}
		return D3D12_CULL_MODE_NONE;
	}
	inline D3D12_COMPARISON_FUNC ConvertComparisonFunc(COMPARISON_FUNC value)
	{
		switch (value)
		{
		case COMPARISON_NEVER:
			return D3D12_COMPARISON_FUNC_NEVER;
			break;
		case COMPARISON_LESS:
			return D3D12_COMPARISON_FUNC_LESS;
			break;
		case COMPARISON_EQUAL:
			return D3D12_COMPARISON_FUNC_EQUAL;
			break;
		case COMPARISON_LESS_EQUAL:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		case COMPARISON_GREATER:
			return D3D12_COMPARISON_FUNC_GREATER;
			break;
		case COMPARISON_NOT_EQUAL:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			break;
		case COMPARISON_GREATER_EQUAL:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			break;
		case COMPARISON_ALWAYS:
			return D3D12_COMPARISON_FUNC_ALWAYS;
			break;
		default:
			break;
		}
		return D3D12_COMPARISON_FUNC_NEVER;
	}
	inline D3D12_DEPTH_WRITE_MASK ConvertDepthWriteMask(DEPTH_WRITE_MASK value)
	{
		switch (value)
		{
		case DEPTH_WRITE_MASK_ZERO:
			return D3D12_DEPTH_WRITE_MASK_ZERO;
			break;
		case DEPTH_WRITE_MASK_ALL:
			return D3D12_DEPTH_WRITE_MASK_ALL;
			break;
		default:
			break;
		}
		return D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	inline D3D12_STENCIL_OP ConvertStencilOp(STENCIL_OP value)
	{
		switch (value)
		{
		case STENCIL_OP_KEEP:
			return D3D12_STENCIL_OP_KEEP;
			break;
		case STENCIL_OP_ZERO:
			return D3D12_STENCIL_OP_ZERO;
			break;
		case STENCIL_OP_REPLACE:
			return D3D12_STENCIL_OP_REPLACE;
			break;
		case STENCIL_OP_INCR_SAT:
			return D3D12_STENCIL_OP_INCR_SAT;
			break;
		case STENCIL_OP_DECR_SAT:
			return D3D12_STENCIL_OP_DECR_SAT;
			break;
		case STENCIL_OP_INVERT:
			return D3D12_STENCIL_OP_INVERT;
			break;
		case STENCIL_OP_INCR:
			return D3D12_STENCIL_OP_INCR;
			break;
		case STENCIL_OP_DECR:
			return D3D12_STENCIL_OP_DECR;
			break;
		default:
			break;
		}
		return D3D12_STENCIL_OP_KEEP;
	}
	inline D3D12_BLEND ConvertBlend(BLEND value)
	{
		switch (value)
		{
		case BLEND_ZERO:
			return D3D12_BLEND_ZERO;
			break;
		case BLEND_ONE:
			return D3D12_BLEND_ONE;
			break;
		case BLEND_SRC_COLOR:
			return D3D12_BLEND_SRC_COLOR;
			break;
		case BLEND_INV_SRC_COLOR:
			return D3D12_BLEND_INV_SRC_COLOR;
			break;
		case BLEND_SRC_ALPHA:
			return D3D12_BLEND_SRC_ALPHA;
			break;
		case BLEND_INV_SRC_ALPHA:
			return D3D12_BLEND_INV_SRC_ALPHA;
			break;
		case BLEND_DEST_ALPHA:
			return D3D12_BLEND_DEST_ALPHA;
			break;
		case BLEND_INV_DEST_ALPHA:
			return D3D12_BLEND_INV_DEST_ALPHA;
			break;
		case BLEND_DEST_COLOR:
			return D3D12_BLEND_DEST_COLOR;
			break;
		case BLEND_INV_DEST_COLOR:
			return D3D12_BLEND_INV_DEST_COLOR;
			break;
		case BLEND_SRC_ALPHA_SAT:
			return D3D12_BLEND_SRC_ALPHA_SAT;
			break;
		case BLEND_BLEND_FACTOR:
			return D3D12_BLEND_BLEND_FACTOR;
			break;
		case BLEND_INV_BLEND_FACTOR:
			return D3D12_BLEND_INV_BLEND_FACTOR;
			break;
		case BLEND_SRC1_COLOR:
			return D3D12_BLEND_SRC1_COLOR;
			break;
		case BLEND_INV_SRC1_COLOR:
			return D3D12_BLEND_INV_SRC1_COLOR;
			break;
		case BLEND_SRC1_ALPHA:
			return D3D12_BLEND_SRC1_ALPHA;
			break;
		case BLEND_INV_SRC1_ALPHA:
			return D3D12_BLEND_INV_SRC1_ALPHA;
			break;
		default:
			break;
		}
		return D3D12_BLEND_ZERO;
	}
	inline D3D12_BLEND_OP ConvertBlendOp(BLEND_OP value)
	{
		switch (value)
		{
		case BLEND_OP_ADD:
			return D3D12_BLEND_OP_ADD;
			break;
		case BLEND_OP_SUBTRACT:
			return D3D12_BLEND_OP_SUBTRACT;
			break;
		case BLEND_OP_REV_SUBTRACT:
			return D3D12_BLEND_OP_REV_SUBTRACT;
			break;
		case BLEND_OP_MIN:
			return D3D12_BLEND_OP_MIN;
			break;
		case BLEND_OP_MAX:
			return D3D12_BLEND_OP_MAX;
			break;
		default:
			break;
		}
		return D3D12_BLEND_OP_ADD;
	}
	inline D3D12_INPUT_CLASSIFICATION ConvertInputClassification(INPUT_CLASSIFICATION value)
	{
		switch (value)
		{
		case INPUT_PER_VERTEX_DATA:
			return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			break;
		case INPUT_PER_INSTANCE_DATA:
			return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			break;
		default:
			break;
		}
		return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopologyType(PRIMITIVETOPOLOGY topology)
	{
		switch (topology)
		{
		case POINTLIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			break;
		case LINELIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			break;
		case TRIANGLELIST:
		case TRIANGLESTRIP:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		case PATCHLIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			break;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			break;
		}
	}
	inline D3D12_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(PRIMITIVETOPOLOGY topology)
	{
		switch (topology)
		{
		case TRIANGLELIST:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case TRIANGLESTRIP:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case POINTLIST:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		case LINELIST:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case PATCHLIST:
			return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
			break;
		default:
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			break;
		};
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Allocator heaps:

	GraphicsDevice_DX12::DescriptorAllocator::DescriptorAllocator(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT maxCount)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = maxCount;
		heapDesc.Type = type;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
		assert(SUCCEEDED(hr));

		m_itemSize = device->GetDescriptorHandleIncrementSize(type);
		m_itemCount.store(0);
		m_maxCount = maxCount;
	}
	GraphicsDevice_DX12::DescriptorAllocator::~DescriptorAllocator()
	{
	}
	size_t GraphicsDevice_DX12::DescriptorAllocator::Allocate()
	{
		return m_heap->GetCPUDescriptorHandleForHeapStart().ptr + m_itemCount.fetch_add(1) * m_itemSize;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Allocator upload buffers:

	GraphicsDevice_DX12::UploadBuffer::UploadBuffer(ID3D12Device* device, UINT64 byteSize)
	{
		ThrowIfFailed( device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource) ) );

		void* pData = nullptr;
		
		//No CPU reads will be done from the resource
		CD3DX12_RANGE readRange(0, 0);
		m_resource->Map(0, &readRange, &pData);
		m_dataCurrent = m_dataBegin = reinterpret_cast<UINT8*>(pData);
		m_dataEnd = m_dataBegin + byteSize;
	}
	GraphicsDevice_DX12::UploadBuffer::~UploadBuffer()
	{
	}
	UINT8* GraphicsDevice_DX12::UploadBuffer::Allocate(UINT64 dataSize, UINT64 alignment)
	{
		dataSize = Align(dataSize, alignment);
		assert(m_dataCurrent + dataSize <= m_dataEnd);

		UINT8* retVal = m_dataCurrent;
		m_dataCurrent += dataSize;

		return retVal;
	}
	void GraphicsDevice_DX12::UploadBuffer::Clear()
	{
		m_dataCurrent = m_dataBegin;
	}
	UINT64 GraphicsDevice_DX12::UploadBuffer::CalculateOffset(UINT8* address)
	{
		assert(address >= m_dataBegin && address < m_dataEnd);
		return static_cast<UINT64>(address - m_dataBegin);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	GraphicsDevice_DX12::GraphicsDevice_DX12()
	{
	}

	GraphicsDevice_DX12::~GraphicsDevice_DX12()
	{
		// Ensure that the GPU is no longer referencing resources that are about to be
		// cleaned up by the destructor.
		Flush();

		CloseHandle(m_fenceEvent);

		SAFE_DELETE(RTAllocator);
		SAFE_DELETE(DSAllocator);
		SAFE_DELETE(ResourceAllocator);
		SAFE_DELETE(SamplerAllocator);

		SAFE_DELETE(BufferUploader);
		SAFE_DELETE(TextureUploader);

		SAFE_DELETE(m_nullSampler);
		SAFE_DELETE(m_nullCBV);
		SAFE_DELETE(m_nullSRV);
		SAFE_DELETE(m_nullUAV);
	}

	void GraphicsDevice_DX12::Initialize(BaseWindow* window)
	{
		// Load rendering pipeline dependencies
		UINT32 dxgiFactoryFlags = 0;

#ifdef _DEBUG
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

		if (window->UseWarpDevice())
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
		}
		else
		{
			m_device = CreateDevice();
			m_commandQueue = CreateCommandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_swapChain = CreateSwapChain(Win32Application::GetHwnd(), m_dxgiFactory, m_commandQueue, window->GetWidth(), window->GetHeight(), st_frameCount);
			m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

			m_rtvHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, st_frameCount);
			m_dsvHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1);

			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (int i = 0; i < st_frameCount; ++i)
				m_commandAllocator[i] = CreateCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_commandList = CreateCommandList(m_device, m_commandAllocator[m_frameIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

			UpdateRenderTargetViews(m_device, m_swapChain, m_rtvHeap);
			UpdateDepthStencil(m_device, m_dsvHeap, window->GetWidth(), window->GetHeight());
			UpdateViewportAndScissor(window->GetWidth(), window->GetHeight());

			m_fence = CreateFence(m_device);
			m_fenceEvent = CreateEventHandle();

			m_graphicsRootSig = CreateGraphicsRootSignature(m_device);
			m_computeRootSig = CreateComputeRootSignature(m_device);
		}

		// Create common descriptor heaps
		RTAllocator = new DescriptorAllocator(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
		DSAllocator = new DescriptorAllocator(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);
		ResourceAllocator = new DescriptorAllocator(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerAllocator = new DescriptorAllocator(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 64);

		// Create resource upload buffer
		BufferUploader = new UploadBuffer(m_device.Get(), 256 * 1024 * 1024);
		TextureUploader = new UploadBuffer(m_device.Get(), 256 * 1024 * 1024);

		CreateNullResources(m_device);

		// Create frame-resident resources
		for (UINT fr = 0; fr < st_frameCount; ++fr)
		{
			Frames[fr].ResourceDescriptorsGPU = new FrameResources::DescriptorTableFrameAllocator(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);
			Frames[fr].SamplerDescriptorsGPU = new FrameResources::DescriptorTableFrameAllocator(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 16);
			Frames[fr].ResourceBuffer = new FrameResources::ResourceFrameAllocator(m_device, 1024 * 1024 * 128);
		}

		m_isInitialized = true;
	}

	void GraphicsDevice_DX12::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		UINT32 destAdapterIndex = 0;
		UINT64 videoMemory = 0;
		for (UINT32 adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				LOG( "%ls", desc.Description );
				if (desc.DedicatedVideoMemory > videoMemory)
				{
					videoMemory = desc.DedicatedVideoMemory;
					destAdapterIndex = adapterIndex;
				}
			}
		}

		pFactory->EnumAdapters1(0, ppAdapter);
	}

	bool GraphicsDevice_DX12::CheckTearingSupport(ComPtr<IDXGIFactory4> factory4)
	{
		bool allowTearing = false;

		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = false;
			}
		}

		return allowTearing;
	}

	ComPtr<ID3D12Device> GraphicsDevice_DX12::CreateDevice()
	{
		// Create the D3D graphics device
		ComPtr<IDXGIAdapter1> pAdapter;
		ComPtr<ID3D12Device> pDevice;

		DXGI_ADAPTER_DESC1 adapterDesc = {};
		SIZE_T maxSize = 0;

		for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (desc.DedicatedVideoMemory > maxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice))))
			{
				pAdapter->GetDesc1(&desc);
				maxSize = desc.DedicatedVideoMemory;
				adapterDesc = desc;
			}
		}

		if (maxSize > 0)
		{
			LOG("GpuApi: D3D12-capable hardware found!");
			LOG("GpuApi: Adapter details:");
			LOG("GpuApi:   Description: %ls", adapterDesc.Description);
			LOG("GpuApi:   Vendor ID: %u", adapterDesc.VendorId);
			LOG("GpuApi:   Device ID: %u", adapterDesc.DeviceId);
			LOG("GpuApi:   SubSys ID: %u", adapterDesc.SubSysId);
			LOG("GpuApi:   Revision: %u", adapterDesc.Revision);
			LOG("GpuApi:   Dedicated Video Memory: %uMB", UINT32(adapterDesc.DedicatedVideoMemory / size_t(1024 * 1024)));
			LOG("GpuApi:   Dedicated System Memory: %uMB", UINT32(adapterDesc.DedicatedSystemMemory / size_t(1024 * 1024)));
			LOG("GpuApi:   Shared System Memory: %uMB", UINT32(adapterDesc.SharedSystemMemory / size_t(1024 * 1024)));
		}
		else
		{
			LOG("GpuApi: Unable to create Device.");
			return nullptr;
		}

		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		{
			bool DeveloperModeEnabled = false;

			// Look in the Windows Registry to determine if Developer Mode is enabled
			HKEY hKey;
			LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
			if (result == ERROR_SUCCESS)
			{
				DWORD keyValue, keySize = sizeof(DWORD);
				result = RegQueryValueEx(hKey, "AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
				if (result == ERROR_SUCCESS && keyValue == 1)
					DeveloperModeEnabled = true;
				RegCloseKey(hKey);
			}

			if (!DeveloperModeEnabled)
			{
				LOG("Enable Developer Mode on Windows 10 to get consistent profiling results");
			}

			// Prevent the GPU from overclocking or underclocking to get consistent timings
			if (DeveloperModeEnabled)
				pDevice->SetStablePowerState(TRUE);
		}

		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(pDevice.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				// I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

				// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,

				// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,

				// This occurs when there are uninitialized descriptors in a descriptor table, even when a
				// shader does not access the missing descriptors.  I find this is common when switching
				// shader permutations and not wanting to change much code to reorder resources.
				D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

				// Triggered when a shader does not export all color components of a render target, such as
				// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
				D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

				// This occurs when a descriptor table is unbound even when a shader does not access the missing
				// descriptors.  This is common with a root signature shared between disparate shaders that
				// don't all need the same types of resources.
				D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

				// RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
				(D3D12_MESSAGE_ID)1008,
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
		}
#endif

		return pDevice.Detach();
	}

	ComPtr<ID3D12CommandQueue> GraphicsDevice_DX12::CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

		return d3d12CommandQueue;
	}

	ComPtr<IDXGISwapChain4> GraphicsDevice_DX12::CreateSwapChain(HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount)
	{
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = ConvertFormat( m_backBufferFormat );
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = CheckTearingSupport(factory) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
		ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

		return dxgiSwapChain4;
	}

	ComPtr<ID3D12DescriptorHeap> GraphicsDevice_DX12::CreateDescriptorHeap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT32 numDescriptors)
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = flags;

		ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

		return descriptorHeap;
	}

	ComPtr<ID3D12CommandAllocator> GraphicsDevice_DX12::CreateCommandAllocator(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList> GraphicsDevice_DX12::CreateCommandList(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12GraphicsCommandList> commandList;
		ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

		ThrowIfFailed(commandList->Close());

		return commandList;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Descriptor layout counts:
#define GPU_RESOURCE_HEAP_CBV_COUNT		12
#define GPU_RESOURCE_HEAP_SRV_COUNT		64
#define GPU_RESOURCE_HEAP_UAV_COUNT		8
#define GPU_SAMPLER_HEAP_COUNT			16

	GraphicsDevice_DX12::FrameResources::DescriptorTableFrameAllocator::DescriptorTableFrameAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT maxRenameCount)
	{
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			m_itemCount = GPU_SAMPLER_HEAP_COUNT;
		}
		else if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			m_itemCount = (GPU_RESOURCE_HEAP_CBV_COUNT + GPU_RESOURCE_HEAP_SRV_COUNT + GPU_RESOURCE_HEAP_UAV_COUNT);
		}
		else
		{
			assert(0);
		}

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NodeMask = 0;
		heapDesc.Type = type;

		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NumDescriptors = m_itemCount * SHADERSTAGE_MAX;
		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapCPU));

		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NumDescriptors = m_itemCount * SHADERSTAGE_MAX * maxRenameCount;
		hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapGPU));
		assert(SUCCEEDED(hr));

		m_descriptorType = type;
		m_itemSize = device->GetDescriptorHandleIncrementSize(type);

		m_boundDescriptors = new D3D12_CPU_DESCRIPTOR_HANDLE*[SHADERSTAGE_MAX * m_itemCount];
	}
	GraphicsDevice_DX12::FrameResources::DescriptorTableFrameAllocator::~DescriptorTableFrameAllocator()
	{
		SAFE_RELEASE(m_heapCPU);
		SAFE_RELEASE(m_heapGPU);
		SAFE_DELETE_ARRAY(m_boundDescriptors);
	}
	void GraphicsDevice_DX12::FrameResources::DescriptorTableFrameAllocator::Reset(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE* nullDescriptorsSamplerCBVSRVUAV)
	{
		memset(m_boundDescriptors, 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE*)*SHADERSTAGE_MAX*m_itemCount);

		m_ringOffset = 0;

		for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
		{
			m_isDirty[stage] = true;

			// Fill staging tables with null descriptors (TODO make nicer and reduce copy ops):
			if (m_descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			{
				for (int slot = 0; slot < GPU_RESOURCE_HEAP_CBV_COUNT; ++slot)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dst_staging = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
					dst_staging.ptr += (stage * m_itemCount + slot) * m_itemSize;

					device->CopyDescriptorsSimple(1, dst_staging, nullDescriptorsSamplerCBVSRVUAV[1], (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);
				}
				for (int slot = 0; slot < GPU_RESOURCE_HEAP_SRV_COUNT; ++slot)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dst_staging = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
					dst_staging.ptr += (stage * m_itemCount + GPU_RESOURCE_HEAP_CBV_COUNT + slot) * m_itemSize;

					device->CopyDescriptorsSimple(1, dst_staging, nullDescriptorsSamplerCBVSRVUAV[2], (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);
				}
				for (int slot = 0; slot < GPU_RESOURCE_HEAP_UAV_COUNT; ++slot)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dst_staging = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
					dst_staging.ptr += (stage * m_itemCount + GPU_RESOURCE_HEAP_CBV_COUNT + GPU_RESOURCE_HEAP_SRV_COUNT + slot) * m_itemSize;

					device->CopyDescriptorsSimple(1, dst_staging, nullDescriptorsSamplerCBVSRVUAV[3], (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);
				}
			}
			else
			{
				for (int slot = 0; slot < GPU_SAMPLER_HEAP_COUNT; ++slot)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dst_staging = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
					dst_staging.ptr += (stage * m_itemCount + slot) * m_itemSize;

					device->CopyDescriptorsSimple(1, dst_staging, nullDescriptorsSamplerCBVSRVUAV[0], (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);
				}
			}

		}
	}
	void GraphicsDevice_DX12::FrameResources::DescriptorTableFrameAllocator::Update(SHADERSTAGE stage, UINT offset, D3D12_CPU_DESCRIPTOR_HANDLE* descriptor, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		if (descriptor == nullptr)
		{
			return;
		}
		UINT idx = stage * m_itemCount + offset;

		if (m_boundDescriptors[idx] == descriptor)
		{
			return;
		}

		m_boundDescriptors[idx] = descriptor;

		m_isDirty[stage] = true;

		D3D12_CPU_DESCRIPTOR_HANDLE dst_staging = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
		dst_staging.ptr += idx * m_itemSize;

		device->CopyDescriptorsSimple(1, dst_staging, *descriptor, (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);
	}
	void GraphicsDevice_DX12::FrameResources::DescriptorTableFrameAllocator::Validate(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		for (int stage = VS; stage < SHADERSTAGE_MAX; ++stage)
		{
			if (m_isDirty[stage])
			{
				// copy prev table contents to new dest
				D3D12_CPU_DESCRIPTOR_HANDLE dst = m_heapGPU->GetCPUDescriptorHandleForHeapStart();
				dst.ptr += m_ringOffset;
				D3D12_CPU_DESCRIPTOR_HANDLE src = m_heapCPU->GetCPUDescriptorHandleForHeapStart();
				src.ptr += (stage * m_itemCount) * m_itemSize;
				device->CopyDescriptorsSimple(m_itemCount, dst, src, (D3D12_DESCRIPTOR_HEAP_TYPE)m_descriptorType);

				// bind table to root sig
				D3D12_GPU_DESCRIPTOR_HANDLE table = m_heapGPU->GetGPUDescriptorHandleForHeapStart();
				table.ptr += m_ringOffset;

				if (stage == CS)
				{
					// compute descriptor heap:

					if (m_descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
					{
						commandList->SetComputeRootDescriptorTable(0, table);
					}
					else
					{
						commandList->SetComputeRootDescriptorTable(1, table);
					}
				}
				else
				{
					// graphics descriptor heap:

					if (m_descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
					{
						commandList->SetGraphicsRootDescriptorTable(stage * 2 + 0, table);
					}
					else
					{
						commandList->SetGraphicsRootDescriptorTable(stage * 2 + 1, table);
					}
				}

				// mark this as up to date
				m_isDirty[stage] = false;

				// allocate next chunk
				m_ringOffset += m_itemCount * m_itemSize;
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	GraphicsDevice_DX12::FrameResources::ResourceFrameAllocator::ResourceFrameAllocator(ComPtr<ID3D12Device> device, size_t size)
	{
		ThrowIfFailed( device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m_resource)));

		void* pData;
		//
		// No CPU reads will be done from the resource.
		//
		CD3DX12_RANGE readRange(0, 0);
		m_resource->Map(0, &readRange, &pData);
		m_dataCur = m_dataBegin = reinterpret_cast< UINT8* >(pData);
		m_dataEnd = m_dataBegin + size;
	}
	GraphicsDevice_DX12::FrameResources::ResourceFrameAllocator::~ResourceFrameAllocator()
	{
	}
	UINT8* GraphicsDevice_DX12::FrameResources::ResourceFrameAllocator::Allocate(size_t dataSize, size_t alignment)
	{
		m_dataCur = reinterpret_cast<uint8_t*>(Align(reinterpret_cast<size_t>(m_dataCur), alignment));
		assert(m_dataCur + dataSize <= m_dataEnd);

		uint8_t* retVal = m_dataCur;

		m_dataCur += dataSize;

		return retVal;
	}
	void GraphicsDevice_DX12::FrameResources::ResourceFrameAllocator::Clear()
	{
		m_dataCur = m_dataBegin;
	}
	UINT64 GraphicsDevice_DX12::FrameResources::ResourceFrameAllocator::CalculateOffset(UINT8* address)
	{
		assert(address >= m_dataBegin && address < m_dataEnd);
		return static_cast<UINT64>(address - m_dataBegin);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ComPtr<ID3D12RootSignature> GraphicsDevice_DX12::CreateGraphicsRootSignature(ComPtr<ID3D12Device> device)
	{
		D3D12_DESCRIPTOR_RANGE samplerRange = {};
		samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		samplerRange.BaseShaderRegister = 0;
		samplerRange.RegisterSpace = 0;
		samplerRange.NumDescriptors = GPU_SAMPLER_HEAP_COUNT;
		samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		{
			UINT descriptorRangeCount = 3;
			D3D12_DESCRIPTOR_RANGE* descriptorRanges = new D3D12_DESCRIPTOR_RANGE[descriptorRangeCount];

			descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descriptorRanges[0].BaseShaderRegister = 0;
			descriptorRanges[0].RegisterSpace = 0;
			descriptorRanges[0].NumDescriptors = GPU_RESOURCE_HEAP_CBV_COUNT;
			descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRanges[1].BaseShaderRegister = 0;
			descriptorRanges[1].RegisterSpace = 0;
			descriptorRanges[1].NumDescriptors = GPU_RESOURCE_HEAP_SRV_COUNT;
			descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descriptorRanges[2].BaseShaderRegister = 0;
			descriptorRanges[2].RegisterSpace = 0;
			descriptorRanges[2].NumDescriptors = GPU_RESOURCE_HEAP_UAV_COUNT;
			descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			UINT paramCount = 2 * (SHADERSTAGE_MAX - 1); // 2: resource,sampler;   5: vs,hs,ds,gs,ps;
			D3D12_ROOT_PARAMETER* params = new D3D12_ROOT_PARAMETER[paramCount];
			params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			params[0].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[0].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			params[1].DescriptorTable.NumDescriptorRanges = 1;
			params[1].DescriptorTable.pDescriptorRanges = &samplerRange;

			params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
			params[2].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[2].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
			params[3].DescriptorTable.NumDescriptorRanges = 1;
			params[3].DescriptorTable.pDescriptorRanges = &samplerRange;

			params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
			params[4].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[4].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
			params[5].DescriptorTable.NumDescriptorRanges = 1;
			params[5].DescriptorTable.pDescriptorRanges = &samplerRange;

			params[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
			params[6].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[6].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
			params[7].DescriptorTable.NumDescriptorRanges = 1;
			params[7].DescriptorTable.pDescriptorRanges = &samplerRange;

			params[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			params[8].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[8].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			params[9].DescriptorTable.NumDescriptorRanges = 1;
			params[9].DescriptorTable.pDescriptorRanges = &samplerRange;

			D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
			rootSigDesc.NumStaticSamplers = 0;
			rootSigDesc.NumParameters = paramCount;
			rootSigDesc.pParameters = params;
			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			ComPtr<ID3D12RootSignature>	graphicsRootSig;
			ID3DBlob* rootSigBlob;
			ID3DBlob* rootSigError;
			ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &rootSigError));
			ThrowIfFailed(device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsRootSig)));

			SAFE_DELETE_ARRAY(descriptorRanges);
			SAFE_DELETE_ARRAY(params);

			return graphicsRootSig;
		}
	}

	ComPtr<ID3D12RootSignature> GraphicsDevice_DX12::CreateComputeRootSignature(ComPtr<ID3D12Device> device)
	{
		D3D12_DESCRIPTOR_RANGE samplerRange = {};
		samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		samplerRange.BaseShaderRegister = 0;
		samplerRange.RegisterSpace = 0;
		samplerRange.NumDescriptors = GPU_SAMPLER_HEAP_COUNT;
		samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		
		{
			UINT descriptorRangeCount = 3;
			D3D12_DESCRIPTOR_RANGE* descriptorRanges = new D3D12_DESCRIPTOR_RANGE[descriptorRangeCount];

			descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descriptorRanges[0].BaseShaderRegister = 0;
			descriptorRanges[0].RegisterSpace = 0;
			descriptorRanges[0].NumDescriptors = GPU_RESOURCE_HEAP_CBV_COUNT;
			descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRanges[1].BaseShaderRegister = 0;
			descriptorRanges[1].RegisterSpace = 0;
			descriptorRanges[1].NumDescriptors = GPU_RESOURCE_HEAP_SRV_COUNT;
			descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descriptorRanges[2].BaseShaderRegister = 0;
			descriptorRanges[2].RegisterSpace = 0;
			descriptorRanges[2].NumDescriptors = GPU_RESOURCE_HEAP_UAV_COUNT;
			descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			UINT paramCount = 2;
			D3D12_ROOT_PARAMETER* params = new D3D12_ROOT_PARAMETER[paramCount];
			params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			params[0].DescriptorTable.NumDescriptorRanges = descriptorRangeCount;
			params[0].DescriptorTable.pDescriptorRanges = descriptorRanges;

			params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			params[1].DescriptorTable.NumDescriptorRanges = 1;
			params[1].DescriptorTable.pDescriptorRanges = &samplerRange;

			D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
			rootSigDesc.NumStaticSamplers = 0;
			rootSigDesc.NumParameters = paramCount;
			rootSigDesc.pParameters = params;
			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			ComPtr<ID3D12RootSignature> computeRootSig;
			ID3DBlob* rootSigBlob;
			ID3DBlob* rootSigError;
			ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &rootSigError));
			ThrowIfFailed(device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSig)));

			SAFE_DELETE_ARRAY(descriptorRanges);
			SAFE_DELETE_ARRAY(params);

			return computeRootSig;
		}
	}

	Microsoft::WRL::ComPtr<ID3D12Fence> GraphicsDevice_DX12::CreateFence(ComPtr<ID3D12Device> device)
	{
		ComPtr<ID3D12Fence> fence;

		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		return fence;
	}

	HANDLE GraphicsDevice_DX12::CreateEventHandle()
	{
		HANDLE fenceEvent;

		fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent && "Failed to create fence event.");

		return fenceEvent;
	}

	void GraphicsDevice_DX12::CreateNullResources(ComPtr<ID3D12Device> device)
	{
		D3D12_SAMPLER_DESC sampler_desc = {};
		sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		m_nullSampler = new D3D12_CPU_DESCRIPTOR_HANDLE;
		m_nullSampler->ptr = SamplerAllocator->Allocate();
		device->CreateSampler(&sampler_desc, *m_nullSampler);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
		m_nullCBV = new D3D12_CPU_DESCRIPTOR_HANDLE;
		m_nullCBV->ptr = ResourceAllocator->Allocate();
		device->CreateConstantBufferView(&cbv_desc, *m_nullCBV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_R32_UINT;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		m_nullSRV = new D3D12_CPU_DESCRIPTOR_HANDLE;
		m_nullSRV->ptr = ResourceAllocator->Allocate();
		device->CreateShaderResourceView(nullptr, &srv_desc, *m_nullSRV);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = DXGI_FORMAT_R32_UINT;
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		m_nullUAV = new D3D12_CPU_DESCRIPTOR_HANDLE;
		m_nullUAV->ptr = ResourceAllocator->Allocate();
		device->CreateUnorderedAccessView(nullptr, nullptr, &uav_desc, *m_nullUAV);
	}

	void GraphicsDevice_DX12::SetupForDraw()
	{
		GetFrameResources().ResourceDescriptorsGPU->Validate(GetDevice(), GetCommandList());
		GetFrameResources().SamplerDescriptorsGPU->Validate(GetDevice(), GetCommandList());
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void GraphicsDevice_DX12::UpdateRenderTargetViews(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr <ID3D12DescriptorHeap> descriptorHeap)
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame
		for (UINT32 n = 0; n < st_frameCount; ++n)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	void GraphicsDevice_DX12::UpdateDepthStencil(ComPtr<ID3D12Device> device, ComPtr <ID3D12DescriptorHeap> descriptorHeap, UINT backBufferWidth, UINT backBufferHeight)
	{
		if (m_depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			// Allocate 2-D surface as the depth/stencil buffer and create a depth/stencil view on this surface
			CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( ConvertFormat( m_depthStencilFormat ), backBufferWidth, backBufferHeight, 1, 1 );
			
			depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = ConvertFormat( m_depthStencilFormat );
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;

			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(m_depthStencil.GetAddressOf())));

			// Create descriptor to mip level 0 of entire resource using the format of the resource
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = ConvertFormat( m_depthStencilFormat );
			dsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	void GraphicsDevice_DX12::UpdateViewportAndScissor(UINT backBufferWidth, UINT backBufferHeight)
	{
		// Update the viewport transform to cover the client area.
		m_screenViewport.TopLeftX = 0;
		m_screenViewport.TopLeftY = 0;
		m_screenViewport.Width = static_cast<float>(backBufferWidth);
		m_screenViewport.Height = static_cast<float>(backBufferHeight);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;

		m_scissorRect = { 0, 0, (LONG)backBufferWidth, (LONG)backBufferHeight };
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::Flush()
	{
		// Schedule a Signal command in the queue.
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

		// Wait until the fence has been processed.
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		m_fenceValues[m_frameIndex]++;
	}

	void GraphicsDevice_DX12::MoveToNextFrame()
	{
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

		// Update the frame index.
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_fenceValues[m_frameIndex] = currentFenceValue + 1;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::PresentBegin()
	{
		ThrowIfFailed(GetCommandAllocator()->Reset());

		ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

		ID3D12DescriptorHeap* heaps[] = {
			GetFrameResources().ResourceDescriptorsGPU->m_heapGPU.Get(), GetFrameResources().SamplerDescriptorsGPU->m_heapGPU.Get()
		};
		GetCommandList()->SetDescriptorHeaps(ARRAYSIZE(heaps), heaps);

		GetCommandList()->SetGraphicsRootSignature(m_graphicsRootSig.Get());
		GetCommandList()->SetComputeRootSignature(m_computeRootSig.Get());

		D3D12_CPU_DESCRIPTOR_HANDLE nullDescriptors[] = {
			*m_nullSampler, *m_nullCBV, *m_nullSRV, *m_nullUAV
		};
		GetFrameResources().ResourceDescriptorsGPU->Reset(m_device, nullDescriptors);
		GetFrameResources().SamplerDescriptorsGPU->Reset(m_device, nullDescriptors);

		GetCommandList()->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentRenderTarget().Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		{
			// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
			GetCommandList()->RSSetViewports(1, &m_screenViewport);
			GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
		}

		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), GetCurrentFrameIndex(), GetRTVDescriptorSize());

			// Record commands.
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			GetCommandList()->ClearDepthStencilView(GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			// Set the back buffer as the render target.
			GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	void GraphicsDevice_DX12::PresentEnd()
	{
		GetCommandList()->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentRenderTarget().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT) );

		ThrowIfFailed(GetCommandList()->Close());

		// Execute command list
		ID3D12CommandList* ppCommandLists[] = { GetCommandList().Get() };
		GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present
		ThrowIfFailed(GetSwapChain()->Present(st_useVsync, 0));

		MoveToNextFrame();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::BindVertexBuffers(GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets /*=nullptr*/)
	{
		assert(count <= 8);
		D3D12_VERTEX_BUFFER_VIEW res[8] = { 0 };
		for (int i = 0; i < count; ++i)
		{
			if (vertexBuffers[i] != nullptr)
			{
				res[i].BufferLocation = vertexBuffers[i]->m_resource->GetGPUVirtualAddress();
				res[i].SizeInBytes = vertexBuffers[i]->m_desc.ByteWidth;
				if (offsets != nullptr)
				{
					res[i].BufferLocation += (D3D12_GPU_VIRTUAL_ADDRESS)offsets[i];
					res[i].SizeInBytes -= offsets[i];
				}
				res[i].StrideInBytes = strides[i];
			}
		}
		GetCommandList()->IASetVertexBuffers(static_cast<UINT>(slot), static_cast<UINT>(count), res);
	}

	void GraphicsDevice_DX12::BindIndexBuffer(GPUBuffer* indexBuffer, const FORMAT format, UINT offset)
	{
		D3D12_INDEX_BUFFER_VIEW res = {};
		if (indexBuffer != nullptr)
		{
			res.BufferLocation = indexBuffer->m_resource->GetGPUVirtualAddress() + (D3D12_GPU_VIRTUAL_ADDRESS)offset;
			res.Format = (format == FORMAT_R16_UINT) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			res.SizeInBytes = indexBuffer->m_desc.ByteWidth;
		}
		GetCommandList()->IASetIndexBuffer(&res);
	}

	void GraphicsDevice_DX12::BindConstantBuffer(SHADERSTAGE stage, GPUBuffer* buffer, int slot)
	{
		assert(slot < GPU_RESOURCE_HEAP_CBV_COUNT);

		if (buffer != nullptr && buffer->m_resource != nullptr)
		{
			GetFrameResources().ResourceDescriptorsGPU->Update(stage, slot,	buffer->m_cbv, GetDevice(), GetCommandList());
		}
	}

	void GraphicsDevice_DX12::BindGraphicsPSO(GraphicsPSO* pso)
	{
		GetCommandList()->SetPipelineState(pso->m_pso.Get());
		GetCommandList()->IASetPrimitiveTopology(ConvertPrimitiveTopology(pso->m_desc.PT));
	}

	void GraphicsDevice_DX12::BindComputePSO(ComputePSO* pso)
	{
		GetCommandList()->SetPipelineState(pso->m_pso.Get());
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::Draw(int vertexCount, UINT startVertexLocation)
	{
		SetupForDraw();

		GetCommandList()->DrawInstanced(static_cast<UINT>(vertexCount), 1, startVertexLocation, 0);
	}

	void GraphicsDevice_DX12::DrawIndexed(int indexCount, UINT startIndexLocation, UINT baseVertexLocation)
	{
		SetupForDraw();

		GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(indexCount), 1, startIndexLocation, baseVertexLocation, 0);
	}

	void GraphicsDevice_DX12::DrawInstanced(int vertexCount, int instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
	{
		SetupForDraw();

		GetCommandList()->DrawInstanced(static_cast<UINT>(vertexCount), static_cast<UINT>(instanceCount), startVertexLocation, startInstanceLocation);
	}

	void GraphicsDevice_DX12::DrawIndexedInstanced(int indexCount, int instanceCount, UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation)
	{
		SetupForDraw();

		GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(indexCount), static_cast<UINT>(instanceCount), startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::CreateBlob(UINT byteSize, CPUBuffer* buffer)
	{
		ThrowIfFailed(D3DCreateBlob( byteSize, &buffer->m_blob) );
	}

	void GraphicsDevice_DX12::CreateBuffer(const GPUBufferDesc& desc, const SubresourceData* initialData, GPUBuffer* buffer)
	{
		assert(buffer != nullptr);

		buffer->m_desc = desc;

		uint32_t alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		if(desc.BindFlags & BIND_CONSTANT_BUFFER)
		{
			alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		}
		UINT64 alignedSize = Align(desc.ByteWidth, alignment);

		D3D12_HEAP_PROPERTIES heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
		if (desc.BindFlags & BIND_UNORDERED_ACCESS)
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize, resFlags);
		
		D3D12_RESOURCE_STATES resState = D3D12_RESOURCE_STATE_COMMON;

		// Create the actual default buffer resource.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapDesc,
			heapFlags,
			&resDesc,
			resState,
			nullptr,
			IID_PPV_ARGS(buffer->m_resource.GetAddressOf())));

		if (initialData != nullptr)
		{
			// Issue data copy on request
			// #TODO: This is not thread safe
			UINT8* dest = BufferUploader->Allocate(desc.ByteWidth, alignment);
			memcpy(dest, initialData->SysMem, desc.ByteWidth);
			m_commandList->CopyBufferRegion(buffer->m_resource.Get(), 0, BufferUploader->m_resource.Get(), BufferUploader->CalculateOffset(dest), desc.ByteWidth);
		}

		// Create resource views if needed
		if (desc.BindFlags & BIND_CONSTANT_BUFFER)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = buffer->m_resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = (UINT)alignedSize;

			buffer->m_cbv = new D3D12_CPU_DESCRIPTOR_HANDLE;
			buffer->m_cbv->ptr = ResourceAllocator->Allocate();
			m_device->CreateConstantBufferView(
				&cbvDesc,
				*buffer->m_cbv);
			assert(buffer->m_cbv);
		}

		if (desc.BindFlags & BIND_SHADER_RESOURCE)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;

			if (desc.MiscFlags & RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
			{
				// This is raw buffer
				srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;

				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
				srvDesc.Buffer.NumElements = desc.ByteWidth / 4;
			}
			else if (desc.MiscFlags & RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// It's a structured buffer
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				
				srvDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
				srvDesc.Buffer.StructureByteStride = desc.StructureByteStride;
			}
			else
			{
				// It's a typed buffer
				srvDesc.Format = ConvertFormat(desc.Format);

				srvDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
			}

			buffer->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
			buffer->m_srv->ptr = ResourceAllocator->Allocate();
			m_device->CreateShaderResourceView(
				buffer->m_resource.Get(), 
				&srvDesc, 
				*buffer->m_srv);
			assert(buffer->m_srv);
		}

		if (desc.BindFlags & BIND_UNORDERED_ACCESS)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;

			if (desc.MiscFlags & RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
			{
				// This is a Raw Buffer
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		
				uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
			}
			else if (desc.MiscFlags & RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// This is a Structured Buffer
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;

				uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
				uavDesc.Buffer.StructureByteStride = desc.StructureByteStride;
			}
			else
			{
				// This is a Typed Buffer
				uavDesc.Format = ConvertFormat(desc.Format);

				uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
			}
			
			buffer->m_uav = new D3D12_CPU_DESCRIPTOR_HANDLE;
			buffer->m_uav->ptr = ResourceAllocator->Allocate();
			m_device->CreateUnorderedAccessView(
				buffer->m_resource.Get(),
				nullptr,
				&uavDesc,
				*buffer->m_uav);
			assert(buffer->m_uav);
		}
	}

	void GraphicsDevice_DX12::CreateShader(const std::wstring& filename, BaseShader* shader)
	{
		assert(shader != nullptr);
		
		switch (shader->GetShaderStage())
		{
			case SHADERSTAGE::VS:
			{
				D3D_SHADER_MACRO _macros[2] = { "VERTEX_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "vs_main", "vs_5_0");
			}
			break;
			case SHADERSTAGE::PS:
			{
				D3D_SHADER_MACRO _macros[2] = { "PIXEL_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "ps_main", "ps_5_0");
			}
			break;
			case SHADERSTAGE::CS:
			{
				D3D_SHADER_MACRO _macros[2] = { "COMPUTE_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "cs_main", "cs_5_0");
			}
			break;
			case SHADERSTAGE::GS:
			{
				D3D_SHADER_MACRO _macros[2] = { "GEOMETRY_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "gs_main", "gs_5_0");
			}
			break;
			case SHADERSTAGE::HS:
			{
				D3D_SHADER_MACRO _macros[2] = { "HULL_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "hs_main", "hs_5_0");
			}
			break;
			case SHADERSTAGE::DS:
			{
				D3D_SHADER_MACRO _macros[2] = { "DOMAIN_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "ds_main", "ds_5_0");
			}
			break;
			default:
				break;
		}
	}

	void GraphicsDevice_DX12::CreateInputLayout(const VertexInputLayoutDesc* inputElementDescs, UINT numElements, VertexLayout* inputLayout)
	{
		inputLayout->m_desc.reserve((size_t)numElements);
		for (UINT i = 0; i < numElements; ++i)
		{
			inputLayout->m_desc.push_back(inputElementDescs[i]);
		}
	}

	void GraphicsDevice_DX12::CreateGraphicsPSO(const GraphicsPSODesc* psoDesc, GraphicsPSO* pso)
	{
		pso->m_desc = *psoDesc;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		if (psoDesc->VS != nullptr)
		{
			desc.VS.pShaderBytecode = psoDesc->VS->m_blob->GetBufferPointer();
			desc.VS.BytecodeLength = psoDesc->VS->m_blob->GetBufferSize();
		}
		if (psoDesc->PS != nullptr)
		{
			desc.PS.pShaderBytecode = psoDesc->PS->m_blob->GetBufferPointer();
			desc.PS.BytecodeLength = psoDesc->PS->m_blob->GetBufferSize();
		}
		if (psoDesc->GS != nullptr)
		{
			desc.GS.pShaderBytecode = psoDesc->GS->m_blob->GetBufferPointer();
			desc.GS.BytecodeLength = psoDesc->GS->m_blob->GetBufferSize();
		}
		if (psoDesc->HS != nullptr)
		{
			desc.HS.pShaderBytecode = psoDesc->HS->m_blob->GetBufferPointer();
			desc.HS.BytecodeLength = psoDesc->HS->m_blob->GetBufferSize();
		}
		if (psoDesc->DS != nullptr)
		{
			desc.DS.pShaderBytecode = psoDesc->DS->m_blob->GetBufferPointer();
			desc.DS.BytecodeLength = psoDesc->DS->m_blob->GetBufferSize();
		}

		RasterizerStateDesc pRasterizerStateDesc = psoDesc->RS != nullptr ? psoDesc->RS->GetDesc() : RasterizerStateDesc();
		desc.RasterizerState.FillMode = ConvertFillMode(pRasterizerStateDesc.FillMode);
		desc.RasterizerState.CullMode = ConvertCullMode(pRasterizerStateDesc.CullMode);
		desc.RasterizerState.FrontCounterClockwise = pRasterizerStateDesc.FrontCounterClockwise;
		desc.RasterizerState.DepthBias = pRasterizerStateDesc.DepthBias;
		desc.RasterizerState.DepthBiasClamp = pRasterizerStateDesc.DepthBiasClamp;
		desc.RasterizerState.SlopeScaledDepthBias = pRasterizerStateDesc.SlopeScaledDepthBias;
		desc.RasterizerState.DepthClipEnable = pRasterizerStateDesc.DepthClipEnable;
		desc.RasterizerState.MultisampleEnable = pRasterizerStateDesc.MultisampleEnable;
		desc.RasterizerState.AntialiasedLineEnable = pRasterizerStateDesc.AntialiasedLineEnable;
		desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		desc.RasterizerState.ForcedSampleCount = pRasterizerStateDesc.ForcedSampleCount;

		DepthStencilStateDesc pDepthStencilStateDesc = psoDesc->DSS != nullptr ? psoDesc->DSS->GetDesc() : DepthStencilStateDesc();
		desc.DepthStencilState.DepthEnable = pDepthStencilStateDesc.DepthEnable;
		desc.DepthStencilState.DepthWriteMask = ConvertDepthWriteMask(pDepthStencilStateDesc.DepthWriteMask);
		desc.DepthStencilState.DepthFunc = ConvertComparisonFunc(pDepthStencilStateDesc.DepthFunc);
		desc.DepthStencilState.StencilEnable = pDepthStencilStateDesc.StencilEnable;
		desc.DepthStencilState.StencilReadMask = pDepthStencilStateDesc.StencilReadMask;
		desc.DepthStencilState.StencilWriteMask = pDepthStencilStateDesc.StencilWriteMask;
		desc.DepthStencilState.FrontFace.StencilDepthFailOp = ConvertStencilOp(pDepthStencilStateDesc.FrontFace.StencilDepthFailOp);
		desc.DepthStencilState.FrontFace.StencilFailOp = ConvertStencilOp(pDepthStencilStateDesc.FrontFace.StencilFailOp);
		desc.DepthStencilState.FrontFace.StencilFunc = ConvertComparisonFunc(pDepthStencilStateDesc.FrontFace.StencilFunc);
		desc.DepthStencilState.FrontFace.StencilPassOp = ConvertStencilOp(pDepthStencilStateDesc.FrontFace.StencilPassOp);
		desc.DepthStencilState.BackFace.StencilDepthFailOp = ConvertStencilOp(pDepthStencilStateDesc.BackFace.StencilDepthFailOp);
		desc.DepthStencilState.BackFace.StencilFailOp = ConvertStencilOp(pDepthStencilStateDesc.BackFace.StencilFailOp);
		desc.DepthStencilState.BackFace.StencilFunc = ConvertComparisonFunc(pDepthStencilStateDesc.BackFace.StencilFunc);
		desc.DepthStencilState.BackFace.StencilPassOp = ConvertStencilOp(pDepthStencilStateDesc.BackFace.StencilPassOp);

		BlendStateDesc pBlendStateDesc = psoDesc->BS != nullptr ? psoDesc->BS->GetDesc() : BlendStateDesc();
		desc.BlendState.AlphaToCoverageEnable = pBlendStateDesc.AlphaToCoverageEnable;
		desc.BlendState.IndependentBlendEnable = pBlendStateDesc.IndependentBlendEnable;
		for (int i = 0; i < 8; ++i)
		{
			desc.BlendState.RenderTarget[i].BlendEnable = pBlendStateDesc.RenderTarget[i].BlendEnable;
			desc.BlendState.RenderTarget[i].SrcBlend = ConvertBlend(pBlendStateDesc.RenderTarget[i].SrcBlend);
			desc.BlendState.RenderTarget[i].DestBlend = ConvertBlend(pBlendStateDesc.RenderTarget[i].DestBlend);
			desc.BlendState.RenderTarget[i].BlendOp = ConvertBlendOp(pBlendStateDesc.RenderTarget[i].BlendOp);
			desc.BlendState.RenderTarget[i].SrcBlendAlpha = ConvertBlend(pBlendStateDesc.RenderTarget[i].SrcBlendAlpha);
			desc.BlendState.RenderTarget[i].DestBlendAlpha = ConvertBlend(pBlendStateDesc.RenderTarget[i].DestBlendAlpha);
			desc.BlendState.RenderTarget[i].BlendOpAlpha = ConvertBlendOp(pBlendStateDesc.RenderTarget[i].BlendOpAlpha);
			desc.BlendState.RenderTarget[i].RenderTargetWriteMask = ParseColorWriteMask(pBlendStateDesc.RenderTarget[i].RenderTargetWriteMask);
		}

		D3D12_INPUT_ELEMENT_DESC* elements = nullptr;
		if (psoDesc->IL != nullptr)
		{
			desc.InputLayout.NumElements = (UINT)psoDesc->IL->m_desc.size();
			elements = new D3D12_INPUT_ELEMENT_DESC[desc.InputLayout.NumElements];
			for (UINT i = 0; i < desc.InputLayout.NumElements; ++i)
			{
				elements[i].SemanticName = psoDesc->IL->m_desc[i].SemanticName;
				elements[i].SemanticIndex = psoDesc->IL->m_desc[i].SemanticIndex;
				elements[i].Format = ConvertFormat(psoDesc->IL->m_desc[i].Format);
				elements[i].InputSlot = psoDesc->IL->m_desc[i].InputSlot;
				elements[i].AlignedByteOffset = psoDesc->IL->m_desc[i].InputSlot;
				if (elements[i].AlignedByteOffset == APPEND_ALIGNED_ELEMENT)
					elements[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				elements[i].InputSlotClass = ConvertInputClassification( psoDesc->IL->m_desc[i].InputSlotClass );
				elements[i].InstanceDataStepRate = psoDesc->IL->m_desc[i].InstanceDataStepRate;
			}
			desc.InputLayout.pInputElementDescs = elements;
		}

		desc.NumRenderTargets = psoDesc->NumRTs;
		for (UINT i = 0; i < desc.NumRenderTargets; ++i)
			desc.RTVFormats[i] = ConvertFormat(psoDesc->RTFormats[i]);
		desc.DSVFormat = ConvertFormat(psoDesc->DSFormat);

		desc.SampleDesc.Count = psoDesc->SampleDesc.Count;
		desc.SampleDesc.Quality = psoDesc->SampleDesc.Quality;
		desc.SampleMask = psoDesc->SampleMask;

		desc.PrimitiveTopologyType = ConvertPrimitiveTopologyType(psoDesc->PT);

		desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		desc.pRootSignature = m_graphicsRootSig.Get();

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso->m_pso)));

		SAFE_DELETE_ARRAY(elements);
	}

	void GraphicsDevice_DX12::CreateComputePSO(const ComputePSODesc* psoDesc, ComputePSO* pso)
	{
		pso->m_desc = *psoDesc;

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};

		if (psoDesc->CS != nullptr)
		{
			desc.CS.pShaderBytecode = psoDesc->CS->GetBufferPtr();
			desc.CS.BytecodeLength = psoDesc->CS->GetSize();
		}

		desc.pRootSignature = m_computeRootSig.Get();

		ThrowIfFailed(m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso->m_pso)));
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::TransitionBarrier(GPUResource* resource, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{
		if (resource != nullptr)
		{
			GetCommandList()->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					resource->m_resource.Get(),
					ConvertResourceStates(stateBefore),
					ConvertResourceStates(stateAfter)));
		}
	}

	void GraphicsDevice_DX12::TransitionBarriers(GPUResource* const* resources, UINT numBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{
		if (resources != nullptr)
		{
			D3D12_RESOURCE_BARRIER barriers[8];
			for (UINT i = 0; i < numBarriers; ++i)
			{
				barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[i].Transition.pResource = resources[i]->m_resource.Get();
				barriers[i].Transition.StateBefore = ConvertResourceStates(stateBefore);
				barriers[i].Transition.StateAfter = ConvertResourceStates(stateAfter);
				barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			GetCommandList()->ResourceBarrier(numBarriers, barriers);
		}

	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::UpdateBuffer(GPUBuffer* buffer, const void* data, int dataSize /*= -1*/)
	{
		assert(buffer->m_desc.Usage != USAGE_IMMUTABLE && "Cannot update IMMUTABLE GpuBuffer.");
		assert((int)buffer->m_desc.ByteWidth >= dataSize || dataSize < 0 && "Data size is too big.");

		if (dataSize == 0)
			return;

		dataSize = MathHelper::Min((int)buffer->m_desc.ByteWidth, dataSize);
		dataSize = (dataSize >= 0 ? dataSize : buffer->m_desc.ByteWidth);

		UINT64 alignment = buffer->m_desc.BindFlags & BIND_CONSTANT_BUFFER ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = buffer->m_resource.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		if (buffer->m_desc.BindFlags & BIND_CONSTANT_BUFFER || buffer->m_desc.BindFlags & BIND_VERTEX_BUFFER)
		{
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		}
		else if (buffer->m_desc.BindFlags & BIND_INDEX_BUFFER)
		{
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
		}
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		
		GetCommandList()->ResourceBarrier(1, &barrier);

		// allocate memory from resource buffer
		UINT8* dest = GetFrameResources().ResourceBuffer->Allocate(dataSize, alignment);
		memcpy(dest, data, dataSize);
		GetCommandList()->CopyBufferRegion(
			buffer->m_resource.Get(), 0, 
			GetFrameResources().ResourceBuffer->m_resource.Get(), 
			GetFrameResources().ResourceBuffer->CalculateOffset(dest),
			dataSize);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

		GetCommandList()->ResourceBarrier(1, &barrier);
	}
}