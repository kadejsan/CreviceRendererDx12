#include "../Common.hlsl"

struct HitInfo
{
	float T;
};

struct Attributes 
{
	float2 bary;
};

#ifdef RAY_GENERATION_SHADER

cbuffer cbView : register(b0)
{
	float4x4 View;
	float4x4 PrevView;
	float4x4 PrevViewProj;
	float4	 EyePos;
	float4	 ResolutionTanHalfFovYAndAspectRatio;
	float2	 CameraNearFar;
};

cbuffer cbRTAO : register(b1)
{
	float  AORadius;
	float  AOStrength;
	int	   FrameNo;
	int    SampleCount;
	int	   SampleStartIndex;
};

RaytracingAccelerationStructure SceneBVH : register(t0);
Texture2D<float4> Normal				 : register(t1);
Texture2D<float> LinearDepthCurrent		 : register(t2);
Texture2D<float> LinearDepthPrevious	 : register(t3);
Texture1D<float3> SampleKernel			 : register(t4);
Texture2D<float4> AOPrevious			 : register(t5);

RWTexture2D<float4> AOOutput			 : register(u0);

SamplerState LinearClampSampler			 : register(s0);

float IsSky(float depth)
{
	return depth == 0.0f;
}

[shader("raygeneration")]
void RayGen()
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 launchDimensions = float2(DispatchRaysDimensions().xy);

	float2 resolution = ResolutionTanHalfFovYAndAspectRatio.xy;
	float tanHalfFovY = ResolutionTanHalfFovYAndAspectRatio.z;
	float aspectRatio = ResolutionTanHalfFovYAndAspectRatio.w;

	float depth = LinearDepthCurrent[launchIndex].x;
	float3 normal = Normal[launchIndex].xyz;

	// is sky
	if (IsSky(depth))
	{
		AOOutput[launchIndex] = 1.0f;
		return;
	}

	// Calculate AO:
	float2 d = (((launchIndex.xy + 0.5f) / resolution) * 2.f - 1.f);
	float3 rayOrigin = EyePos.xyz;
	float3 rayDirection = normalize((d.x * View[0].xyz * tanHalfFovY * aspectRatio) - (d.y * View[1].xyz * tanHalfFovY) + View[2].xyz);

	// Pixel world space position (using length of a primary ray found in previous pass)
	float3 pixelWorldSpacePosition = rayOrigin + (rayDirection * depth * CameraNearFar.y);

	// Construct TBN matrix to orient sampling hemisphere along the surface normal
	float3 n = normalize(normal);

	float3 rvec = rayDirection;
	float3 b1 = normalize(rvec - n * dot(rvec, n));
	float3 b2 = cross(n, b1);
	float3x3 tbn = float3x3(b1, b2, n);

	// Pick subset of samples to use based on "frame number % 4" and position on screen within a block of 3x3 pixels
	int pixelIdx = dot(int2(fmod(float2(launchIndex), 3)), int2(1, 3));
	int currentSamplesStartIndex = SampleStartIndex + (pixelIdx * SampleCount) + int(FrameNo * SampleCount * 9);

	// Setup ray desc and payload
	RayDesc aoRay;
	HitInfo aoHitData;

	aoRay.Origin = pixelWorldSpacePosition;
	aoRay.TMin = 0.1f;
	aoRay.TMax = AORadius; //< Set max ray length to AO radius for early termination

	float ao = 0.0f;
	
	[unroll(4)]
	for (int i = 0; i < SampleCount; i++)
	{
		float3 aoSampleDirection = mul(SampleKernel.Load(int2(currentSamplesStartIndex + i, 0)).rgb, tbn);

		// Setup the ray
		aoRay.Direction = aoSampleDirection;
		aoHitData.T = AORadius; //< Set T to "maximum", to produce no occlusion in case ray doesn't hit anything (miss shader won't modify this value)

		// Trace the ray
		TraceRay(
			SceneBVH,
			RAY_FLAG_NONE,
			0xFF,
			0,
			0,
			0,
			aoRay,
			aoHitData);

		ao += aoHitData.T / AORadius;
	}

	ao /= float(SampleCount);
	
	// Reverse reprojection
	float4 previousPixelViewSpacePosition = mul(PrevView, float4(pixelWorldSpacePosition, 1.0f));
	float previousReprojectedLinearDepth = length(previousPixelViewSpacePosition.xyz);
	float4 previousPixelScreenSpacePosition = mul(PrevViewProj, float4(pixelWorldSpacePosition, 1.0f));
	previousPixelScreenSpacePosition.xyz /= previousPixelScreenSpacePosition.w;

	float2 previousUvs = (previousPixelScreenSpacePosition.xy * float2(0.5f, -0.5f)) + 0.5f;

	bool isReprojectionValid = true;

	// Discard invalid reprojection (outside of the frame)
	if (previousUvs.x > 1.0f || previousUvs.x < 0.0f ||
		previousUvs.y > 1.0f || previousUvs.y < 0.0f)
		isReprojectionValid = false;

	// Discard invalid reprojection (depth mismatch)
	const float maxReprojectionDepthDifference = 0.03f;
	float previousSampledLinearDepth = LinearDepthPrevious.SampleLevel(LinearClampSampler, previousUvs.xy, 0) * CameraNearFar.y;

	if (abs(1.0f - (previousReprojectedLinearDepth / previousSampledLinearDepth)) > maxReprojectionDepthDifference)
		isReprojectionValid = false;

	// Store AO to history cache
	if (isReprojectionValid)
		AOOutput[launchIndex.xy] = float4(ao, AOPrevious.SampleLevel(LinearClampSampler, previousUvs.xy, 0).xyz); //< Store current AO in history cache
	else
		AOOutput[launchIndex.xy] = ao.xxxx; //< Replace all cached AO with current result
}

#endif

#ifdef RAY_MISS_SHADER

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
	// Intentionally left empty
}

#endif

#ifdef RAY_CLOSEST_HIT_SHADER

[shader("closesthit")]
void ClosestHit(inout HitInfo payload : SV_RayPayload, Attributes attrib : SV_IntersectionAttributes)
{
	payload.T = RayTCurrent();
}

#endif