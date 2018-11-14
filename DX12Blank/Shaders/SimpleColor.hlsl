struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

#ifdef VERTEX_SHADER

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

VertexOut vs_main(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;

	return vout;
}

#endif

#ifdef PIXEL_SHADER

float4 ps_main(VertexOut pin) : SV_Target
{
	return pin.Color;
}

#endif