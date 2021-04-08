#include "Common.hlsl"

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

	// Tangent space basis vectors (for normal mapping)
	float3x3 TBN = float3x3(vin.Tangent, vin.Bitangent, vin.Normal);
	vout.TangentBasis = mul((float3x3)gWorld, transpose(TBN));

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

Texture2D<float4>	Albedo		: register(t0);
Texture2D<float4>	Normal		: register(t1);
Texture2D<float>	Roughness	: register(t2);
Texture2D<float>	Metalness	: register(t3);

SamplerState		Sampler		: register(s0);

cbuffer cbPerObject : register(b0)
{
	float3	 gColor;
	float	 gRoughness;
	float	 gMetalness;
	uint	 gObjectID;
};

void ps_main(in VertexOut pin, out PixelOut pout)
{
	float4 albedo = Albedo.Sample(Sampler, pin.TexCoord);
	float roughness = Roughness.Sample(Sampler, pin.TexCoord).r;
	float metalness = Metalness.Sample(Sampler, pin.TexCoord).r;
	
	// Get current pixel's normal and transform to world space.
	float3 N = normalize(2.0 * Normal.Sample(Sampler, pin.TexCoord).rgb - 1.0);
	N = normalize(mul(pin.TangentBasis, N));

	pout.GBuffer0 = albedo;
	pout.GBuffer1 = float4(CompressNormalsToUnsignedGBuffer(N), 0);
	pout.GBuffer2 = float4(roughness, metalness, 0, gObjectID);
}

#endif