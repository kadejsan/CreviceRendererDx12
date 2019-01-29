float4 GammaToLinear(float4 c)
{
	return float4(pow(c.rgb, 2.2f), c.a);
}

float LinearizeDepth(float depth, float n, float f)
{
	return (2.0f * n) / (f + n - depth * (f - n));
}

float3 PositionFromDepth(in float depth, in float2 pixelCoord, float aspectRatio, float4x4 screenToWorld)
{
	float2 cpos = pixelCoord;
	cpos *= 2.0f;
	cpos -= 1.0f;
	cpos.y *= -1.0f;
	float4 positionWS = mul(float4(cpos, depth, 1.0f), screenToWorld);
	positionWS /= positionWS.w;
	return positionWS.xyz;
}