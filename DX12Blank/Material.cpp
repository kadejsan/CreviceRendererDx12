#include "stdafx.h"
#include "Material.h"

Material::Material()
	: m_vs(nullptr)
	, m_gs(nullptr)
	, m_ps(nullptr)
{
}

Material::~Material()
{
	SAFE_DELETE(m_vs);
	SAFE_DELETE(m_gs);
	SAFE_DELETE(m_ps);
}
