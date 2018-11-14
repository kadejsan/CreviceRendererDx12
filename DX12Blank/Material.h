#pragma once

#include "GraphicsResource.h"

class Material
{
public:
	Material();
	~Material();

	GraphicsTypes::VertexShader*	m_vs;
	GraphicsTypes::GeometryShader*	m_gs;
	GraphicsTypes::PixelShader*		m_ps;
};