float4 GammaToLinear(float4 c)
{
	return float4(pow(c.rgb, 2.2f), c.a);
}

float LinearizeDepth(float depth, float n, float f)
{
	return (2.0f * n) / (f + n - depth * (f - n));
}

float LinearizeDepth2(float depth, float n, float f)
{
	float z_b = depth;
	float z_n = 2.0 * z_b - 1.0;
	float lin = 2.0 * f * n / (n + f - z_n * (n - f));
	return lin;
}

float DelinearizeDepth2(float lin, float n, float f)
{
	return n * (lin - f) / ((n - f) * lin);
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

float3 CompressNormalsToUnsignedGBuffer(float3 vNormal)
{
	const float maxXYZ = max(abs(vNormal.x), max(abs(vNormal.y), abs(vNormal.z)));
	return vNormal / maxXYZ * 0.5 + 0.5;
}