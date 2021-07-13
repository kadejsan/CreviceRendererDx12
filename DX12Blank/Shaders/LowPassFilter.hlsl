#pragma warning( disable : 4000 )

#include "Common.hlsl"

struct PixelShaderInput
{
	float4 position		: SV_POSITION;
	float2 texcoord		: TEXCOORD;
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

cbuffer LowPassFilterCB : register(b0)
{
	float4x4 InvView;
	float2   TexelSize;
	float2	 FilterDirection;
	int		 DoTemporalFiltering;
};

Texture2D<float> LinearDepth		: register(t0);
Texture2D<float4> Normal			: register(t1);
Texture2D<float4> AOCurrent			: register(t2);
Texture2D<float>  AOTemp			: register(t3);

SamplerState SamplerLinear			: register(s0);

static const float gauss[5] =
{
	0.05448868f, 0.2442013f, 0.40262f, 0.2442013f, 0.05448868f
};

bool isValidTap(float tapDepth, float centerDepth, float3 tapNormal, float3 centerNormal, float dotViewNormal)
{
	const float depthRelativeDifferenceEpsilonMin = 0.003f;
	const float depthRelativeDifferenceEpsilonMax = 0.02f;
	const float dotNormalsEpsilon = 0.9f;

	// Adjust depth difference epsilon based on view space normal
	float depthRelativeDifferenceEpsilon = lerp(depthRelativeDifferenceEpsilonMax, depthRelativeDifferenceEpsilonMin, dotViewNormal);

	// Check depth
	if (abs(1.0f - (tapDepth / centerDepth)) > depthRelativeDifferenceEpsilon) return false;

	// Check normals
	if (dot(tapNormal, centerNormal) < dotNormalsEpsilon) return false;

	return true;
}

// Filter ambient occlusion using low-pass tent filter and mix it with color buffer
float lowPassFilter(float2 texCoord, float2 filterDirection, const bool doTemporalFilter)
{
	float ao = 0.0f;
	float weight = 1.0f;

	float centerDepth = LinearDepth.Sample(SamplerLinear, texCoord);
	float3 centerNormal = normalize(Normal.Sample(SamplerLinear, texCoord).rgb);
	float dotViewNormal = abs(mul(InvView, float4(centerNormal, 0.0f)).z);

	float2 offsetScale = TexelSize * filterDirection;

	[unroll(5)]
	for (int i = -2; i <= 2; ++i)
	{
		float2 offset = float(i) * offsetScale;
		float2 texCoordOffseted = texCoord + offset;

		float4 tapAO = doTemporalFilter
			? AOCurrent.Sample(SamplerLinear, texCoordOffseted)
			: AOTemp.Sample(SamplerLinear, texCoordOffseted).xxxx;
		float tapDepth = LinearDepth.Sample(SamplerLinear, texCoordOffseted);
		float3 tapNormal = normalize(Normal.Sample(SamplerLinear, texCoordOffseted).rgb);

		float tapWeight = gauss[i + 2];

		if (isValidTap(tapDepth, centerDepth, tapNormal, centerNormal, dotViewNormal))
			ao += (doTemporalFilter
				? dot(tapAO, float4(0.25f, 0.25f, 0.25f, 0.25f))
				: tapAO.r) * tapWeight;
		else
			weight -= tapWeight;
	}

	ao /= weight;

	return ao;
}

float ps_main(PixelShaderInput pin) : SV_Target
{
	bool doTemporalFiltering = DoTemporalFiltering;
	
	return lowPassFilter(pin.texcoord, FilterDirection, doTemporalFiltering);
}

#endif