#include "../Common.hlsl"
#include "RTCommon.hlsl"

cbuffer cbRayTracedGbuffer : register(b0)
{
    float4x4 View;
    float4   EyePos;
    float4   ResolutionTanHalfFovYAndAspectRatio;
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

    payload.colorAndDistance = float4(gColor, 1.0f - RayTCurrent() / 1000.0f);
    payload.normal = vertex.Normal;
    payload.roughnessMetalnessID = float3(gRoughness, gMetalness, gObjectID);
}

#endif