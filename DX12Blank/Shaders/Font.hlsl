struct VertexIn
{
	float2 pos : POSITION;
	half2 tex : TEXCOORD;
};

struct VertexOut
{
	float4 pos				: SV_POSITION;
	float2 tex				: TEXCOORD;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4 gColor;
};

#ifdef VERTEX_SHADER

VertexOut vs_main(VertexIn vIn)
{
	VertexOut Out = (VertexOut)0;

	Out.pos = mul(float4(vIn.pos.xy, 0, 1), gWorldViewProj);
	
	Out.tex= vIn.tex;

	return Out;
}

#endif

#ifdef PIXEL_SHADER

Texture2D<float4>	Texture	: register(t0);
SamplerState		Sampler	: register(s0);

float4 ps_main(VertexOut PSIn) : SV_TARGET
{
	return Texture.Sample(Sampler, PSIn.tex) * gColor;
}

#endif