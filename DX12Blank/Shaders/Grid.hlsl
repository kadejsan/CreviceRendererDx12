struct VertexIn
{
	float3 PosL		: POSITION;
	float3 Color	: COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
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

	// Just pass vertex normal into the pixel shader.
	vout.Color = float4( vin.Color, 1 );

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

void ps_main(VertexOut pin, out PixelOut pout)
{
	pout.GBuffer0 = pin.Color;
	pout.GBuffer1 = float4(0, 1, 0, 0);
	pout.GBuffer2 = float4(1.0f, 0.0f, 0, 0);
}

#endif