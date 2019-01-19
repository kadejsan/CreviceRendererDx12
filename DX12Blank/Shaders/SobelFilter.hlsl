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

Texture2D<float> Depth	   : register(t0);
SamplerState Sampler	   : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4	gScreenToWorld;
	float4x4	gViewProj;
	float4x4	gLightViewProj;
	float4x4	gCubemapRotation;
	float4		gScreenDim;
};

static float3x3 Kx = {
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1 };

static float3x3 Ky = {
	1, 2, 1,
	0, 0, 0,
	-1,-2,-1 };

float4 ps_main(PixelShaderInput pin) : SV_Target
{
	float2 pixelCoord = pin.texcoord.xy;

	float Lx = 0;
	float Ly = 0;

	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			float2 offset = float2(x, y) / gScreenDim.xy;
			float tex = Depth.SampleLevel(Sampler, pixelCoord + offset, 0).x;

			Lx += tex * Kx[y + 1][x + 1];
			Ly += tex * Ky[y + 1][x + 1];
		}
	}

	float g = sqrt((Lx*Lx) + (Ly*Ly));

	float4 color = 0;
	if (g > 1.0f) 
		color = float4(1, 1, 0, 1);
	
	return color;
}

#endif