
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

	return vout;
}

#endif

#ifdef PIXEL_SHADER

cbuffer cbPerObject : register(b0)
{
	float3	 gColor;
	float	 gRoughness;
	float	 gMetalness;
	uint	 gObjectID;
};

float4 ps_main(in VertexOut pin) : SV_Target0
{
	float4 albedo = float4(gColor, 1);
	return albedo;
}

#endif