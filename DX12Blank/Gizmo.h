#pragma once

#include "RenderObject.h"

class Camera;

enum eAxis
{
	X = 0,
	Y = 1,
	Z = 2
};

enum eGizmoType
{
	Translator,
	Rotator,
	Scaler
};

struct Ray
{
	float3 m_origin;
	float3 m_end;
	float  m_t;
};

struct Circle
{
	float3 m_point;
	float  m_radius;
	float3 m_orientation;
};

struct Plane
{
	float3 m_point;
	float3 m_normal;

	Plane(const Circle& c)
		: m_point(c.m_point)
		, m_normal(c.m_orientation)
	{
	}
};

class Gizmo
{
public:
	static const UINT	st_gizmoOffset = 2;
	static const float  st_threshold;

	Gizmo();
	~Gizmo();

	void Initialize();
	void Render(int i);
	inline void SetType(eGizmoType type) { m_type = type; }
	inline eGizmoType GetType() const { return m_type; }
	inline bool IsActive() const { return m_isActive; }
	inline bool IsDragging() const { return m_isDragging; }
	inline bool IsGizmoActive() { return GetAxis(eAxis::X).IsActive() || GetAxis(eAxis::Y).IsActive() || GetAxis(eAxis::Z).IsActive(); }
	inline void SetActive(bool active, float3 position = float3(0,0,0)) { m_isActive = active; m_position = position; }
	inline void SetDrag(bool drag) 
	{ 
		bool isGizmoActive = IsGizmoActive();
		m_isDragging = isGizmoActive && drag;
		if(!drag) m_position = GetAxis(eAxis::X).m_translator.GetTranslation();		
	}

	Ray Unproject(int mouseX, int mouseY, int width, int height, const Camera* cam);

	float ClosestDistanceBetweenLines(Ray& l1, Ray& l2, float3& p);
	float ClosestDistanceBetweenSegments(Ray& l1, Ray& l2, float3& p);
	float ClosestDistanceBetweenLineAndCircle(const Ray& line, const Circle& c, float3& p);

	struct Axis
	{
		float3		 m_baseColor;
		RenderObject m_translator;
		RenderObject m_rotator;
		RenderObject m_scaler;

		inline void SetActive()
		{
			m_translator.m_color = float3(1, 1, 0);
			m_rotator.m_color = float3(1, 1, 0);
			m_scaler.m_color = float3(1, 1, 0);
		}

		inline bool IsActive() const 
		{
			return m_translator.m_color.x == 1 && m_translator.m_color.y == 1 && m_translator.m_color.z == 0;
		}
	};

	void UpdateFocus(int mouseX, int mouseY, int width, int height, const Camera* cam);

	void EditTransform(int mouseX, int mouseY, int width, int height, const Camera* cam, Transform& worldTransform);
	
	inline void ResetColors()
	{
		GetAxis(0).m_translator.m_color = GetAxis(0).m_baseColor;
		GetAxis(1).m_translator.m_color = GetAxis(1).m_baseColor;
		GetAxis(2).m_translator.m_color = GetAxis(2).m_baseColor;
		
		GetAxis(0).m_rotator.m_color = GetAxis(0).m_baseColor;
		GetAxis(1).m_rotator.m_color = GetAxis(1).m_baseColor;
		GetAxis(2).m_rotator.m_color = GetAxis(2).m_baseColor;

		GetAxis(0).m_scaler.m_color = GetAxis(0).m_baseColor;
		GetAxis(1).m_scaler.m_color = GetAxis(1).m_baseColor;
		GetAxis(2).m_scaler.m_color = GetAxis(2).m_baseColor;
	}
	
	inline void SetTransform(const Transform& t)
	{
		for (int i = eAxis::X; i <= eAxis::Z; ++i)
		{
			m_axis[i].m_translator.SetTranslation(t.m_translation.x, t.m_translation.y, t.m_translation.z);
			m_axis[i].m_rotator.SetTranslation(t.m_translation.x, t.m_translation.y, t.m_translation.z);
			m_axis[i].m_scaler.SetTranslation(t.m_translation.x, t.m_translation.y, t.m_translation.z);
		}
	}
	
	inline void SetScale(float xyz)
	{
		m_scale = 2.0f * xyz;
		for (int i = eAxis::X; i <= eAxis::Z; ++i)
		{
			m_axis[i].m_translator.SetScale(xyz, xyz, xyz);
			m_axis[i].m_rotator.SetScale(xyz, xyz, xyz);
			m_axis[i].m_scaler.SetScale(xyz, xyz, xyz);
		}
	}

	inline Axis& GetAxis(UINT8 i) { assert(i < 3); return m_axis[i]; }
private:

	Axis				m_axis[3];
	eGizmoType			m_type;
	bool				m_isActive;
	bool				m_isDragging;
	float3				m_position;
	float				m_scale;
	float3				m_pick;
};