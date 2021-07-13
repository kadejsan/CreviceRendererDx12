#include "../Common.hlsl"
#include "RTCommon.hlsl"

cbuffer cbRayTracedGbuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4   EyePos;
    float4   ResolutionTanHalfFovYAndAspectRatio;
    float2   CameraNearFar;
};

cbuffer cbPerObject : register(b1)
{
    float3	 gColor;
    float	 gRoughness;
    float	 gMetalness;
    uint	 gObjectID;
};

// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> GBuffer1             : register(u0);
RWTexture2D<float4> GBuffer2             : register(u1);
RWTexture2D<float4> GBuffer3             : register(u2);
RWTexture2D<float> Depth                 : register(u3);

// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

ByteAddressBuffer VertexBuffer           : register(t1);
ByteAddressBuffer IndexBuffer            : register(t2);

Texture2D<float4> Albedo		         : register(t3);
Texture2D<float4> Normal		         : register(t4);
Texture2D<float>  Roughness	             : register(t5);
Texture2D<float>  Metalness	             : register(t6);

SamplerState	  Sampler		         : register(s0);

#ifdef RAY_GENERATION_SHADER

[shader("raygeneration")] 
void RayGen() 
{    
    // Get the location within the dispatched 2D grid of work items
    // (often maps to pixels, so this could represent a pixel coordinate).
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 launchDimensions = float2(DispatchRaysDimensions().xy);

    float2 resolution = ResolutionTanHalfFovYAndAspectRatio.xy;
    float tanHalfFovY = ResolutionTanHalfFovYAndAspectRatio.z;
    float aspectRatio = ResolutionTanHalfFovYAndAspectRatio.w;

    float x = GBuffer1[launchIndex].x;
    float2 d = (((launchIndex.xy + 0.5f) / resolution) * 2.f - 1.f);
    
    // Define a ray, consisting of origin, direction, and the min-max distance values
    RayDesc ray;
    ray.Origin = EyePos.xyz;
    ray.Direction = normalize((d.x * View[0].xyz * tanHalfFovY * aspectRatio) - (d.y * View[1].xyz * tanHalfFovY) + View[2].xyz);
    ray.TMin = 0;
    ray.TMax = 1000;

    // Initialize the ray payload
    HitInfo payload = (HitInfo)0;

    TraceRay(
        SceneBVH,
        RAY_FLAG_NONE,
        0xFF,
        0,
        0,
        0,
        ray,
        payload);
      
    GBuffer1[launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
    GBuffer2[launchIndex] = float4(CompressNormalsToUnsignedGBuffer(payload.normal.xyz), 0);
    GBuffer3[launchIndex] = float4(payload.roughnessMetalnessID.x, payload.roughnessMetalnessID.y, 0, payload.roughnessMetalnessID.z);
    Depth[launchIndex] = payload.colorAndDistance.a;
}

#endif

#ifdef RAY_MISS_SHADER

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
    payload.colorAndDistance = float4(0.0f, 0.0f, 0.0f, 1.f);
    payload.normal = float3(0, 0, 0);
    payload.roughnessMetalnessID = float3(0, 0, 0);
}

#endif

#ifdef RAY_CLOSEST_HIT_SHADER

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    uint triangleIndex = PrimitiveIndex();
    float3 barycentrics = float3((1.0f - attrib.bary.x - attrib.bary.y), attrib.bary.x, attrib.bary.y);
    VertexAttributes vertex = GetVertexAttributes(triangleIndex, barycentrics, VertexBuffer, IndexBuffer);

    // Tangent space basis vectors (for normal mapping)
    float3x3 TBN = float3x3(vertex.Tangent, vertex.Bitangent, vertex.Normal);
    TBN = mul((float3x3)World, transpose(TBN));

    float3 posL = mul(float4(vertex.Position, 1.0f), World).xyz;
    float lod = clamp(length(posL - EyePos.xyz)/10.0f, 0.f, 13.f);

    float4 albedo = Albedo.SampleLevel(Sampler, vertex.TexCoord, lod);
    float roughness = Roughness.SampleLevel(Sampler, vertex.TexCoord, lod).r;
    float metalness = Metalness.SampleLevel(Sampler, vertex.TexCoord, lod).r;

    // Get current pixel's normal and transform to world space.
    float3 N = normalize(2.0 * Normal.SampleLevel(Sampler, vertex.TexCoord, lod).rgb - 1.0);
    N = normalize(mul(TBN, N));

    float linearDepth = RayTCurrent();
    float depth = DelinearizeDepth2(linearDepth, CameraNearFar.x, CameraNearFar.y);

    payload.colorAndDistance = float4(albedo.rgb, 1.0f - depth);
    payload.normal = N;
    payload.roughnessMetalnessID = float3(roughness, metalness, gObjectID);
}

#endif