#pragma once

#include "MathHelper.h"
#include "Frustum.h"

class GlobalLightCamera
{
public:
	GlobalLightCamera(float3 lightDir);

	XMMATRIX m_view;
	XMMATRIX m_proj;
	XMMATRIX m_viewProj;

	float	 m_width;
	float	 m_near;
	float	 m_far;

	Frustum	 m_frustum;
};