#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Color
{
public:
	Color()
		: m_value(g_XMOne)
	{}

	Color(FXMVECTOR vec);
	Color(const XMVECTORF32& vec);
	Color(float r, float g, float b, float a = 1.0f);
	Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8);
	explicit Color(uint32_t rgbaLittleEndian);

	operator XMVECTOR() const { return m_value; }

	float R() const { return XMVectorGetX(m_value); }
	float G() const { return XMVectorGetY(m_value); }
	float B() const { return XMVectorGetZ(m_value); }
	float A() const { return XMVectorGetW(m_value); }

	bool operator==(const Color& rhs) const { return XMVector4Equal(m_value, rhs.m_value); }
	bool operator!=(const Color& rhs) const { return !XMVector4Equal(m_value, rhs.m_value); }

	void SetR(float r) { m_value.f[0] = r; }
	void SetG(float g) { m_value.f[1] = g; }
	void SetB(float b) { m_value.f[2] = b; }
	void SetA(float a) { m_value.f[3] = a; }

	float* GetPtr(void) { return reinterpret_cast<float*>(this); }
	float& operator[](int idx) { return GetPtr()[idx]; }

	void SetRGB(float r, float g, float b) { m_value.v = XMVectorSelect(m_value, XMVectorSet(r, g, b, b), g_XMMask3); }

	Color ToSRGB() const;
	Color FromSRGB() const;

	inline static Color Max(Color a, Color b) { return Color(XMVectorMax(a, b)); }
	inline static Color Min(Color a, Color b) { return Color(XMVectorMin(a, b)); }
	inline static Color Clamp(Color x, Color a, Color b) { return Color(XMVectorClamp(x, a, b)); }

private:
	XMVECTORF32 m_value;
};