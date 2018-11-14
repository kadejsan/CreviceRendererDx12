#pragma once

#include "MathHelper.h"
#include "Mesh.h"

struct RenderObject
{
public:
	RenderObject();
	~RenderObject();

	float4x4	m_world = MathHelper::Identity4x4();
	std::shared_ptr<Mesh> m_mesh;
};
