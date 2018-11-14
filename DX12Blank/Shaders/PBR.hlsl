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
	float4x4 gView;
	float4x4 gWorldViewProj;
};

VertexOut vs_main(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Pos = mul(float4(vin.PosL, 1.0f), gView);
	vout.TexCoord = float2(vin.Tex.x, 1.0f - vin.Tex.y);

	// Tangent space basis vectors (for normal mapping)
	float3x3 TBN = float3x3(vin.Tangent, vin.Bitangent, vin.Normal);
	vout.TangentBasis = mul((float3x3)gView, transpose(TBN));

	return vout;
}

#endif

#ifdef PIXEL_SHADER

Texture2D<float4>	Texture	: register(t0);
SamplerState		Sampler	: register(s0);

float4 ps_main(VertexOut pin) : SV_Target
{
	float4 color = Texture.Sample(Sampler, pin.TexCoord);
	return float4(color.xyz, 1.0f);
}

#endif