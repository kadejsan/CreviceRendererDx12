#pragma once

namespace GraphicsTypes
{
	class GraphicsDevice;
}

class GeometryGenerator
{
public:
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const float3& p,
			const float3& n,
			const float3& t,
			const float2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {}

		float3 Position;
		float3 Normal;
		float3 TangentU;
		float2 TexC;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT32> Indices32;
		DirectX::BoundingBox BBox;

		std::vector<UINT16>& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
					mIndices16[i] = static_cast<UINT16>(Indices32[i]);
			}

			return mIndices16;
		}

	private:
		std::vector<UINT16> mIndices16;
	};

	GeometryGenerator() {};
	~GeometryGenerator() {};

public:
	static MeshData CreateBox(float width, float height, float depth, UINT32 numSubdivisions);
	static MeshData CreateSphere(float radius, UINT32 sliceCount, UINT32 stackCount);
	static MeshData CreateGeosphere(float radius, UINT32 numSubdivisions);
	static MeshData CreateCylinder(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount);
	static MeshData CreateGrid(float width, float depth, UINT32 m, UINT32 n);
	static MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
	static void Subdivide(MeshData& meshData);
	static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	static void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount, MeshData& meshData);
	static void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount, MeshData& meshData);
};