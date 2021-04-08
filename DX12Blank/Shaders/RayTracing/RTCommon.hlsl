// Hit information, aka ray payload
struct HitInfo
{
	float4 colorAndDistance;
	float3 normal;
	float3 roughnessMetalnessID;
};

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes
{
	float2 bary;
};

struct VertexAttributes
{
	float3 Position;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
	float2 TexCoord;
};

#define NUM_COMPONENTS 14 // 14 floats
#define COMPONENT_BYTE_SIZE 4
#define INDEX_BYTE_SIZE 4

uint3 GetIndices(uint triangleIndex, ByteAddressBuffer ib)
{
	uint baseIndex = (triangleIndex * 3);
	int address = (baseIndex * INDEX_BYTE_SIZE);
	return ib.Load3(address);
}

VertexAttributes GetVertexAttributes(uint triangleIndex, float3 barycentrics, ByteAddressBuffer vb, ByteAddressBuffer ib)
{
	uint3 indices = GetIndices(triangleIndex, ib);
	VertexAttributes v = (VertexAttributes)0;

	for (uint i = 0; i < 3; i++)
	{
		int address = (indices[i] * NUM_COMPONENTS) * COMPONENT_BYTE_SIZE;
		v.Position += asfloat(vb.Load3(address)) * barycentrics[i];
		address += (3 * COMPONENT_BYTE_SIZE);
		v.Normal += asfloat(vb.Load3(address)) * barycentrics[i];
		address += (3 * COMPONENT_BYTE_SIZE);
		v.Tangent += asfloat(vb.Load3(address)) * barycentrics[i];
		address += (3 * COMPONENT_BYTE_SIZE);
		v.Bitangent += asfloat(vb.Load3(address)) * barycentrics[i];
		address += (3 * COMPONENT_BYTE_SIZE);
		v.TexCoord += asfloat(vb.Load2(address)) * barycentrics[i];
	}
	v.TexCoord.y = 1.0f - v.TexCoord.y;

	return v;
}