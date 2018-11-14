#pragma once

#include "GraphicsResource.h"

class Material
{
public:
	Material();
	~Material();

	Graphics::VertexShader*	m_vs;
	Graphics::GeometryShader*	m_gs;
	Graphics::PixelShader*		m_ps;
};