// Background rendering

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

#ifdef VERTEX_SHADER

PixelShaderInput vs_main(uint vertexID : SV_VertexID)
{
	PixelShaderInput vout;

	if (vertexID == 0) {
		vout.texcoord = float2(1.0, -1.0);
		vout.position = float4(1.0, 3.0, 0.0, 1.0);
	}
	else if (vertexID == 1) {
		vout.texcoord = float2(-1.0, 1.0);
		vout.position = float4(-3.0, -1.0, 0.0, 1.0);
	}
	else /* if(vertexID == 2) */ {
		vout.texcoord = float2(1.0, 1.0);
		vout.position = float4(1.0, -1.0, 0.0, 1.0);
	}
	return vout;
}

#endif

#ifdef PIXEL_SHADER

#include "Common.hlsl"

TextureCube EnvironmentMap : register(t0);
Texture2D<float> Depth	   : register(t1);
SamplerState Sampler	   : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4	gScreenToWorld;
	float4x4	gViewProj;
	float4x4	gLightViewProj;
	float4x4	gCubemapRotation;
	float4		gScreenDim;
};

float IsSky(float depth)
{
	return depth < 1.0f;
}

float4 ps_main(PixelShaderInput pin) : SV_Target
{
	float2 pixelCoord = pin.texcoord.xy;

	// is sky
	float depth = Depth.SampleLevel(Sampler, pixelCoord, 0).x;
	if( IsSky(depth) ) discard;

	float3 viewDir = normalize( PositionFromDepth(1.0f, pixelCoord, gScreenDim.w, gScreenToWorld) );

	float3 resultColor = EnvironmentMap.SampleLevel(Sampler, mul(viewDir, (float3x3)gCubemapRotation), 0).xyz;
	return float4(resultColor, 1);
}

#endif