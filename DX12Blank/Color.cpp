#include "stdafx.h"
#include "Color.h"

Color::Color(FXMVECTOR vec)
{
	m_value.v = vec;
}

Color::Color(const XMVECTORF32& vec)
{
	m_value = vec;
}

Color::Color(float r, float g, float b, float a)
{
	m_value.v = XMVectorSet(r, g, b, a);
}

Color::Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth)
{
	m_value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / ((1 << bitDepth) - 1));
}

Color::Color(uint32_t u32)
{
	float r = (float)((u32 >> 0) & 0xFF);
	float g = (float)((u32 >> 8) & 0xFF);
	float b = (float)((u32 >> 16) & 0xFF);
	float a = (float)((u32 >> 24) & 0xFF);
	m_value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / 255.0f);
}

Color Color::ToSRGB(void) const
{
	XMVECTOR T = XMVectorSaturate(m_value);
	XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
	result = XMVectorSelect(result, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return XMVectorSelect(T, result, g_XMSelect1110);
}

Color Color::FromSRGB(void) const
{
	XMVECTOR T = XMVectorSaturate(m_value);
	XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
	result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return XMVectorSelect(T, result, g_XMSelect1110);
}