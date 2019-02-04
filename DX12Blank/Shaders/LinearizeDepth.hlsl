#include "Common.hlsl"

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

Texture2D Depth				: register(t0);
SamplerState SamplerPoint	: register(s0);

cbuffer cbPerObject : register(b1)
{
	float gScreenWidth;
	float gScreenHeight;
	uint  gSampleKernelSize;
	float gNoiseTextureDimension;
	float gOcclusionRadius;
	float gOcclusionPower;
	float gOcclusionFallof;
	float gOcclusionDarkness;
	float gOcclusionRange;
	float gCameraNear;
	float gCameraFar;
	float gCameraNearInv;
	float gCameraFarInv;
};

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

float IsSky(float depth)
{
	return depth == 1.0f;
}

float ps_main(PixelShaderInput pin) : SV_Target
{
	float depth = Depth.SampleLevel(SamplerPoint, pin.texcoord.xy, 0).r;

	if (IsSky(depth)) discard;

	return LinearizeDepth2(1.0f - depth, gCameraNear, gCameraFar) * gCameraFarInv;
}
