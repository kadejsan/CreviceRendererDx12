
static const float PI = 3.141592;
static const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

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

static const uint MaxLights = 3;

struct Light
{
	float3   LightDirection;
	float    LightRadiance;
};

cbuffer cbPerObject : register(b0)
{
	float4	 gEyePosition;
	Light	 gLights[MaxLights];
	uint	 gNumLights;
	float2	 gMousePos;
};

cbuffer cbPerObject : register(b1)
{
	float4x4	gScreenToWorld;
	float4x4	gCubemapRotation;
	float4		gScreenDim;
};

Texture2D<float4>	GBuffer0		: register(t0);
Texture2D<float4>	GBuffer1		: register(t1);
Texture2D<float4>	GBuffer2		: register(t2);
Texture2D<float>	Depth			: register(t3);

TextureCube			SpecularMap		 : register(t4);
TextureCube			IrradianceMap    : register(t5);
Texture2D			SpecularBRDF_LUT : register(t6);

RWBuffer<int>		HitProxy		 : register(u0);

SamplerState		Sampler		  : register(s0);
SamplerState		spBRDFSampler : register(s1);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
	uint width, height, levels;
	SpecularMap.GetDimensions(0, width, height, levels);
	return levels;
}

float4 GammaToLinear(float4 c)
{
	return float4(pow(c.rgb, 2.2f), c.a);
}

float3 PositionFromDepth(in float depth, in float2 pixelCoord, float aspectRatio, float4x4 customScreenToWorld)
{
	float2 cpos = (pixelCoord + 0.5f) * aspectRatio;
	cpos *= 2.0f;
	cpos -= 1.0f;
	cpos.y *= -1.0f;
	float4 positionWS = mul(float4(cpos, depth, 1.0f), customScreenToWorld);
	return positionWS.xyz / positionWS.w;
}

float IsSky(float depth)
{
	return depth < 1.0f;
}

float4 ps_main(PixelShaderInput pin) : SV_Target
{
	float2 pixelCoord = pin.texcoord.xy;

	float depth = Depth.SampleLevel(Sampler, pixelCoord, 0).x;
	float4 rm = GBuffer2.Sample(Sampler, pixelCoord);
	// Hit proxy
	if (all(abs(pixelCoord - (gMousePos.xy / gScreenDim.xy)) < 0.001f))
	{
		HitProxy[0] = rm.w;
		HitProxy[1] = asuint(depth);
	}

	// is sky
	if (!IsSky(depth)) discard;

	float4 albedo = GammaToLinear(GBuffer0.Sample(Sampler, pixelCoord));
	float roughness = rm.x;
	float metalness = rm.y;

	// Outgoing light direction (vector from world-space pixel position to the "eye").
	float3 posWS = PositionFromDepth(depth, pixelCoord, gScreenDim.w, gScreenToWorld);
	float3 Lo = normalize(gEyePosition.xyz - posWS);

	// Get current pixel's normal and transform to world space.

	float3 N = normalize(2.0 * GBuffer1.Sample(Sampler, pixelCoord).rgb - 1.0);

	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));

	// Specular reflection vector.
	float3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	float3 F0 = lerp(Fdielectric.xxx, albedo.xyz, metalness);

	float3 directLighting = 0.0f;
	for(uint i = 0; i < gNumLights; ++i)
	{
		float3 Li = -gLights[i].LightDirection;
		float3 Lradiance = gLights[i].LightRadiance;

		// Half-vector between Li and Lo.
		float3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		float3 diffuseBRDF = kd * albedo.xyz;

		// Cook-Torrance specular microfacet BRDF.
		float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	float3 ambientLighting = 0.0f;
	{
		// Sample diffuse irradiance at normal direction.
		float3 irradiance = IrradianceMap.Sample(Sampler, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		float3 F = fresnelSchlick(F0, cosLo);

		// Get diffuse contribution factor (as with direct lighting).
		float3 kd = lerp(1.0 - F, 0.0, metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		float3 diffuseIBL = kd * albedo.rgb * irradiance;

		// Sample pre-filtered specular reflection environment at correct mipmap level.
		uint specularTextureLevels = querySpecularTextureLevels();
		float3 specularIrradiance = SpecularMap.SampleLevel(Sampler, mul(Lr, (float3x3)gCubemapRotation), roughness * specularTextureLevels).rgb;

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		float2 specularBRDF = SpecularBRDF_LUT.Sample(spBRDFSampler, float2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
		ambientLighting = diffuseIBL + specularIBL;
	}

	// Final pixel color.
	return float4(directLighting + ambientLighting, 1.0);
}

#endif