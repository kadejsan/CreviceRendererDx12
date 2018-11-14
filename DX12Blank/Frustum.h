#pragma once

#include "MathHelper.h"

struct AABB;

#define FRUSTUM_PLANE_NEAR 0
#define FRUSTUM_PLANE_FAR 1
#define FRUSTUM_PLANE_LEFT 2
#define FRUSTUM_PLANE_RIGHT 3
#define FRUSTUM_PLANE_TOP 4
#define FRUSTUM_PLANE_BOTTOM 5

class Frustum
{
public:
	Frustum();
	void CleanUp();

	void ConstructFrustum(float screenDepth, float4x4& projMatrix, const float4x4& viewMatrix, const float4x4& world = MathHelper::Identity4x4());

	bool CheckPoint(const float3& pt) const ;
	bool CheckSphere(const float3& center, float radius) const;

#define BOX_FRUSTUM_INTERSECTS 1
#define BOX_FRUSTUM_INSIDE 2
	INT CheckBox(const BoundingBox& box) const;

	inline const float4& GetLeftPlane() const { return m_planes[FRUSTUM_PLANE_LEFT]; }
	inline const float4& GetRightPlane()const { return m_planes[FRUSTUM_PLANE_RIGHT]; }
	inline const float4& GetTopPlane()const { return m_planes[FRUSTUM_PLANE_TOP]; }
	inline const float4& GetBottomPlane()const { return m_planes[FRUSTUM_PLANE_BOTTOM]; }
	inline const float4& GetFarPlane()const { return m_planes[FRUSTUM_PLANE_FAR]; }
	inline const float4& GetNearPlane()const { return m_planes[FRUSTUM_PLANE_NEAR]; }
	inline float3 GetCamPos()
	{
		return float3(-m_view._41, -m_view._42, -m_view._43);
	}

private:
	float4		m_planesNorm[6];
	float4		m_planes[6];
	float4x4	m_view;
};