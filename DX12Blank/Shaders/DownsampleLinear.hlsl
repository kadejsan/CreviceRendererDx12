// Converts equirectangular (lat-long) projection texture into a proper cubemap.

// Texture mip level downsampling with linear filtering (used in manual mip chain generation).

static const float gamma = 2.2;

Texture2D inputTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);

[numthreads(8, 8, 1)]
void cs_main(uint2 ThreadID : SV_DispatchThreadID)
{
	int3 sampleLocation = int3(2 * ThreadID.x, 2 * ThreadID.y, 0);
	float4 gatherValue =
		inputTexture.Load(sampleLocation, int2(0, 0)) +
		inputTexture.Load(sampleLocation, int2(1, 0)) +
		inputTexture.Load(sampleLocation, int2(0, 1)) +
		inputTexture.Load(sampleLocation, int2(1, 1));
	outputTexture[ThreadID] = 0.25 * gatherValue;
}