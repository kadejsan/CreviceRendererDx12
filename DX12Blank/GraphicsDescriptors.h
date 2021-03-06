#pragma once

namespace Graphics
{
	class VertexShader;
	class PixelShader;
	class HullShader;
	class DomainShader;
	class GeometryShader;
	class ComputeShader;
	class RayGenerationShader;
	class RayMissShader;
	class RayClosestHitShader;
	class RayAnyHitShader;
	class RayIntersectionShader;
	class BlendState;
	class RasterizerState;
	class DepthStencilState;
	class VertexLayout;
	class HitGroup;

	enum SHADERSTAGE
	{
		VS,	 // VertexShader
		HS,	 // HullShader
		DS,	 // DomainShader
		GS,  // GeometryShader
		PS,	 // PixelShader
		CS,	 // ComputeShader
		RGS, // RayGenerationShader
		MS,  // MissShader
		AHS, // AnyHitShader
		CHS, // ClosestHitShader
		IS,  // IntersectionShader
		SHADERSTAGE_MAX,
	};

	enum PRIMITIVETOPOLOGY
	{
		UNDEFINED_TOPOLOGY,
		TRIANGLELIST,
		TRIANGLESTRIP,
		POINTLIST,
		LINELIST,
		PATCHLIST,
	};

	enum COMPARISON_FUNC
	{
		COMPARISON_NEVER,
		COMPARISON_LESS,
		COMPARISON_EQUAL,
		COMPARISON_LESS_EQUAL,
		COMPARISON_GREATER,
		COMPARISON_NOT_EQUAL,
		COMPARISON_GREATER_EQUAL,
		COMPARISON_ALWAYS,
	};

	enum DEPTH_WRITE_MASK
	{
		DEPTH_WRITE_MASK_ZERO,
		DEPTH_WRITE_MASK_ALL,
	};

	enum STENCIL_OP
	{
		STENCIL_OP_KEEP,
		STENCIL_OP_ZERO,
		STENCIL_OP_REPLACE,
		STENCIL_OP_INCR_SAT,
		STENCIL_OP_DECR_SAT,
		STENCIL_OP_INVERT,
		STENCIL_OP_INCR,
		STENCIL_OP_DECR,
	};

	enum BLEND
	{
		BLEND_ZERO,
		BLEND_ONE,
		BLEND_SRC_COLOR,
		BLEND_INV_SRC_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_INV_SRC_ALPHA,
		BLEND_DEST_ALPHA,
		BLEND_INV_DEST_ALPHA,
		BLEND_DEST_COLOR,
		BLEND_INV_DEST_COLOR,
		BLEND_SRC_ALPHA_SAT,
		BLEND_BLEND_FACTOR,
		BLEND_INV_BLEND_FACTOR,
		BLEND_SRC1_COLOR,
		BLEND_INV_SRC1_COLOR,
		BLEND_SRC1_ALPHA,
		BLEND_INV_SRC1_ALPHA,
	};

	enum BLEND_OP
	{
		BLEND_OP_ADD,
		BLEND_OP_SUBTRACT,
		BLEND_OP_REV_SUBTRACT,
		BLEND_OP_MIN,
		BLEND_OP_MAX,
	};

	enum COLOR_WRITE_ENABLE
	{
		COLOR_WRITE_DISABLE = 0,
		COLOR_WRITE_ENABLE_RED = 1,
		COLOR_WRITE_ENABLE_GREEN = 2,
		COLOR_WRITE_ENABLE_BLUE = 4,
		COLOR_WRITE_ENABLE_ALPHA = 8,
		COLOR_WRITE_ENABLE_ALL = (((COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_GREEN) | COLOR_WRITE_ENABLE_BLUE) | COLOR_WRITE_ENABLE_ALPHA)
	};

	enum FILL_MODE
	{
		FILL_WIREFRAME,
		FILL_SOLID,
	};
	enum CULL_MODE
	{
		CULL_NONE,
		CULL_FRONT,
		CULL_BACK,
	};

	enum USAGE
	{
		USAGE_DEFAULT,
		USAGE_IMMUTABLE,
		USAGE_DYNAMIC,
		USAGE_STAGING,
		USAGE_ACCELERATION_STRUCTURE
	};

	enum INPUT_CLASSIFICATION
	{
		INPUT_PER_VERTEX_DATA,
		INPUT_PER_INSTANCE_DATA,
	};

	enum TEXTURE_ADDRESS_MODE
	{
		TEXTURE_ADDRESS_WRAP,
		TEXTURE_ADDRESS_MIRROR,
		TEXTURE_ADDRESS_CLAMP,
		TEXTURE_ADDRESS_BORDER,
		TEXTURE_ADDRESS_MIRROR_ONCE,
	};

	enum FILTER
	{
		FILTER_MIN_MAG_MIP_POINT,
		FILTER_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_MAG_MIP_LINEAR,
		FILTER_ANISOTROPIC,
		FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		FILTER_COMPARISON_ANISOTROPIC,
		FILTER_MINIMUM_MIN_MAG_MIP_POINT,
		FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
		FILTER_MINIMUM_ANISOTROPIC,
		FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
		FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
		FILTER_MAXIMUM_ANISOTROPIC,
	};

	enum FORMAT
	{
		FORMAT_UNKNOWN,
		FORMAT_R32G32B32A32_TYPELESS,
		FORMAT_R32G32B32A32_FLOAT,
		FORMAT_R32G32B32A32_UINT,
		FORMAT_R32G32B32A32_SINT,
		FORMAT_R32G32B32_TYPELESS,
		FORMAT_R32G32B32_FLOAT,
		FORMAT_R32G32B32_UINT,
		FORMAT_R32G32B32_SINT,
		FORMAT_R16G16B16A16_TYPELESS,
		FORMAT_R16G16B16A16_FLOAT,
		FORMAT_R16G16B16A16_UNORM,
		FORMAT_R16G16B16A16_UINT,
		FORMAT_R16G16B16A16_SNORM,
		FORMAT_R16G16B16A16_SINT,
		FORMAT_R32G32_TYPELESS,
		FORMAT_R32G32_FLOAT,
		FORMAT_R32G32_UINT,
		FORMAT_R32G32_SINT,
		FORMAT_R32G8X24_TYPELESS,
		FORMAT_D32_FLOAT_S8X24_UINT,
		FORMAT_R32_FLOAT_X8X24_TYPELESS,
		FORMAT_X32_TYPELESS_G8X24_UINT,
		FORMAT_R10G10B10A2_TYPELESS,
		FORMAT_R10G10B10A2_UNORM,
		FORMAT_R10G10B10A2_UINT,
		FORMAT_R11G11B10_FLOAT,
		FORMAT_R8G8B8A8_TYPELESS,
		FORMAT_R8G8B8A8_UNORM,
		FORMAT_R8G8B8A8_UNORM_SRGB,
		FORMAT_R8G8B8A8_UINT,
		FORMAT_R8G8B8A8_SNORM,
		FORMAT_R8G8B8A8_SINT,
		FORMAT_R16G16_TYPELESS,
		FORMAT_R16G16_FLOAT,
		FORMAT_R16G16_UNORM,
		FORMAT_R16G16_UINT,
		FORMAT_R16G16_SNORM,
		FORMAT_R16G16_SINT,
		FORMAT_R32_TYPELESS,
		FORMAT_D32_FLOAT,
		FORMAT_R32_FLOAT,
		FORMAT_R32_UINT,
		FORMAT_R32_SINT,
		FORMAT_R24G8_TYPELESS,
		FORMAT_D24_UNORM_S8_UINT,
		FORMAT_R24_UNORM_X8_TYPELESS,
		FORMAT_X24_TYPELESS_G8_UINT,
		FORMAT_R8G8_TYPELESS,
		FORMAT_R8G8_UNORM,
		FORMAT_R8G8_UINT,
		FORMAT_R8G8_SNORM,
		FORMAT_R8G8_SINT,
		FORMAT_R16_TYPELESS,
		FORMAT_R16_FLOAT,
		FORMAT_D16_UNORM,
		FORMAT_R16_UNORM,
		FORMAT_R16_UINT,
		FORMAT_R16_SNORM,
		FORMAT_R16_SINT,
		FORMAT_R8_TYPELESS,
		FORMAT_R8_UNORM,
		FORMAT_R8_UINT,
		FORMAT_R8_SNORM,
		FORMAT_R8_SINT,
		FORMAT_A8_UNORM,
		FORMAT_R1_UNORM,
		FORMAT_R9G9B9E5_SHAREDEXP,
		FORMAT_R8G8_B8G8_UNORM,
		FORMAT_G8R8_G8B8_UNORM,
		FORMAT_BC1_TYPELESS,
		FORMAT_BC1_UNORM,
		FORMAT_BC1_UNORM_SRGB,
		FORMAT_BC2_TYPELESS,
		FORMAT_BC2_UNORM,
		FORMAT_BC2_UNORM_SRGB,
		FORMAT_BC3_TYPELESS,
		FORMAT_BC3_UNORM,
		FORMAT_BC3_UNORM_SRGB,
		FORMAT_BC4_TYPELESS,
		FORMAT_BC4_UNORM,
		FORMAT_BC4_SNORM,
		FORMAT_BC5_TYPELESS,
		FORMAT_BC5_UNORM,
		FORMAT_BC5_SNORM,
		FORMAT_B5G6R5_UNORM,
		FORMAT_B5G5R5A1_UNORM,
		FORMAT_B8G8R8A8_UNORM,
		FORMAT_B8G8R8X8_UNORM,
		FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
		FORMAT_B8G8R8A8_TYPELESS,
		FORMAT_B8G8R8A8_UNORM_SRGB,
		FORMAT_B8G8R8X8_TYPELESS,
		FORMAT_B8G8R8X8_UNORM_SRGB,
		FORMAT_BC6H_TYPELESS,
		FORMAT_BC6H_UF16,
		FORMAT_BC6H_SF16,
		FORMAT_BC7_TYPELESS,
		FORMAT_BC7_UNORM,
		FORMAT_BC7_UNORM_SRGB,
		FORMAT_AYUV,
		FORMAT_Y410,
		FORMAT_Y416,
		FORMAT_NV12,
		FORMAT_P010,
		FORMAT_P016,
		FORMAT_420_OPAQUE,
		FORMAT_YUY2,
		FORMAT_Y210,
		FORMAT_Y216,
		FORMAT_NV11,
		FORMAT_AI44,
		FORMAT_IA44,
		FORMAT_P8,
		FORMAT_A8P8,
		FORMAT_B4G4R4A4_UNORM,
		FORMAT_FORCE_UINT = 0xffffffff,
	};

	enum INDEXBUFFER_FORMAT
	{
		INDEXFORMAT_16BIT,
		INDEXFORMAT_32BIT,
	};

	enum CLEAR_FLAG
	{
		CLEAR_DEPTH = 0x1L,
		CLEAR_STENCIL = 0x2L,
	};

	enum BIND_FLAG
	{
		BIND_VERTEX_BUFFER = 0x1L,
		BIND_INDEX_BUFFER = 0x2L,
		BIND_CONSTANT_BUFFER = 0x4L,
		BIND_SHADER_RESOURCE = 0x8L,
		BIND_STREAM_OUTPUT = 0x10L,
		BIND_RENDER_TARGET = 0x20L,
		BIND_DEPTH_STENCIL = 0x40L,
		BIND_UNORDERED_ACCESS = 0x80L,
		BIND_RESOURCE_NONE = 0x100L,
	};

	enum CPU_ACCESS
	{
		CPU_ACCESS_WRITE = 0x10000L,
		CPU_ACCESS_READ = 0x20000L,
	};

	enum RESOURCE_MISC_FLAG
	{
		RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		RESOURCE_MISC_SHARED = 0x2L,
		RESOURCE_MISC_TEXTURECUBE = 0x4L,
		RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
		RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
		RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
		RESOURCE_MISC_ACCELERATION_STRUCTURE = 0x80L,
		RESOURCE_MISC_TILED = 0x40000L,
	};

	enum RESOURCE_STATES
	{
		RESOURCE_STATE_COMMON = 0,
		RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		RESOURCE_STATE_INDEX_BUFFER = 0x2,
		RESOURCE_STATE_RENDER_TARGET = 0x4,
		RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		RESOURCE_STATE_DEPTH_WRITE = 0x10,
		RESOURCE_STATE_DEPTH_READ = 0x20,
		RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		RESOURCE_STATE_STREAM_OUT = 0x100,
		RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		RESOURCE_STATE_COPY_DEST = 0x400,
		RESOURCE_STATE_COPY_SOURCE = 0x800,
		RESOURCE_STATE_RESOLVE_DEST = 0x1000,
		RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
		RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
		RESOURCE_STATE_SHADING_RATE_SOURCE = 0x1000000,
		RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		RESOURCE_STATE_PRESENT = 0,
		RESOURCE_STATE_PREDICATION = 0x200,
		RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
		RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
		RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
		RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000
	};

	enum ACCELERATION_STRUCTURE_BUILD_FLAGS
	{
		AS_BUILD_FLAG_EMPTY = 0,
		AS_BUILD_FLAG_ALLOW_UPDATE = 1 << 0,
		AS_BUILD_FLAG_ALLOW_COMPACTION = 1 << 1,
		AS_BUILD_FLAG_PREFER_FAST_TRACE = 1 << 2,
		AS_BUILD_FLAG_PREFER_FAST_BUILD = 1 << 3,
		AS_BUILD_FLAG_MINIMIZE_MEMORY = 1 << 4
	};

	enum ACCELERATION_STRUCTURE_TYPE
	{
		AS_TYPE_BOTTOMLEVEL,
		AS_TYPE_TOPLEVEL
	};

	enum ACCELERATION_STRUCTURE_BOTTOM_LEVEL_GEOMETRY_FLAGS
	{
		AS_BOTTOM_LEVEL_GEOMETRY_FLAG_EMPTY = 0,
		AS_BOTTOM_LEVEL_GEOMETRY_FLAG_OPAQUE = 1 << 0,
		AS_BOTTOM_LEVEL_GEOMETRY_FLAG_DUPLICATE_ANYHIT_INVOCATION = 1 << 1,
		AS_BOTTOM_LEVEL_GEOMETRY_FLAG_USE_TRANSFORM = 1 << 2
	};

	enum ACCELERATION_STRUCTURE_BOTTOM_LEVEL_GEOMETRY_TYPE
	{
		AS_BOTTOM_LEVEL_GEOMETRY_TYPE_TRIANGLES,
		AS_BOTTOM_LEVEL_GEOMETRY_TYPE_PROCEDURAL_AABB
	};

	enum RAYTRACING_INSTANCE_FLAGS
	{
		RAYTRACING_INSTANCE_FLAG_NONE = 0,
		RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 1 << 0,
		RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 1 << 1,
		RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE = 1 << 2,
		RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE = 1 << 3
	};

	enum RAYTRACING_PASS
	{
		RT_PASS_GBUFFER,
		RT_PASS_AMBIENT_OCCLUSION,
		RT_PASS_MAX
	};

	enum VARIABLE_SHADING_RATE
	{
		VRS_1x1 = 0,
		VRS_1x2 = 0x1,
		VRS_2x1 = 0x4,
		VRS_2x2 = 0x5,
		VRS_2x4 = 0x6,
		VRS_4x2 = 0x9,
		VRS_4x4 = 0xa
	};
	
	enum VARIABLE_SHADING_RATE_COMBINER
	{
		VRS_COMBINER_OVERRIDE,
		VRS_COMBINER_MIN,
		VRS_COMBINER_MAX,
		VRS_COMBINER_SUM,
		VRS_COMBINER_PASSTHROUGH
	};

#define	APPEND_ALIGNED_ELEMENT			( 0xffffffff )
#define FLOAT32_MAX						( 3.402823466e+38f )
#define DEFAULT_STENCIL_READ_MASK		( 0xff )
#define SO_NO_RASTERIZED_STREAM			( 0xffffffff )
#define RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES 32

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct ViewPort
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;

		ViewPort()
			: TopLeftX(0.0f)
			, TopLeftY(0.0f)
			, Width(0.0f)
			, Height(0.0f)
			, MinDepth(0.0f)
			, MaxDepth(0.0f)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct VertexInputLayoutDesc
	{
		const char*				SemanticName;
		UINT					SemanticIndex;
		FORMAT					Format;
		UINT					InputSlot;
		UINT					AlignedByteOffset;
		INPUT_CLASSIFICATION	InputSlotClass;
		UINT					InstanceDataStepRate;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct GPUBufferDesc
	{
		UINT	ByteWidth;
		USAGE	Usage;
		UINT	BindFlags;
		UINT	CpuAccessFlags;
		UINT	MiscFlags;
		UINT	StructureByteStride;
		FORMAT	Format;

		GPUBufferDesc()
			: ByteWidth(0)
			, Usage(USAGE_DEFAULT)
			, BindFlags(0)
			, CpuAccessFlags(0)
			, MiscFlags(0)
			, StructureByteStride(0)
			, Format(FORMAT_UNKNOWN)
		{}
	};

	struct SubresourceData
	{
		const void* SysMem;
		UINT		SysMemPitch;
		UINT		SysMemSlicePitch;

		SubresourceData() :
			SysMem(nullptr),
			SysMemPitch(0),
			SysMemSlicePitch(0)
		{}
	};

	struct Rect
	{
		LONG left;
		LONG top;
		LONG right;
		LONG bottom;

		Rect() :
			left(0),
			top(0),
			right(0),
			bottom(0)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct SampleDesc
	{
		UINT Count;
		UINT Quality;

		SampleDesc() 
			: Count(1)
			, Quality(0) 
		{}
	};

	struct TextureDesc
	{
		UINT		Width;
		UINT		Height;
		UINT		Depth;
		UINT		ArraySize;
		UINT		MipLevels;
		FORMAT		Format;
		SampleDesc	SampleDesc;
		USAGE		Usage;
		UINT		BindFlags;
		UINT		CPUAccessFlags;
		UINT		MiscFlags;
		FLOAT		ClearDepth;
		UINT8		ClearStencil;

		TextureDesc()
			: Width(0)
			, Height(0)
			, Depth(0)
			, ArraySize(1)
			, MipLevels(1)
			, Format(FORMAT_UNKNOWN)
			, Usage(USAGE_DEFAULT)
			, BindFlags(0)
			, CPUAccessFlags(0)
			, MiscFlags(0)
			, ClearDepth(1.0f)
			, ClearStencil(0)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct RenderTargetBlendStateDesc
	{
		bool BlendEnable;
		BLEND SrcBlend;
		BLEND DestBlend;
		BLEND_OP BlendOp;
		BLEND SrcBlendAlpha;
		BLEND DestBlendAlpha;
		BLEND_OP BlendOpAlpha;
		UINT8 RenderTargetWriteMask;

		RenderTargetBlendStateDesc() :
			BlendEnable(false),
			SrcBlend(BLEND_SRC_ALPHA),
			DestBlend(BLEND_INV_SRC_ALPHA),
			BlendOp(BLEND_OP_ADD),
			SrcBlendAlpha(BLEND_ONE),
			DestBlendAlpha(BLEND_ONE),
			BlendOpAlpha(BLEND_OP_ADD),
			RenderTargetWriteMask(COLOR_WRITE_ENABLE_ALL)
		{}
	};
	struct BlendStateDesc
	{
		bool AlphaToCoverageEnable;
		bool IndependentBlendEnable;
		RenderTargetBlendStateDesc RenderTarget[8];

		BlendStateDesc() :
			AlphaToCoverageEnable(false),
			IndependentBlendEnable(false)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct SamplerDesc
	{
		FILTER Filter;
		TEXTURE_ADDRESS_MODE AddressU;
		TEXTURE_ADDRESS_MODE AddressV;
		TEXTURE_ADDRESS_MODE AddressW;
		float MipLODBias;
		UINT MaxAnisotropy;
		COMPARISON_FUNC ComparisonFunc;
		float BorderColor[4];
		float MinLOD;
		float MaxLOD;

		SamplerDesc() :
			Filter(FILTER_MIN_MAG_MIP_POINT),
			AddressU(TEXTURE_ADDRESS_CLAMP),
			AddressV(TEXTURE_ADDRESS_CLAMP),
			AddressW(TEXTURE_ADDRESS_CLAMP),
			MipLODBias(0.0f),
			MaxAnisotropy(0),
			ComparisonFunc(COMPARISON_NEVER),
			BorderColor{ 0.0f,0.0f,0.0f,0.0f },
			MinLOD(0.0f),
			MaxLOD(FLT_MAX)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct RasterizerStateDesc
	{
		FILL_MODE FillMode;
		CULL_MODE CullMode;
		bool FrontCounterClockwise;
		INT DepthBias;
		float DepthBiasClamp;
		float SlopeScaledDepthBias;
		bool DepthClipEnable;
		bool MultisampleEnable;
		bool AntialiasedLineEnable;
		bool ConservativeRasterizationEnable;
		UINT ForcedSampleCount;

		RasterizerStateDesc() :
			FillMode(FILL_SOLID),
			CullMode(CULL_BACK),
			FrontCounterClockwise(false),
			DepthBias(0),
			DepthBiasClamp(0.0f),
			SlopeScaledDepthBias(0.0f),
			DepthClipEnable(true),
			MultisampleEnable(false),
			AntialiasedLineEnable(false),
			ConservativeRasterizationEnable(false),
			ForcedSampleCount(0)
		{}
	};

	struct DepthStencilOpDesc
	{
		STENCIL_OP StencilFailOp;
		STENCIL_OP StencilDepthFailOp;
		STENCIL_OP StencilPassOp;
		COMPARISON_FUNC StencilFunc;

		DepthStencilOpDesc() :
			StencilFailOp(STENCIL_OP_KEEP),
			StencilDepthFailOp(STENCIL_OP_KEEP),
			StencilPassOp(STENCIL_OP_KEEP),
			StencilFunc(COMPARISON_NEVER)
		{}
	};

	struct DepthStencilStateDesc
	{
		bool DepthEnable;
		DEPTH_WRITE_MASK DepthWriteMask;
		COMPARISON_FUNC DepthFunc;
		bool StencilEnable;
		UINT8 StencilReadMask;
		UINT8 StencilWriteMask;
		DepthStencilOpDesc FrontFace;
		DepthStencilOpDesc BackFace;

		DepthStencilStateDesc() :
			DepthEnable(false),
			DepthWriteMask(DEPTH_WRITE_MASK_ZERO),
			DepthFunc(COMPARISON_NEVER),
			StencilEnable(false),
			StencilReadMask(0xff),
			StencilWriteMask(0xff)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct HitGroupDesc
	{
		std::wstring HitGroupName;
		std::wstring ClosestHitSymbol;
		std::wstring AnyHitSymbol;
		std::wstring IntersectionSymbol;

		HitGroupDesc() :
			HitGroupName(L"HitGroup"),
			ClosestHitSymbol(L""),
			AnyHitSymbol(L""),
			IntersectionSymbol(L"")
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct RayTracingAccelerationStructureDesc
	{
		struct BottomLevelAccelerationStructure
		{
			struct Geometry
			{
				struct Triangles
				{
					UINT64				VertexBufferGPUVirtualAddress;
					UINT64				IndexBufferGPUVirtualAddress;
					UINT32				VertexCount;
					UINT32				VertexByteOffset;
					UINT32				VertexStride;
					FORMAT				VertexFormat;
					UINT32				IndexCount;
					UINT32				IndexOffset;
					INDEXBUFFER_FORMAT	IndexFormat;
					UINT64				Transform3x4BufferVirtualAddress;
					UINT32				Transform3x4BufferOffset;

					Triangles()
						: VertexBufferGPUVirtualAddress(0)
						, IndexBufferGPUVirtualAddress(0)
						, VertexCount(0)
						, VertexByteOffset(0)
						, VertexStride(0)
						, VertexFormat(FORMAT_R32G32B32_FLOAT)
						, IndexCount(0)
						, IndexOffset(0)
						, IndexFormat(INDEXFORMAT_16BIT)
						, Transform3x4BufferVirtualAddress(0)
						, Transform3x4BufferOffset(0)
					{}
				};

				struct Procedural_AABBs
				{
					UINT64		AABBBufferGPUVirtualAddress;
					UINT32		Offset;
					UINT32		Count;
					UINT32		Stride;

					Procedural_AABBs()
						: AABBBufferGPUVirtualAddress(0)
						, Offset(0)
						, Count(0)
						, Stride(0)
					{}
				};

				UINT32											  Flags;
				ACCELERATION_STRUCTURE_BOTTOM_LEVEL_GEOMETRY_TYPE Type;
				Triangles										  Triangles;
				Procedural_AABBs								  AABBs;

				Geometry()
					: Flags(AS_BOTTOM_LEVEL_GEOMETRY_FLAG_EMPTY)
					, Type(AS_BOTTOM_LEVEL_GEOMETRY_TYPE_TRIANGLES)
				{}
			};

			std::vector<Geometry> Geometries;

			BottomLevelAccelerationStructure()
			{}
		};

		struct TopLevelAccelerationStructure
		{
			struct Instance
			{
				XMFLOAT3X4		Transform;
				UINT32			InstanceID : 24;
				UINT32			InstanceMask : 8;
				UINT32			InstanceContributionToHitGroupIndex : 24;
				UINT32			Flags : 8;

				Instance()
					: Transform()
					, InstanceID(0)
					, InstanceMask(0)
					, InstanceContributionToHitGroupIndex(0)
					, Flags(RAYTRACING_INSTANCE_FLAG_NONE)
				{}
			};

			std::vector<Instance> Instances;

			TopLevelAccelerationStructure()
			{}
		};

		UINT32								Flags;
		ACCELERATION_STRUCTURE_TYPE			Type;
		BottomLevelAccelerationStructure	BottomLevelAS;
		TopLevelAccelerationStructure		TopLevelAS;

		RayTracingAccelerationStructureDesc()
			: Flags(AS_BUILD_FLAG_EMPTY)
			, Type(AS_TYPE_BOTTOMLEVEL)
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct ShaderTableDesc
	{
		UINT64  GpuAddress;
		UINT64	Size;
		UINT64	Stride;

		ShaderTableDesc()
			: GpuAddress(0)
			, Size(0)
			, Stride(0)
		{}
	};

	struct DispatchRaysDesc
	{
		ShaderTableDesc RayGeneration;
		ShaderTableDesc	Miss;
		ShaderTableDesc	HitGroup;
		ShaderTableDesc	Callable;
		UINT32			Width;
		UINT32			Height;
		UINT32			Depth;
		RAYTRACING_PASS Pass;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	struct GraphicsPSODesc
	{
		VertexShader*			VS;
		PixelShader*			PS;
		HullShader*				HS;
		DomainShader*			DS;
		GeometryShader*			GS;
		BlendState*				BS;
		RasterizerState*		RS;
		DepthStencilState*		DSS;
		VertexLayout*			IL;
		PRIMITIVETOPOLOGY		PT;
		UINT					NumRTs;
		FORMAT					RTFormats[8];
		FORMAT					DSFormat;
		SampleDesc				SampleDesc;
		UINT					SampleMask;

		GraphicsPSODesc()
		{
			SAFE_INIT(VS);
			SAFE_INIT(PS);
			SAFE_INIT(HS);
			SAFE_INIT(DS);
			SAFE_INIT(GS);
			SAFE_INIT(BS);
			SAFE_INIT(RS);
			SAFE_INIT(DSS);
			SAFE_INIT(IL);
			PT = TRIANGLELIST;
			NumRTs = 0;
			for (int i = 0; i < ARRAYSIZE(RTFormats); ++i)
			{
				RTFormats[i] = FORMAT_UNKNOWN;
			}
			DSFormat = FORMAT_UNKNOWN;
			SampleDesc.Count = 1;
			SampleDesc.Quality = 0;
			SampleMask = 0xFFFFFFFF;
		}

		~GraphicsPSODesc() {};
	};

	struct ComputePSODesc
	{
		ComputeShader*			CS;

		ComputePSODesc()
		{
			SAFE_INIT(CS);
		}
	};

	struct RayTracePSODesc
	{
		RayGenerationShader*	RGS;
		RayMissShader*			MS;
		RayClosestHitShader*	CHS;
		RayAnyHitShader*		AHS;
		RayIntersectionShader*  IS;
		HitGroup*				HitGroup;
		UINT					MaxPayloadSize;
		UINT					MaxAttributeSize;
		UINT					MaxRecursionDepth;

		RayTracePSODesc()
		{
			SAFE_INIT(RGS);
			SAFE_INIT(MS);
			SAFE_INIT(CHS);
			SAFE_INIT(AHS);
			SAFE_INIT(IS);
			SAFE_INIT(HitGroup);
			MaxPayloadSize = 0;
			MaxAttributeSize = RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
			MaxRecursionDepth = 1;
		}
	};
}