#include "stdafx.h"
#include "RenderObject.h"

RenderObject::RenderObject()
	: m_isEnabled(true)
	, m_castsShadows(true)
	, m_mesh(nullptr)
	, m_roughness(0.5f)
	, m_metalness(0.5f)
{

}

RenderObject::~RenderObject()
{
}
