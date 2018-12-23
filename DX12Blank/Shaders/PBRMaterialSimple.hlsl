
struct VertexIn
{
	float3 PosL		: POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float3 Bitangent: BITANGENT;
	float2 Tex		: TEXCOORD;
};

struct VertexOut
{
	float4 PosH				: SV_POSITION;
	float3 Pos				: POSITION;
	float3 Normal			: NORMAL;
	float2 TexCoord			: TEXCOORD;
	float3x3 TangentBasis	: TBASIS;
};

#ifdef VERTEX_SHADER

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gView;
	float4x4 gWorldViewProj;
};

VertexOut vs_main(VertexIn vin)
{
	VertexOut vout = (VertexOut)0;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Pos = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.TexCoord = float2(vin.Tex.x, 1.0f - vin.Tex.y);
	vout.Normal = vin.Normal;

	return vout;
}

#endif

#ifdef PIXEL_SHADER

struct PixelOut
{
	float4 GBuffer0			: SV_Target0;
	float4 GBuffer1			: SV_Target1;
	float4 GBuffer2			: SV_Target2;
};

cbuffer cbPerObject : register(b0)
{
	float3	 gColor;
	float	 gRoughness;
	float	 gMetalness;
	uint	 gObjectID;
};

SamplerState		Sampler		  : register(s0);

float3 CompressNormalsToUnsignedGBuffer(float3 vNormal)
{
	const float maxXYZ = max(abs(vNormal.x), max(abs(vNormal.y), abs(vNormal.z)));
	return vNormal / maxXYZ * 0.5 + 0.5;
}

void ps_main(in VertexOut pin, out PixelOut pout)
{
	float4 albedo = float4(gColor, 1);
	float3 normal = pin.Normal;
	float roughness = gRoughness;
	float metalness = gMetalness;

	pout.GBuffer0 = albedo;
	pout.GBuffer1 = float4(CompressNormalsToUnsignedGBuffer(normal), 0);
	pout.GBuffer2 = float4(roughness, metalness, 0, gObjectID);
}

#endif