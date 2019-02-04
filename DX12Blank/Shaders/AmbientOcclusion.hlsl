#include "Common.hlsl"

struct PixelShaderInput
{
	float4 position		: SV_POSITION;
	float2 texcoord		: TEXCOORD;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gInvProj;
}

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

Texture2D<float4> Normal			  : register(t0);
Texture2D<float> LinearDepth		  : register(t1);
StructuredBuffer<float3> SampleKernel : register(t2);
Texture2D<float3> Noise				  : register(t3);

SamplerState SamplerLinear			  : register(s0);
SamplerState SamplerPoint			  : register(s1);

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

float IsSky(float depth)
{
	return depth == 0.0f;
}

#define NUM_SAMPLES	 32
static const float invSamples = 1.0 / (float)NUM_SAMPLES;

float ps_main(PixelShaderInput pin) : SV_Target
{
	float2 pixelCoord = pin.texcoord.xy;
	float depth = LinearDepth.SampleLevel(SamplerPoint, pixelCoord, 0).x * gCameraFar;

	// is sky
	if (IsSky(depth)) discard;

	float3 normal = normalize(2.0 * Normal.Sample(SamplerLinear, pixelCoord).rgb - 1.0);
	float3 fres = normalize(Noise.Load(int3((64 * pin.texcoord.xy * 400) % 64, 0)).xyz * 2.0 - 1.0);

	float3 ep = float3(pin.texcoord.xy, depth);
	float bl = 0.0;
	float radD = gOcclusionRadius / depth;

	float3 ray;
	float3 occFrag;
	float  depthDiff;

	for (int i = 0; i < NUM_SAMPLES; ++i)
	{
		ray = radD * reflect(SampleKernel[i], fres);
		float2 newTex = ep.xy + ray.xy;

		occFrag = normalize(2.0 * Normal.Sample(SamplerLinear, newTex).rgb - 1.0);

		float newDepth = LinearDepth.SampleLevel(SamplerLinear, newTex, 0).r * gCameraFar;
		depthDiff = (depth - newDepth);

		const float rangeCheck = abs(depth - newDepth) < gOcclusionRange ? 1.0f : 0.0f;

		bl += (step(gOcclusionFallof, depthDiff) * (1.0 - saturate(dot(occFrag.xyz, normal))) * (1 - smoothstep(gOcclusionFallof, gOcclusionPower, depthDiff))) * rangeCheck;
	}

	float ao = 1.0 - bl * invSamples * gOcclusionDarkness;
	return saturate(ao);
}

#endif