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

cbuffer cbPerObject : register(b0)
{
	uint gBlurDimension;
};

Texture2D<float> Texture : register(t0);

SamplerState SamplerLinear			  : register(s0);

float ps_main(PixelShaderInput pin) : SV_Target
{
	float2 pixelCoord = pin.texcoord.xy;

	float w;
	float h;
	Texture.GetDimensions(w, h);
	const float2 texelSize = 1.0f / float2(w, h);
	float result = 0.0f;
	const float hlimComponent = -float(gBlurDimension) * 0.5f + 0.5f;
	const float2 hlim = float2(hlimComponent, hlimComponent);
	for (uint i = 0U; i < gBlurDimension; ++i) 
	{
		for (uint j = 0U; j < gBlurDimension; ++j) 
		{
			const float2 offset = (hlim + float2(float(i), float(j))) * texelSize;
			result += Texture.Sample(SamplerLinear, pixelCoord + offset).r;
		}
	}

	return result / float(gBlurDimension * gBlurDimension);
}

#endif