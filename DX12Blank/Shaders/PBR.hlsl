
static const float PI = 3.141592;
static const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

struct VertexIn
{
	float3 PosL		: POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float3 Bitangent: BITANGENT;
	float2 Tex		: TEXCOORD;
};

struct VertexOut
{
	float4 PosH				: SV_POSITION;
	float3 Pos				: POSITION;
	float2 TexCoord			: TEXCOORD;
	float3x3 TangentBasis	: TBASIS;
};

#ifdef VERTEX_SHADER

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gView;
	float4x4 gWorldViewProj;
};

VertexOut vs_main(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Pos = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.TexCoord = float2(vin.Tex.x, 1.0f - vin.Tex.y);

	// Tangent space basis vectors (for normal mapping)
	float3x3 TBN = float3x3(vin.Tangent, vin.Bitangent, vin.Normal);
	vout.TangentBasis = mul((float3x3)gWorld, transpose(TBN));

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
};

cbuffer cbPerObject : register(b1)
{
	float4x4	gScreenToWorld;
	float4x4	gCubemapRotation;
	float4		gScreenDim;
};

Texture2D<float4>	Albedo		: register(t0);
Texture2D<float4>	Normal		: register(t1);
Texture2D<float>	Roughness	: register(t2);
Texture2D<float>	Metalness	: register(t3);

TextureCube			SpecularMap		 : register(t4);
TextureCube			IrradianceMap    : register(t5);
Texture2D			SpecularBRDF_LUT : register(t6);

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

float4 ps_main(VertexOut pin) : SV_Target
{
	float4 albedo = GammaToLinear(Albedo.Sample(Sampler, pin.TexCoord));
	float roughness = Roughness.Sample(Sampler, pin.TexCoord).r;
	float metalness = Metalness.Sample(Sampler, pin.TexCoord).r;

	// Outgoing light direction (vector from world-space pixel position to the "eye").
	float3 Lo = normalize(gEyePosition.xyz - pin.Pos);

	// Get current pixel's normal and transform to world space.
	float3 N = normalize(2.0 * Normal.Sample(Sampler, pin.TexCoord).rgb - 1.0);
	N = normalize(mul(pin.TangentBasis, N));

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