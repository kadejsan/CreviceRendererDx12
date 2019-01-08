#pragma once

#include "MathHelper.h"
#include "Mesh.h"

struct RenderObject
{
public:
	RenderObject();
	~RenderObject();

	inline float GetX() const { return m_world._41; }
	inline float GetY() const { return m_world._42; }
	inline float GetZ() const { return m_world._43; }

	inline float3 GetTranslation() const { return float3(GetX(), GetY(), GetZ()); }

	inline void SetTranslation(float x, float y, float z)
	{
		m_world._41 = x;
		m_world._42 = y;
		m_world._43 = z;
	}

	float4x4	m_world = MathHelper::Identity4x4();
	float3		m_color;
	std::shared_ptr<Mesh> m_mesh;
};
