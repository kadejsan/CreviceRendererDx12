#pragma once

#include "RenderObject.h"

class Camera;

enum eAxis
{
	X = 0,
	Y = 1,
	Z = 2
};

struct Ray
{
	float3 m_origin;
	float3 m_end;
	float3 m_direction;
	float  m_t;
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
	inline bool IsActive() const { return m_isActive; }
	inline bool IsDragging() const { return m_isDragging; }
	inline bool IsGizmoActive() { return GetAxis(eAxis::X).IsActive() || GetAxis(eAxis::Y).IsActive() || GetAxis(eAxis::Z).IsActive(); }
	inline void SetActive(bool active, float3 position = float3(0,0,0)) { m_isActive = active; m_position = position; }
	inline void SetDrag(bool drag) 
	{ 
		bool isGizmoActive = IsGizmoActive();
		m_isDragging = isGizmoActive && drag;
		if(!drag) m_position = GetAxis(eAxis::X).m_renderObject.GetTranslation();		
	}

	Ray Unproject(int mouseX, int mouseY, int width, int height, const Camera* cam);

	float ClosestDistanceBetweenLines(Ray& l1, Ray& l2, float3& p);
	float ClosestDistanceBetweenSegments(Ray& l1, Ray& l2, float3& p);

	struct Axis
	{
		float3		 m_baseColor;
		float4x4	 m_baseWorld = MathHelper::Identity4x4();
		RenderObject m_renderObject;

		inline void SetActive()
		{
			m_renderObject.m_color = float3(1, 1, 0);
		}

		inline bool IsActive() const 
		{
			return m_renderObject.m_color.x == 1 && m_renderObject.m_color.y == 1 && m_renderObject.m_color.z == 0;
		}
	};

	void UpdateFocus(int mouseX, int mouseY, int width, int height, const Camera* cam);
	void EditTransform(int mouseX, int mouseY, int width, int height, const Camera* cam, float4x4& worldTransform);
	
	inline void ResetColors()
	{
		GetAxis(0).m_renderObject.m_color = GetAxis(0).m_baseColor;
		GetAxis(1).m_renderObject.m_color = GetAxis(1).m_baseColor;
		GetAxis(2).m_renderObject.m_color = GetAxis(2).m_baseColor;
	}
	
	inline void SetTranslation(float x, float y, float z)
	{
		for (int i = 0; i < 3; ++i)
		{
			GetAxis(i).m_renderObject.SetTranslation(x, y, z);
		}
	}
	
	inline void SetScale(float xyz)
	{
		XMMATRIX world0 = XMMatrixMultiply(XMLoadFloat4x4(&m_axis[0].m_baseWorld), XMMatrixScaling(xyz, xyz, xyz));
		XMStoreFloat4x4(&m_axis[0].m_renderObject.m_world, world0);
		XMMATRIX world1 = XMMatrixMultiply(XMLoadFloat4x4(&m_axis[1].m_baseWorld), XMMatrixScaling(xyz, xyz, xyz));
		XMStoreFloat4x4(&m_axis[1].m_renderObject.m_world, world1);
		XMMATRIX world2 = XMMatrixMultiply(XMLoadFloat4x4(&m_axis[2].m_baseWorld), XMMatrixScaling(xyz, xyz, xyz));
		XMStoreFloat4x4(&m_axis[2].m_renderObject.m_world, world2);
	}

	inline Axis& GetAxis(UINT8 i) { assert(i < 3); return m_axis[i]; }
private:

	Axis				m_axis[3];
	bool				m_isActive;
	bool				m_isDragging;
	float3				m_position;
	float3				m_pick;
};