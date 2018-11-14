#include "stdafx.h"
#include "DXHelper.h"
#include "D3DUtils.h"
#include "MathHelper.h"
#include "BaseWindow.h"
#include "GraphicsDevice_DX12.h"
#include "GraphicsResource.h"
#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"
#include "Image.h"
#include "Utils.h"

# define USE_PIX
#include <pix3.h>

namespace Graphics
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
	inline D3D12_FILTER ConvertFilter(FILTER value)
	{
		switch (value)
		{
		case FILTER_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
			break;
		case FILTER_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case FILTER_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case FILTER_ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
			break;
		case FILTER_COMPARISON_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			break;
		case FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			break;
		case FILTER_COMPARISON_ANISOTROPIC:
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
			break;
		case FILTER_MINIMUM_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
			break;
		case FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
			break;
		case FILTER_MINIMUM_ANISOTROPIC:
			return D3D12_FILTER_MINIMUM_ANISOTROPIC;
			break;
		case FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
			break;
		case FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
			break;
		case FILTER_MAXIMUM_ANISOTROPIC:
			return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
			break;
		default:
			break;
		}
		return D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
	inline D3D12_TEXTURE_ADDRESS_MODE ConvertTextureAddressMode(TEXTURE_ADDRESS_MODE value)
	{
		switch (value)
		{
		case TEXTURE_ADDRESS_WRAP:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			break;
		case TEXTURE_ADDRESS_MIRROR:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			break;
		case TEXTURE_ADDRESS_CLAMP:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			break;
		case TEXTURE_ADDRESS_BORDER:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			break;
		case TEXTURE_ADDRESS_MIRROR_ONCE:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			break;
		default:
			break;
		}
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	inline D3D12_SUBRESOURCE_DATA ConvertSubresourceData(const SubresourceData& initialData)
	{
		D3D12_SUBRESOURCE_DATA data;
		data.pData = initialData.SysMem;
		data.RowPitch = initialData.SysMemPitch;
		data.SlicePitch = initialData.SysMemSlicePitch;

		return data;
	}
	inline FORMAT ConvertFormat_Inv(DXGI_FORMAT value)
	{
		switch (value)
		{
		case DXGI_FORMAT_UNKNOWN:
			return FORMAT_UNKNOWN;
			break;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return FORMAT_R32G32B32A32_TYPELESS;
			break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return FORMAT_R32G32B32A32_FLOAT;
			break;
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return FORMAT_R32G32B32A32_UINT;
			break;
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return FORMAT_R32G32B32A32_SINT;
			break;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return FORMAT_R32G32B32_TYPELESS;
			break;
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return FORMAT_R32G32B32_FLOAT;
			break;
		case DXGI_FORMAT_R32G32B32_UINT:
			return FORMAT_R32G32B32_UINT;
			break;
		case DXGI_FORMAT_R32G32B32_SINT:
			return FORMAT_R32G32B32_SINT;
			break;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return FORMAT_R16G16B16A16_TYPELESS;
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return FORMAT_R16G16B16A16_FLOAT;
			break;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return FORMAT_R16G16B16A16_UNORM;
			break;
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return FORMAT_R16G16B16A16_UINT;
			break;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return FORMAT_R16G16B16A16_SNORM;
			break;
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return FORMAT_R16G16B16A16_SINT;
			break;
		case DXGI_FORMAT_R32G32_TYPELESS:
			return FORMAT_R32G32_TYPELESS;
			break;
		case DXGI_FORMAT_R32G32_FLOAT:
			return FORMAT_R32G32_FLOAT;
			break;
		case DXGI_FORMAT_R32G32_UINT:
			return FORMAT_R32G32_UINT;
			break;
		case DXGI_FORMAT_R32G32_SINT:
			return FORMAT_R32G32_SINT;
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return FORMAT_R32G8X24_TYPELESS;
			break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return FORMAT_R32_FLOAT_X8X24_TYPELESS;
			break;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return FORMAT_X32_TYPELESS_G8X24_UINT;
			break;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return FORMAT_R10G10B10A2_TYPELESS;
			break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return FORMAT_R10G10B10A2_UNORM;
			break;
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return FORMAT_R10G10B10A2_UINT;
			break;
		case DXGI_FORMAT_R11G11B10_FLOAT:
			return FORMAT_R11G11B10_FLOAT;
			break;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return FORMAT_R8G8B8A8_TYPELESS;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return FORMAT_R8G8B8A8_UNORM;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return FORMAT_R8G8B8A8_UNORM_SRGB;
			break;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return FORMAT_R8G8B8A8_UINT;
			break;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return FORMAT_R8G8B8A8_SNORM;
			break;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return FORMAT_R8G8B8A8_SINT;
			break;
		case DXGI_FORMAT_R16G16_TYPELESS:
			return FORMAT_R16G16_TYPELESS;
			break;
		case DXGI_FORMAT_R16G16_FLOAT:
			return FORMAT_R16G16_FLOAT;
			break;
		case DXGI_FORMAT_R16G16_UNORM:
			return FORMAT_R16G16_UNORM;
			break;
		case DXGI_FORMAT_R16G16_UINT:
			return FORMAT_R16G16_UINT;
			break;
		case DXGI_FORMAT_R16G16_SNORM:
			return FORMAT_R16G16_SNORM;
			break;
		case DXGI_FORMAT_R16G16_SINT:
			return FORMAT_R16G16_SINT;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			return FORMAT_R32_TYPELESS;
			break;
		case DXGI_FORMAT_D32_FLOAT:
			return FORMAT_D32_FLOAT;
			break;
		case DXGI_FORMAT_R32_FLOAT:
			return FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_R32_UINT:
			return FORMAT_R32_UINT;
			break;
		case DXGI_FORMAT_R32_SINT:
			return FORMAT_R32_SINT;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			return FORMAT_R24G8_TYPELESS;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return FORMAT_D24_UNORM_S8_UINT;
			break;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return FORMAT_X24_TYPELESS_G8_UINT;
			break;
		case DXGI_FORMAT_R8G8_TYPELESS:
			return FORMAT_R8G8_TYPELESS;
			break;
		case DXGI_FORMAT_R8G8_UNORM:
			return FORMAT_R8G8_UNORM;
			break;
		case DXGI_FORMAT_R8G8_UINT:
			return FORMAT_R8G8_UINT;
			break;
		case DXGI_FORMAT_R8G8_SNORM:
			return FORMAT_R8G8_SNORM;
			break;
		case DXGI_FORMAT_R8G8_SINT:
			return FORMAT_R8G8_SINT;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			return FORMAT_R16_TYPELESS;
			break;
		case DXGI_FORMAT_R16_FLOAT:
			return FORMAT_R16_FLOAT;
			break;
		case DXGI_FORMAT_D16_UNORM:
			return FORMAT_D16_UNORM;
			break;
		case DXGI_FORMAT_R16_UNORM:
			return FORMAT_R16_UNORM;
			break;
		case DXGI_FORMAT_R16_UINT:
			return FORMAT_R16_UINT;
			break;
		case DXGI_FORMAT_R16_SNORM:
			return FORMAT_R16_SNORM;
			break;
		case DXGI_FORMAT_R16_SINT:
			return FORMAT_R16_SINT;
			break;
		case DXGI_FORMAT_R8_TYPELESS:
			return FORMAT_R8_TYPELESS;
			break;
		case DXGI_FORMAT_R8_UNORM:
			return FORMAT_R8_UNORM;
			break;
		case DXGI_FORMAT_R8_UINT:
			return FORMAT_R8_UINT;
			break;
		case DXGI_FORMAT_R8_SNORM:
			return FORMAT_R8_SNORM;
			break;
		case DXGI_FORMAT_R8_SINT:
			return FORMAT_R8_SINT;
			break;
		case DXGI_FORMAT_A8_UNORM:
			return FORMAT_A8_UNORM;
			break;
		case DXGI_FORMAT_R1_UNORM:
			return FORMAT_R1_UNORM;
			break;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return FORMAT_R9G9B9E5_SHAREDEXP;
			break;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
			return FORMAT_R8G8_B8G8_UNORM;
			break;
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return FORMAT_G8R8_G8B8_UNORM;
			break;
		case DXGI_FORMAT_BC1_TYPELESS:
			return FORMAT_BC1_TYPELESS;
			break;
		case DXGI_FORMAT_BC1_UNORM:
			return FORMAT_BC1_UNORM;
			break;
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return FORMAT_BC1_UNORM_SRGB;
			break;
		case DXGI_FORMAT_BC2_TYPELESS:
			return FORMAT_BC2_TYPELESS;
			break;
		case DXGI_FORMAT_BC2_UNORM:
			return FORMAT_BC2_UNORM;
			break;
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return FORMAT_BC2_UNORM_SRGB;
			break;
		case DXGI_FORMAT_BC3_TYPELESS:
			return FORMAT_BC3_TYPELESS;
			break;
		case DXGI_FORMAT_BC3_UNORM:
			return FORMAT_BC3_UNORM;
			break;
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return FORMAT_BC3_UNORM_SRGB;
			break;
		case DXGI_FORMAT_BC4_TYPELESS:
			return FORMAT_BC4_TYPELESS;
			break;
		case DXGI_FORMAT_BC4_UNORM:
			return FORMAT_BC4_UNORM;
			break;
		case DXGI_FORMAT_BC4_SNORM:
			return FORMAT_BC4_SNORM;
			break;
		case DXGI_FORMAT_BC5_TYPELESS:
			return FORMAT_BC5_TYPELESS;
			break;
		case DXGI_FORMAT_BC5_UNORM:
			return FORMAT_BC5_UNORM;
			break;
		case DXGI_FORMAT_BC5_SNORM:
			return FORMAT_BC5_SNORM;
			break;
		case DXGI_FORMAT_B5G6R5_UNORM:
			return FORMAT_B5G6R5_UNORM;
			break;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return FORMAT_B5G5R5A1_UNORM;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return FORMAT_B8G8R8A8_UNORM;
			break;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return FORMAT_B8G8R8X8_UNORM;
			break;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
			break;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return FORMAT_B8G8R8A8_TYPELESS;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return FORMAT_B8G8R8A8_UNORM_SRGB;
			break;
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return FORMAT_B8G8R8X8_TYPELESS;
			break;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return FORMAT_B8G8R8X8_UNORM_SRGB;
			break;
		case DXGI_FORMAT_BC6H_TYPELESS:
			return FORMAT_BC6H_TYPELESS;
			break;
		case DXGI_FORMAT_BC6H_UF16:
			return FORMAT_BC6H_UF16;
			break;
		case DXGI_FORMAT_BC6H_SF16:
			return FORMAT_BC6H_SF16;
			break;
		case DXGI_FORMAT_BC7_TYPELESS:
			return FORMAT_BC7_TYPELESS;
			break;
		case DXGI_FORMAT_BC7_UNORM:
			return FORMAT_BC7_UNORM;
			break;
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return FORMAT_BC7_UNORM_SRGB;
			break;
		case DXGI_FORMAT_AYUV:
			return FORMAT_AYUV;
			break;
		case DXGI_FORMAT_Y410:
			return FORMAT_Y410;
			break;
		case DXGI_FORMAT_Y416:
			return FORMAT_Y416;
			break;
		case DXGI_FORMAT_NV12:
			return FORMAT_NV12;
			break;
		case DXGI_FORMAT_P010:
			return FORMAT_P010;
			break;
		case DXGI_FORMAT_P016:
			return FORMAT_P016;
			break;
		case DXGI_FORMAT_420_OPAQUE:
			return FORMAT_420_OPAQUE;
			break;
		case DXGI_FORMAT_YUY2:
			return FORMAT_YUY2;
			break;
		case DXGI_FORMAT_Y210:
			return FORMAT_Y210;
			break;
		case DXGI_FORMAT_Y216:
			return FORMAT_Y216;
			break;
		case DXGI_FORMAT_NV11:
			return FORMAT_NV11;
			break;
		case DXGI_FORMAT_AI44:
			return FORMAT_AI44;
			break;
		case DXGI_FORMAT_IA44:
			return FORMAT_IA44;
			break;
		case DXGI_FORMAT_P8:
			return FORMAT_P8;
			break;
		case DXGI_FORMAT_A8P8:
			return FORMAT_A8P8;
			break;
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return FORMAT_B4G4R4A4_UNORM;
			break;
		case DXGI_FORMAT_FORCE_UINT:
			return FORMAT_FORCE_UINT;
			break;
		default:
			break;
		}
		return FORMAT_UNKNOWN;
	}
	inline TextureDesc ConvertTextureDesc_Inv(const D3D12_RESOURCE_DESC& desc)
	{
		TextureDesc retVal;

		retVal.Format = ConvertFormat_Inv(desc.Format);
		retVal.Width = (UINT)desc.Width;
		retVal.Height = desc.Height;
		retVal.MipLevels = desc.MipLevels;
		retVal.ArraySize = desc.DepthOrArraySize;

		return retVal;
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
		: m_currentFence( 0 )
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
			{
				FrameResources& frameResources = Frames[i];
				frameResources.m_commandAllocator = CreateCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
				frameResources.m_commandList = CreateCommandList(m_device, frameResources.m_commandAllocator, D3D12_COMMAND_LIST_TYPE_DIRECT);
				UpdateRenderTargetViews(m_device, m_swapChain, m_rtvHeap, i);
			}

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
		TextureUploader = new UploadBuffer(m_device.Get(), 1024 * 1024 * 1024);

		CreateNullResources(m_device);

		// Create frame-resident resources
		for (UINT fr = 0; fr < st_frameCount; ++fr)
		{
			Frames[fr].ResourceDescriptorsGPU = new FrameResources::DescriptorTableFrameAllocator(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);
			Frames[fr].SamplerDescriptorsGPU = new FrameResources::DescriptorTableFrameAllocator(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 16);
			Frames[fr].ResourceBuffer = new FrameResources::ResourceFrameAllocator(m_device, 1024 * 1024 * 128);
		}

		InitializeDownsamplePSOs();

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

	void GraphicsDevice_DX12::InitializeDownsamplePSOs()
	{
		ComputePSODesc psoDesc = {};

		psoDesc.CS = new ComputeShader();
		CreateShader(L"Shaders\\DownsampleLinear.hlsl", psoDesc.CS);
		m_linearDownsamplePSO = new ComputePSO();
		CreateComputePSO(&psoDesc, m_linearDownsamplePSO);

		psoDesc.CS = new ComputeShader();
		CreateShader(L"Shaders\\DownsampleGamma.hlsl", psoDesc.CS);
		m_gammaDownsamplePSO = new ComputePSO();
		CreateComputePSO(&psoDesc, m_gammaDownsamplePSO);

		psoDesc.CS = new ComputeShader();
		CreateShader(L"Shaders\\DownsampleArray.hlsl", psoDesc.CS);
		m_arrayDownsamplePSO = new ComputePSO();
		CreateComputePSO(&psoDesc, m_arrayDownsamplePSO);
	}

	void GraphicsDevice_DX12::CreateTexture(UINT width, UINT height, UINT depth, DXGI_FORMAT format, UINT levels)
	{
		assert(depth == 1 || depth == 6);

		// TODO: finish
		assert("Not implemented.");
	}

	void GraphicsDevice_DX12::CreateTextureSRV(Texture* texture, D3D12_SRV_DIMENSION dimension, UINT mostDetailedMip, UINT mipLevels)
	{
		const TextureDesc& desc = texture->m_desc;
		const UINT effectiveMipLevels = (mipLevels > 0) ? mipLevels : (desc.MipLevels - mostDetailedMip);
		assert(desc.BindFlags & BIND_SHADER_RESOURCE);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = ConvertFormat(desc.Format);
		srvDesc.ViewDimension = dimension;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		switch (dimension) {
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			srvDesc.Texture2D.MostDetailedMip = mostDetailedMip;
			srvDesc.Texture2D.MipLevels = effectiveMipLevels;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			srvDesc.Texture2DArray.MostDetailedMip = mostDetailedMip;
			srvDesc.Texture2DArray.MipLevels = effectiveMipLevels;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.ArraySize = desc.ArraySize;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			assert(desc.ArraySize == 6);
			srvDesc.TextureCube.MostDetailedMip = mostDetailedMip;
			srvDesc.TextureCube.MipLevels = effectiveMipLevels;
			break;
		default:
			assert(0);
		}

		texture->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
		texture->m_srv->ptr = ResourceAllocator->Allocate();

		m_device->CreateShaderResourceView(texture->m_resource.Get(), &srvDesc, *texture->m_srv);
	}

	void GraphicsDevice_DX12::CreateTextureUAV(Texture* texture, UINT mipSlice)
	{
		const TextureDesc& desc = texture->m_desc;
		assert(desc.BindFlags & BIND_UNORDERED_ACCESS);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = ConvertFormat(desc.Format);
		if (desc.ArraySize > 1) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = mipSlice;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = desc.ArraySize;
		}
		else {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = mipSlice;
		}

		texture->m_uav = new D3D12_CPU_DESCRIPTOR_HANDLE;
		texture->m_uav->ptr = ResourceAllocator->Allocate();

		m_device->CreateUnorderedAccessView(texture->m_resource.Get(), nullptr, &uavDesc, *texture->m_uav);
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

	void GraphicsDevice_DX12::SetupForDispatch()
	{
		GetFrameResources().ResourceDescriptorsGPU->Validate(GetDevice(), GetCommandList());
		GetFrameResources().SamplerDescriptorsGPU->Validate(GetDevice(), GetCommandList());
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void GraphicsDevice_DX12::UpdateRenderTargetViews(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr <ID3D12DescriptorHeap> descriptorHeap, UINT n)
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), n, rtvDescriptorSize);

		// Create a RTV for each frame
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&Frames[n].m_backBuffer)));
		m_device->CreateRenderTargetView(Frames[n].m_backBuffer.Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
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
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), GetFenceValue()));

		// Wait until the fence has been processed.
		ThrowIfFailed(m_fence->SetEventOnCompletion(GetFenceValue(), m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		Frames[m_frameIndex].m_fenceValue++;
	}

	void GraphicsDevice_DX12::MoveToNextFrame()
	{
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = GetFenceValue();
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

		// Update the frame index.
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_fence->GetCompletedValue() < GetFenceValue())
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(GetFenceValue(), m_fenceEvent));
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		Frames[m_frameIndex].m_fenceValue = currentFenceValue + 1;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::PresentBegin()
	{
		FrameResources& frameResources = GetFrameResources();
		auto cmdAllocator = frameResources.GetCommandAllocator();
		auto cmdList = frameResources.GetCommandList();

		ThrowIfFailed(cmdAllocator->Reset());

		ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

		ID3D12DescriptorHeap* heaps[] = {
			GetFrameResources().ResourceDescriptorsGPU->m_heapGPU.Get(), GetFrameResources().SamplerDescriptorsGPU->m_heapGPU.Get()
		};
		cmdList->SetDescriptorHeaps(ARRAYSIZE(heaps), heaps);

		cmdList->SetGraphicsRootSignature(m_graphicsRootSig.Get());
		cmdList->SetComputeRootSignature(m_computeRootSig.Get());

		D3D12_CPU_DESCRIPTOR_HANDLE nullDescriptors[] = {
			*m_nullSampler, *m_nullCBV, *m_nullSRV, *m_nullUAV
		};
		GetFrameResources().ResourceDescriptorsGPU->Reset(m_device, nullDescriptors);
		GetFrameResources().SamplerDescriptorsGPU->Reset(m_device, nullDescriptors);
		GetFrameResources().ResourceBuffer->Clear();

		cmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				frameResources.GetRenderTarget().Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		{
			// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
			cmdList->RSSetViewports(1, &m_screenViewport);
			cmdList->RSSetScissorRects(1, &m_scissorRect);
		}

		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), GetCurrentFrameIndex(), GetRTVDescriptorSize());

			// Record commands.
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			cmdList->ClearDepthStencilView(GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			// Set the back buffer as the render target.
			cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	void GraphicsDevice_DX12::PresentEnd()
	{
		FrameResources& frameResources = GetFrameResources();
		auto cmdList = frameResources.GetCommandList();

		cmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				frameResources.GetRenderTarget().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT) );

		ThrowIfFailed(cmdList->Close());

		// Execute command list
		ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
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

	void GraphicsDevice_DX12::BindResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex /*= -1*/)
	{
		assert(slot < GPU_RESOURCE_HEAP_SRV_COUNT);

		if (resource != nullptr && resource->m_resource != nullptr)
		{
			if (arrayIndex < 0)
			{
				if (resource->m_srv != nullptr)
				{
					GetFrameResources().ResourceDescriptorsGPU->Update(stage, GPU_RESOURCE_HEAP_CBV_COUNT + slot,
						resource->m_srv, m_device, GetCommandList());
				}
			}
			else
			{
				// TODO: implement array support
			}
		}
	}

	void GraphicsDevice_DX12::BindUnorderedAccessResource(GPUResource* resource, int slot, int arrayIndex)
	{
		assert(slot < GPU_RESOURCE_HEAP_UAV_COUNT);

		if (resource != nullptr && resource->m_resource != nullptr)
		{
			if (arrayIndex < 0)
			{
				if (resource->m_uav != nullptr)
				{
					GetFrameResources().ResourceDescriptorsGPU->Update(SHADERSTAGE::CS, GPU_RESOURCE_HEAP_CBV_COUNT + GPU_RESOURCE_HEAP_SRV_COUNT + slot,
						resource->m_uav, m_device, GetCommandList());
				}
			}
			else
			{
				// TODO: implement array support
			}
		}
	}

	void GraphicsDevice_DX12::BindSampler(SHADERSTAGE stage, Sampler* sampler, int slot)
	{
		assert(slot < GPU_SAMPLER_HEAP_COUNT);

		if (sampler != nullptr && sampler->m_resource != nullptr)
		{
			GetFrameResources().SamplerDescriptorsGPU->Update(stage, slot,
				sampler->m_resource, m_device, GetCommandList());
		}
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

	void GraphicsDevice_DX12::Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
	{
		SetupForDispatch();

		GetCommandList()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::CreateBlob(UINT64 byteSize, CPUBuffer* buffer)
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
			GetCommandList()->CopyBufferRegion(buffer->m_resource.Get(), 0, BufferUploader->m_resource.Get(), BufferUploader->CalculateOffset(dest), desc.ByteWidth);
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

	void GraphicsDevice_DX12::CreateTexture2D(const TextureDesc& desc, const SubresourceData* initialData, Texture2D** texture2D)
	{
		if ((*texture2D) == nullptr)
		{
			(*texture2D) = new Texture2D();
		}
		(*texture2D)->m_desc = desc;

		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.CreationNodeMask = 0;
		heapDesc.VisibleNodeMask = 0;

		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;

		D3D12_RESOURCE_DESC resDesc;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Format = ConvertFormat(desc.Format);
		resDesc.Width = desc.Width;
		resDesc.Height = desc.Height;
		resDesc.MipLevels = (desc.MipLevels) > 0 ? desc.MipLevels : Utility::NumMipmapLevels(desc.Width, desc.Height);
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.DepthOrArraySize = (UINT16)desc.ArraySize;
		resDesc.Alignment = 0;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (desc.BindFlags & BIND_DEPTH_STENCIL)
		{
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		else
		{
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		}
		if (desc.BindFlags & BIND_RENDER_TARGET)
		{
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		if (desc.BindFlags & BIND_UNORDERED_ACCESS)
		{
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		if (!(desc.BindFlags & BIND_SHADER_RESOURCE) && !(desc.BindFlags & BIND_RESOURCE_NONE))
		{
			resDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}
		resDesc.SampleDesc.Count = desc.SampleDesc.Count;
		resDesc.SampleDesc.Quality = desc.SampleDesc.Quality;

		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Color[0] = 0;
		optimizedClearValue.Color[1] = 0;
		optimizedClearValue.Color[2] = 0;
		optimizedClearValue.Color[3] = 0;
		optimizedClearValue.DepthStencil.Depth = 0.0f;
		optimizedClearValue.DepthStencil.Stencil = 0;
		optimizedClearValue.Format = resDesc.Format;
		if (optimizedClearValue.Format == DXGI_FORMAT_R16_TYPELESS)
		{
			optimizedClearValue.Format = DXGI_FORMAT_D16_UNORM;
		}
		else if (optimizedClearValue.Format == DXGI_FORMAT_R32_TYPELESS)
		{
			optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		}
		else if (optimizedClearValue.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
		{
			optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		}
		bool useClearValue = desc.BindFlags & BIND_RENDER_TARGET || desc.BindFlags & BIND_DEPTH_STENCIL;

		HRESULT hr = GetDevice()->CreateCommittedResource(&heapDesc, heapFlags, &resDesc, resourceState,
			useClearValue ? &optimizedClearValue : nullptr,
			__uuidof(ID3D12Resource), (void**)&(*texture2D)->m_resource);

		assert(SUCCEEDED(hr));
		
		if ((*texture2D)->m_desc.MipLevels == 0)
		{
			(*texture2D)->m_desc.MipLevels = (UINT)log2(std::max((*texture2D)->m_desc.Width, (*texture2D)->m_desc.Height));
		}

		// Issue data copy on request
		if (initialData != nullptr)
		{
			D3D12_SUBRESOURCE_DATA* data = new D3D12_SUBRESOURCE_DATA[desc.ArraySize];
			for (UINT slice = 0; slice < desc.ArraySize; ++slice)
			{
				data[slice] = ConvertSubresourceData(initialData[slice]);
			}

			UINT numSubresources = desc.ArraySize;
			UINT firstSubresource = 0;

			UINT64 requiredSize = 0;
			GetDevice()->GetCopyableFootprints(&resDesc, 0, numSubresources, 0, nullptr, nullptr, nullptr, &requiredSize);
			uint8_t* dest = TextureUploader->Allocate(static_cast<size_t>(requiredSize), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
			
			UINT64 dataSize = UpdateSubresources(GetCommandList().Get(), (*texture2D)->m_resource.Get(),
				TextureUploader->m_resource.Get(), TextureUploader->CalculateOffset(dest), 0, numSubresources, data);
		}

		// Issue creation of additional descriptors for the resource
		if ((*texture2D)->m_desc.BindFlags & BIND_RENDER_TARGET)
		{
			UINT arraySize = (*texture2D)->m_desc.ArraySize;
			UINT sampleCount = (*texture2D)->m_desc.SampleDesc.Count;
			bool multisampled = sampleCount > 1;

			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			renderTargetViewDesc.Format = ConvertFormat((*texture2D)->m_desc.Format);
			renderTargetViewDesc.Texture2DArray.MipSlice = 0;

			if ((*texture2D)->m_desc.MiscFlags & RESOURCE_MISC_TEXTURECUBE)
			{
				// TextureCube, TextureCubeArray...
				UINT slices = arraySize / 6;

				renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				renderTargetViewDesc.Texture2DArray.MipSlice = 0;

				if ((*texture2D)->m_independentRTVCubemapFaces)
				{
					// independent faces
					for (UINT i = 0; i < arraySize; ++i)
					{
						renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
						renderTargetViewDesc.Texture2DArray.ArraySize = 1;

						(*texture2D)->m_additionalRTVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE());
						(*texture2D)->m_additionalRTVs.back()->ptr = RTAllocator->Allocate();
						GetDevice()->CreateRenderTargetView((*texture2D)->m_resource.Get(), &renderTargetViewDesc, *(*texture2D)->m_additionalRTVs[i]);
					}
				}
				else if ((*texture2D)->m_independentRTVArraySlices)
				{
					// independent slices
					for (UINT i = 0; i < slices; ++i)
					{
						renderTargetViewDesc.Texture2DArray.FirstArraySlice = i * 6;
						renderTargetViewDesc.Texture2DArray.ArraySize = 6;

						(*texture2D)->m_additionalRTVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE());
						(*texture2D)->m_additionalRTVs.back()->ptr = RTAllocator->Allocate();
						GetDevice()->CreateRenderTargetView((*texture2D)->m_resource.Get(), &renderTargetViewDesc, *(*texture2D)->m_additionalRTVs[i]);
					}
				}

				{
					// Create full-resource RTVs:
					renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
					renderTargetViewDesc.Texture2DArray.ArraySize = arraySize;

					(*texture2D)->m_rtv = new D3D12_CPU_DESCRIPTOR_HANDLE();
					(*texture2D)->m_rtv->ptr = RTAllocator->Allocate();
					GetDevice()->CreateRenderTargetView((*texture2D)->m_resource.Get(), &renderTargetViewDesc, *(*texture2D)->m_rtv);
				}
			}
			else
			{
				// Texture2D, Texture2DArray...
				if (arraySize > 1 && (*texture2D)->m_independentRTVArraySlices)
				{
					// Create subresource RTVs:
					for (UINT i = 0; i < arraySize; ++i)
					{
						if (multisampled)
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
							renderTargetViewDesc.Texture2DMSArray.FirstArraySlice = i;
							renderTargetViewDesc.Texture2DMSArray.ArraySize = 1;
						}
						else
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
							renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
							renderTargetViewDesc.Texture2DArray.ArraySize = 1;
							renderTargetViewDesc.Texture2DArray.MipSlice = 0;
						}

						(*texture2D)->m_additionalRTVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE());
						(*texture2D)->m_additionalRTVs.back()->ptr = RTAllocator->Allocate();
						GetDevice()->CreateRenderTargetView((*texture2D)->m_resource.Get(), &renderTargetViewDesc, *(*texture2D)->m_additionalRTVs[i]);
					}
				}

				{
					// Create full-resource RTV:
					if (arraySize > 1)
					{
						if (multisampled)
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
							renderTargetViewDesc.Texture2DMSArray.FirstArraySlice = 0;
							renderTargetViewDesc.Texture2DMSArray.ArraySize = arraySize;
						}
						else
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
							renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
							renderTargetViewDesc.Texture2DArray.ArraySize = arraySize;
							renderTargetViewDesc.Texture2DArray.MipSlice = 0;
						}
					}
					else
					{
						if (multisampled)
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						}
						else
						{
							renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
							renderTargetViewDesc.Texture2D.MipSlice = 0;
						}
					}

					(*texture2D)->m_rtv = new D3D12_CPU_DESCRIPTOR_HANDLE();
					(*texture2D)->m_rtv->ptr = RTAllocator->Allocate();
					GetDevice()->CreateRenderTargetView((*texture2D)->m_resource.Get(), &renderTargetViewDesc, *(*texture2D)->m_rtv);
				}
			}
		}

		if ((*texture2D)->m_desc.BindFlags & BIND_DEPTH_STENCIL)
		{
			UINT arraySize = (*texture2D)->m_desc.ArraySize;
			UINT sampleCount = (*texture2D)->m_desc.SampleDesc.Count;
			bool multisampled = sampleCount > 1;

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
			depthStencilViewDesc.Texture2DArray.MipSlice = 0;
			depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

			// Try to resolve depth stencil format:
			switch ((*texture2D)->m_desc.Format)
			{
			case FORMAT_R16_TYPELESS:
				depthStencilViewDesc.Format = DXGI_FORMAT_D16_UNORM;
				break;
			case FORMAT_R32_TYPELESS:
				depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
				break;
			case FORMAT_R24G8_TYPELESS:
				depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			default:
				depthStencilViewDesc.Format = ConvertFormat((*texture2D)->m_desc.Format);
				break;
			}

			if ((*texture2D)->m_desc.MiscFlags & RESOURCE_MISC_TEXTURECUBE)
			{
				// TextureCube, TextureCubeArray...
				UINT slices = arraySize / 6;

				depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				depthStencilViewDesc.Texture2DArray.MipSlice = 0;

				if ((*texture2D)->m_independentRTVCubemapFaces)
				{
					// independent faces
					for (UINT i = 0; i < arraySize; ++i)
					{
						depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
						depthStencilViewDesc.Texture2DArray.ArraySize = 1;

						(*texture2D)->m_additionalDSVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
						(*texture2D)->m_additionalDSVs.back()->ptr = DSAllocator->Allocate();
						GetDevice()->CreateDepthStencilView((*texture2D)->m_resource.Get(), &depthStencilViewDesc, *(*texture2D)->m_additionalDSVs[i]);
					}
				}
				else if ((*texture2D)->m_independentRTVArraySlices)
				{
					// independent slices
					for (UINT i = 0; i < slices; ++i)
					{
						depthStencilViewDesc.Texture2DArray.FirstArraySlice = i * 6;
						depthStencilViewDesc.Texture2DArray.ArraySize = 6;

						(*texture2D)->m_additionalDSVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
						(*texture2D)->m_additionalDSVs.back()->ptr = DSAllocator->Allocate();
						GetDevice()->CreateDepthStencilView((*texture2D)->m_resource.Get(), &depthStencilViewDesc, *(*texture2D)->m_additionalDSVs[i]);
					}
				}

				{
					// Create full-resource DSV:
					depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
					depthStencilViewDesc.Texture2DArray.ArraySize = arraySize;

					(*texture2D)->m_dsv = new D3D12_CPU_DESCRIPTOR_HANDLE;
					(*texture2D)->m_dsv->ptr = DSAllocator->Allocate();
					GetDevice()->CreateDepthStencilView((*texture2D)->m_resource.Get(), &depthStencilViewDesc, *(*texture2D)->m_dsv);
				}
			}
			else
			{
				// Texture2D, Texture2DArray...
				if (arraySize > 1 && (*texture2D)->m_independentRTVArraySlices)
				{
					// Create subresource DSVs:
					for (UINT i = 0; i < arraySize; ++i)
					{
						if (multisampled)
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
							depthStencilViewDesc.Texture2DMSArray.FirstArraySlice = i;
							depthStencilViewDesc.Texture2DMSArray.ArraySize = 1;
						}
						else
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
							depthStencilViewDesc.Texture2DArray.MipSlice = 0;
							depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
							depthStencilViewDesc.Texture2DArray.ArraySize = 1;
						}

						(*texture2D)->m_additionalDSVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
						(*texture2D)->m_additionalDSVs.back()->ptr = DSAllocator->Allocate();
						GetDevice()->CreateDepthStencilView((*texture2D)->m_resource.Get(), &depthStencilViewDesc, *(*texture2D)->m_additionalDSVs[i]);
					}
				}
				else
				{
					// Create full-resource DSV:
					if (arraySize > 1)
					{
						if (multisampled)
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
							depthStencilViewDesc.Texture2DMSArray.FirstArraySlice = 0;
							depthStencilViewDesc.Texture2DMSArray.ArraySize = arraySize;
						}
						else
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
							depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
							depthStencilViewDesc.Texture2DArray.ArraySize = arraySize;
							depthStencilViewDesc.Texture2DArray.MipSlice = 0;
						}
					}
					else
					{
						if (multisampled)
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
						}
						else
						{
							depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
							depthStencilViewDesc.Texture2D.MipSlice = 0;
						}
					}

					(*texture2D)->m_dsv = new D3D12_CPU_DESCRIPTOR_HANDLE;
					(*texture2D)->m_dsv->ptr = DSAllocator->Allocate();
					GetDevice()->CreateDepthStencilView((*texture2D)->m_resource.Get(), &depthStencilViewDesc, *(*texture2D)->m_dsv);
				}
			}
		}

		if ((*texture2D)->m_desc.BindFlags & BIND_SHADER_RESOURCE)
		{
			UINT arraySize = (*texture2D)->m_desc.ArraySize;
			UINT sampleCount = (*texture2D)->m_desc.SampleDesc.Count;
			bool multisampled = sampleCount > 1;

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
			shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			// Try to resolve shader resource format:
			switch ((*texture2D)->m_desc.Format)
			{
			case FORMAT_R16_TYPELESS:
				shaderResourceViewDesc.Format = DXGI_FORMAT_R16_UNORM;
				break;
			case FORMAT_R32_TYPELESS:
				shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case FORMAT_R24G8_TYPELESS:
				shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
			default:
				shaderResourceViewDesc.Format = ConvertFormat((*texture2D)->m_desc.Format);
				break;
			}

			if (arraySize > 1)
			{
				if ((*texture2D)->m_desc.MiscFlags & RESOURCE_MISC_TEXTURECUBE)
				{
					if (arraySize > 6)
					{
						shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						shaderResourceViewDesc.TextureCubeArray.First2DArrayFace = 0;
						shaderResourceViewDesc.TextureCubeArray.NumCubes = arraySize / 6;
						shaderResourceViewDesc.TextureCubeArray.MostDetailedMip = 0; //from most detailed...
						shaderResourceViewDesc.TextureCubeArray.MipLevels = -1; //...to least detailed
					}
					else
					{
						shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						shaderResourceViewDesc.TextureCube.MostDetailedMip = 0; //from most detailed...
						shaderResourceViewDesc.TextureCube.MipLevels = -1; //...to least detailed
					}
				}
				else
				{
					if (multisampled)
					{
						shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
						shaderResourceViewDesc.Texture2DMSArray.FirstArraySlice = 0;
						shaderResourceViewDesc.Texture2DMSArray.ArraySize = arraySize;
					}
					else
					{
						shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
						shaderResourceViewDesc.Texture2DArray.ArraySize = arraySize;
						shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0; //from most detailed...
						shaderResourceViewDesc.Texture2DArray.MipLevels = -1; //...to least detailed
					}
				}
				(*texture2D)->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
				(*texture2D)->m_srv->ptr = ResourceAllocator->Allocate();
				GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_srv);

				if ((*texture2D)->m_independentSRVArraySlices)
				{
					if ((*texture2D)->m_desc.MiscFlags & RESOURCE_MISC_TEXTURECUBE)
					{
						UINT slices = arraySize / 6;

						// independent cubemaps
						for (UINT i = 0; i < slices; ++i)
						{
							shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
							shaderResourceViewDesc.TextureCubeArray.First2DArrayFace = i * 6;
							shaderResourceViewDesc.TextureCubeArray.NumCubes = 1;
							shaderResourceViewDesc.TextureCubeArray.MostDetailedMip = 0; //from most detailed...
							shaderResourceViewDesc.TextureCubeArray.MipLevels = -1; //...to least detailed

							(*texture2D)->m_additionalSRVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
							(*texture2D)->m_additionalSRVs.back()->ptr = ResourceAllocator->Allocate();
							GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_additionalSRVs[i]);
						}
					}
					else
					{
						UINT slices = arraySize;

						// independent slices
						for (UINT i = 0; i < slices; ++i)
						{
							if (multisampled)
							{
								shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
								shaderResourceViewDesc.Texture2DMSArray.FirstArraySlice = i;
								shaderResourceViewDesc.Texture2DMSArray.ArraySize = 1;
							}
							else
							{
								shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
								shaderResourceViewDesc.Texture2DArray.FirstArraySlice = i;
								shaderResourceViewDesc.Texture2DArray.ArraySize = 1;
								shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0; //from most detailed...
								shaderResourceViewDesc.Texture2DArray.MipLevels = -1; //...to least detailed
							}

							(*texture2D)->m_additionalSRVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
							(*texture2D)->m_additionalSRVs.back()->ptr = ResourceAllocator->Allocate();
							GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_additionalSRVs[i]);
						}
					}
				}
			}
			else
			{
				if (multisampled)
				{
					shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					(*texture2D)->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
					(*texture2D)->m_srv->ptr = ResourceAllocator->Allocate();
					GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_srv);
				}
				else
				{
					shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					if ((*texture2D)->m_independentSRVMIPs)
					{
						// Create subresource SRVs:
						UINT miplevels = (*texture2D)->m_desc.MipLevels;
						for (UINT i = 0; i < miplevels; ++i)
						{
							shaderResourceViewDesc.Texture2D.MostDetailedMip = i;
							shaderResourceViewDesc.Texture2D.MipLevels = 1;

							(*texture2D)->m_additionalSRVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
							(*texture2D)->m_additionalSRVs.back()->ptr = ResourceAllocator->Allocate();
							GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_additionalSRVs[i]);
						}
					}

					{
						// Create full-resource SRV:
						shaderResourceViewDesc.Texture2D.MostDetailedMip = 0; //from most detailed...
						shaderResourceViewDesc.Texture2D.MipLevels = -1; //...to least detailed
						(*texture2D)->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
						(*texture2D)->m_srv->ptr = ResourceAllocator->Allocate();
						GetDevice()->CreateShaderResourceView((*texture2D)->m_resource.Get(), &shaderResourceViewDesc, *(*texture2D)->m_srv);
					}
				}
			}
		}

		if ((*texture2D)->m_desc.BindFlags & BIND_UNORDERED_ACCESS)
		{
			assert((*texture2D)->m_independentRTVArraySlices == false && "TextureArray UAV not implemented!");

			UINT arraySize = (*texture2D)->m_desc.ArraySize;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
			if (arraySize > 1)
			{
				uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uav_desc.Texture2DArray.FirstArraySlice = 0;
				uav_desc.Texture2DArray.ArraySize = arraySize;
				uav_desc.Texture2DArray.MipSlice = 0; // TODO: expose to desc
			}
			else
			{
				uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uav_desc.Texture2D.MipSlice = 0; // TODO: expose to desc
			}

			if ((*texture2D)->m_independentUAVMIPs)
			{
				// Create subresource UAVs:
				UINT miplevels = (*texture2D)->m_desc.MipLevels;
				for (UINT i = 0; i < miplevels; ++i)
				{
					uav_desc.Texture2D.MipSlice = i;

					(*texture2D)->m_additionalUAVs.push_back(new D3D12_CPU_DESCRIPTOR_HANDLE);
					(*texture2D)->m_additionalUAVs.back()->ptr = ResourceAllocator->Allocate();
					GetDevice()->CreateUnorderedAccessView((*texture2D)->m_resource.Get(), nullptr, &uav_desc, *(*texture2D)->m_additionalUAVs[i]);
				}
			}

			{
				// Create main resource UAV:
				uav_desc.Texture2D.MipSlice = 0;
				(*texture2D)->m_uav = new D3D12_CPU_DESCRIPTOR_HANDLE;
				(*texture2D)->m_uav->ptr = ResourceAllocator->Allocate();
				GetDevice()->CreateUnorderedAccessView((*texture2D)->m_resource.Get(), nullptr, &uav_desc, *(*texture2D)->m_uav);
			}
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
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "vs_main", "vs_5_1");
			}
			break;
			case SHADERSTAGE::PS:
			{
				D3D_SHADER_MACRO _macros[2] = { "PIXEL_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "ps_main", "ps_5_1");
			}
			break;
			case SHADERSTAGE::CS:
			{
				D3D_SHADER_MACRO _macros[2] = { "COMPUTE_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "cs_main", "cs_5_1");
			}
			break;
			case SHADERSTAGE::GS:
			{
				D3D_SHADER_MACRO _macros[2] = { "GEOMETRY_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "gs_main", "gs_5_1");
			}
			break;
			case SHADERSTAGE::HS:
			{
				D3D_SHADER_MACRO _macros[2] = { "HULL_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "hs_main", "hs_5_1");
			}
			break;
			case SHADERSTAGE::DS:
			{
				D3D_SHADER_MACRO _macros[2] = { "DOMAIN_SHADER", "1" };
				shader->m_blob = D3DUtils::CompileShader(filename, _macros, "ds_main", "ds_5_1");
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
				elements[i].AlignedByteOffset = psoDesc->IL->m_desc[i].AlignedByteOffset;
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

	void GraphicsDevice_DX12::CreateSamplerState(const SamplerDesc *pSamplerDesc, Sampler *pSamplerState)
	{
		D3D12_SAMPLER_DESC desc;
		desc.Filter = ConvertFilter(pSamplerDesc->Filter);
		desc.AddressU = ConvertTextureAddressMode(pSamplerDesc->AddressU);
		desc.AddressV = ConvertTextureAddressMode(pSamplerDesc->AddressV);
		desc.AddressW = ConvertTextureAddressMode(pSamplerDesc->AddressW);
		desc.MipLODBias = pSamplerDesc->MipLODBias;
		desc.MaxAnisotropy = pSamplerDesc->MaxAnisotropy;
		desc.ComparisonFunc = ConvertComparisonFunc(pSamplerDesc->ComparisonFunc);
		desc.BorderColor[0] = pSamplerDesc->BorderColor[0];
		desc.BorderColor[1] = pSamplerDesc->BorderColor[1];
		desc.BorderColor[2] = pSamplerDesc->BorderColor[2];
		desc.BorderColor[3] = pSamplerDesc->BorderColor[3];
		desc.MinLOD = pSamplerDesc->MinLOD;
		desc.MaxLOD = pSamplerDesc->MaxLOD;

		pSamplerState->m_desc = *pSamplerDesc;

		pSamplerState->m_resource = new D3D12_CPU_DESCRIPTOR_HANDLE;
		pSamplerState->m_resource->ptr = SamplerAllocator->Allocate();
		m_device->CreateSampler(&desc, *pSamplerState->m_resource);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::TransitionBarrier(GPUResource* resource, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter, UINT subresource)
	{
		if (resource != nullptr)
		{
			GetCommandList()->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					resource->m_resource.Get(),
					ConvertResourceStates(stateBefore),
					ConvertResourceStates(stateAfter),
					subresource));
		}
	}

	void GraphicsDevice_DX12::TransitionBarriers(GPUResource* const* resources, UINT* subresources, UINT numBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{
		if (resources != nullptr)
		{
			assert(numBarriers <= 8);
			D3D12_RESOURCE_BARRIER barriers[8];
			for (UINT i = 0; i < numBarriers; ++i)
			{
				barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[i].Transition.pResource = resources[i]->m_resource.Get();
				barriers[i].Transition.StateBefore = ConvertResourceStates(stateBefore);
				barriers[i].Transition.StateAfter = ConvertResourceStates(stateAfter);
				barriers[i].Transition.Subresource = subresources == nullptr ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : subresources[i];
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

	void* GraphicsDevice_DX12::AllocateFromRingBuffer(GPURingBuffer* buffer, UINT dataSize, UINT& offsetIntoBuffer)
	{
		assert(buffer->m_desc.Usage == USAGE_DYNAMIC && (buffer->m_desc.CpuAccessFlags & CPU_ACCESS_WRITE) && "Ring buffer must be writeable by the CPU");
		assert(buffer->m_desc.ByteWidth > dataSize && "Data of the required size cannot fit.");

		if (dataSize == 0)
			return nullptr;

		dataSize = MathHelper::Min(buffer->m_desc.ByteWidth, dataSize);

		UINT position = (UINT)buffer->GetByteOffset();
		bool wrap = position + dataSize > buffer->m_desc.ByteWidth || buffer->GetResidentFrame() != st_frameCount;
		position = wrap ? 0 : position;

		size_t alignment = buffer->m_desc.BindFlags & BIND_CONSTANT_BUFFER ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

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

		uint8_t* dest = GetFrameResources().ResourceBuffer->Allocate(dataSize, alignment);
		GetCommandList()->CopyBufferRegion(
			buffer->m_resource.Get(), (UINT64)position,
			GetFrameResources().ResourceBuffer->m_resource.Get(), GetFrameResources().ResourceBuffer->CalculateOffset(dest),
			dataSize
		);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		GetCommandList()->ResourceBarrier(1, &barrier);

		// Thread safety is compromised!
		buffer->m_byteOffset = position + dataSize;
		buffer->m_residentFrame = st_frameCount;

		offsetIntoBuffer = (UINT)position;
		return reinterpret_cast<void*>(dest);
	}

	void GraphicsDevice_DX12::InvalidateBufferAccess(GPUBuffer* buffer)
	{

	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::CreateTextureFromFile(const std::string& fileName, Texture2D **ppTexture, bool mipMaps)
	{
		(*ppTexture) = new Texture2D();

		std::unique_ptr<uint8_t[]> imageData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		std::shared_ptr<Image> image;
		bool isCubeMap = false;

		if (!fileName.substr(fileName.length() - 4).compare(std::string(".dds")))
		{
			// Load dds
			ThrowIfFailed( LoadDDSTextureFromFile(m_device.Get(), std::wstring(fileName.begin(), fileName.end()).c_str(), &(*ppTexture)->m_resource, imageData, subresources, 0, nullptr, &isCubeMap) );
		}
		else if (!fileName.substr(fileName.length() - 4).compare(std::string(".hdr")))
		{
			// Load hdr
			subresources.push_back({});
			image = Image::FromFile(fileName);

			TextureDesc desc;
			desc.Width = image->Width();
			desc.Height = image->Height();
			desc.Format = FORMAT_R32G32B32A32_FLOAT;
			desc.BindFlags = BIND_RESOURCE_NONE;

			CreateTexture2D(desc, nullptr, ppTexture);

			subresources[0].pData = image->Pixels<void>();
			subresources[0].RowPitch = image->Pitch();
			subresources[0].SlicePitch = image->DataSize();
		}
		else
		{
			// Load png, jpeg, ...
			subresources.push_back({});
			ThrowIfFailed( LoadWICTextureFromFile(m_device.Get(), std::wstring(fileName.begin(), fileName.end()).c_str(), &(*ppTexture)->m_resource, imageData, subresources[0], 0Ui64, mipMaps) );
		}

		{
			D3D12_RESOURCE_DESC desc = (*ppTexture)->m_resource->GetDesc();
			(*ppTexture)->m_desc = ConvertTextureDesc_Inv(desc);

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = desc.Format;
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (isCubeMap)
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv_desc.TextureCube.MipLevels = -1;
				srv_desc.TextureCube.MostDetailedMip = 0;
				srv_desc.TextureCube.ResourceMinLODClamp = -1;
			}
			else
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = -1;
				srv_desc.Texture2D.MostDetailedMip = 0;
				srv_desc.Texture2D.PlaneSlice = 0;
				srv_desc.Texture2D.ResourceMinLODClamp = -1;
			}
			(*ppTexture)->m_srv = new D3D12_CPU_DESCRIPTOR_HANDLE;
			(*ppTexture)->m_srv->ptr = ResourceAllocator->Allocate();
			m_device->CreateShaderResourceView((*ppTexture)->m_resource.Get(), &srv_desc, *(*ppTexture)->m_srv);

			UINT64 RequiredSize = 0;
			m_device->GetCopyableFootprints(&desc, 0, (UINT)subresources.size(), 0, nullptr, nullptr, nullptr, &RequiredSize);
			uint8_t* dest = TextureUploader->Allocate(static_cast<size_t>(RequiredSize), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

			UINT64 dataSize = UpdateSubresources(GetCommandList().Get(), (*ppTexture)->m_resource.Get(),
				TextureUploader->m_resource.Get(), TextureUploader->CalculateOffset(dest), 0, (UINT)subresources.size(), subresources.data());

			TransitionBarrier(*ppTexture, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_COMMON);

			if (mipMaps)
				GenerateMipmaps(*ppTexture);
		}
	}

	void GraphicsDevice_DX12::GenerateMipmaps(Texture* texture)
	{
		Graphics::Texture& tex = *texture;

		assert(tex.m_desc.Width == tex.m_desc.Height);
		assert(Utility::IsPowerOfTwo(tex.m_desc.Width));

		tex.m_desc.MipLevels = Utility::NumMipmapLevels(tex.m_desc.Width, tex.m_desc.Height);
		tex.m_desc.BindFlags |= BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

		Graphics::ComputePSO* pso = nullptr;

		TextureDesc desc = tex.m_desc;
		if (desc.ArraySize == 1 && desc.Format == FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			pso = m_gammaDownsamplePSO;
		}
		else if (desc.ArraySize > 1 && desc.Format != FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			pso = m_arrayDownsamplePSO;
		}
		else
		{
			assert(desc.ArraySize == 1);
			pso = m_gammaDownsamplePSO;
		}

		Texture linearTexture = tex;
		tex.m_resource->AddRef();
		if (desc.Format == FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			TextureDesc desc = tex.m_desc;
			desc.Format = FORMAT_R8G8B8A8_UNORM;
			CreateTexture(desc.Width, desc.Height, 1, DXGI_FORMAT_R8G8B8A8_UNORM, desc.MipLevels);

			GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(linearTexture.m_resource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
			GetCommandList()->CopyResource(linearTexture.m_resource.Get(), tex.m_resource.Get());
			GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(linearTexture.m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
		}

		BindComputePSO(pso);

		std::vector<CD3DX12_RESOURCE_BARRIER> preDispatchBarriers{ desc.ArraySize };
		std::vector<CD3DX12_RESOURCE_BARRIER> postDispatchBarriers{ desc.ArraySize };
		for (UINT level = 1, levelWidth = desc.Width / 2, levelHeight = desc.Height / 2; level < desc.MipLevels; ++level, levelWidth /= 2, levelHeight /= 2)
		{
			CreateTextureSRV(&linearTexture, desc.ArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D, level - 1, 1);
			CreateTextureUAV(&linearTexture, level);

			for (UINT arraySlice = 0; arraySlice < desc.ArraySize; ++arraySlice) {
				const UINT subresourceIndex = D3D12CalcSubresource(level, arraySlice, 0, desc.MipLevels, desc.ArraySize);
				preDispatchBarriers[arraySlice] = CD3DX12_RESOURCE_BARRIER::Transition(linearTexture.m_resource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, subresourceIndex);
				postDispatchBarriers[arraySlice] = CD3DX12_RESOURCE_BARRIER::Transition(linearTexture.m_resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, subresourceIndex);
			}

			GetCommandList()->ResourceBarrier(desc.ArraySize, preDispatchBarriers.data());
			BindResource(SHADERSTAGE::CS, &linearTexture, 0);
			BindUnorderedAccessResource(&linearTexture, 0);
			Dispatch(std::max(1u, levelWidth / 8), std::max(1u, levelHeight / 8), desc.ArraySize);
			GetCommandList()->ResourceBarrier(desc.ArraySize, postDispatchBarriers.data());
		}

		if (tex.m_resource == linearTexture.m_resource)
		{
			TransitionBarrier(&linearTexture, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, RESOURCE_STATE_COMMON);
		}
		else
		{
			GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex.m_resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
			GetCommandList()->CopyResource(tex.m_resource.Get(), linearTexture.m_resource.Get());
			GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex.m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::BeginProfilerBlock(const char* name)
	{
		::PIXBeginEvent(GetCommandList().Get(), 0, name);
	}

	void GraphicsDevice_DX12::EndProfilerBlock()
	{
		::PIXEndEvent(GetCommandList().Get());
	}

	void GraphicsDevice_DX12::SetMarker(const char* name)
	{
		::PIXSetMarker(GetCommandList().Get(), 0, name);
	}
}