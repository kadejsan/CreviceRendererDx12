struct VertexIn
{
	float3 PosL		: POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float3 Pos   : POSITION;
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
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Pos = vin.PosL;

	return vout;
}

#endif

#ifdef PIXEL_SHADER

TextureCube EnvironmentMap : register(t0);
SamplerState Sampler	   : register(s0);

float4 ps_main(VertexOut pin) : SV_Target
{
	float3 envVector = normalize(pin.Pos);
	return EnvironmentMap.SampleLevel(Sampler, envVector, 0);
}

#endif