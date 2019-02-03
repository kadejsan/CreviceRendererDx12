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
	float gCameraNear;
	float gCameraFar;
	float gCameraNearInv;
	float gCameraFarInv;
};

float IsSky(float depth)
{
	return depth == 0.0f;
}

#define NUM_SAMPLES	 16
static const float invSamples = 1.0 / (float)NUM_SAMPLES;

// AO sampling directions 
static const float3 AO_SAMPLES[NUM_SAMPLES] =
{
	float3(0.355512, -0.709318, -0.102371),
	float3(0.534186, 0.71511, -0.115167),
	float3(-0.87866, 0.157139, -0.115167),
	float3(0.140679, -0.475516, -0.0639818),
	float3(-0.0796121, 0.158842, -0.677075),
	float3(-0.0759516, -0.101676, -0.483625),
	float3(0.12493, -0.0223423, -0.483625),
	float3(-0.0720074, 0.243395, -0.967251),
	float3(-0.207641, 0.414286, 0.187755),
	float3(-0.277332, -0.371262, 0.187755),
	float3(0.63864, -0.114214, 0.262857),
	float3(-0.184051, 0.622119, 0.262857),
	float3(0.110007, -0.219486, 0.435574),
	float3(0.235085, 0.314707, 0.696918),
	float3(-0.290012, 0.0518654, 0.522688),
	float3(0.0975089, -0.329594, 0.609803)
};

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
		ray = radD * reflect(AO_SAMPLES[i], fres);
		float2 newTex = ep.xy + ray.xy;

		occFrag = normalize(2.0 * Normal.Sample(SamplerLinear, newTex).rgb - 1.0);
		if (!occFrag.x && !occFrag.y && !occFrag.z)
			break;

		float newDepth = LinearDepth.SampleLevel(SamplerLinear, newTex, 0).r * gCameraFar;
		depthDiff = (depth - newDepth);

		const float rangeCheck = abs(depth - newDepth) < 2.0f ? 1.0f : 0.0f;

		bl += (step(gOcclusionFallof, depthDiff) * (1.0 - saturate(dot(occFrag.xyz, normal))) * (1 - smoothstep(gOcclusionFallof, gOcclusionPower, depthDiff))) * rangeCheck;
	}

	float ao = 1.0 - bl * invSamples * gOcclusionDarkness;
	return saturate(ao);
}

#endif