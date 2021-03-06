#pragma once

#include "MathHelper.h"
#include "Mesh.h"

struct Transform
{
	float3 m_translation;
	float3 m_rotation;
	float3 m_scale;

	Transform()
		: m_translation(0,0,0)
		, m_rotation(0,0,0)
		, m_scale(1,1,1)
	{}
};

struct RenderObject
{
public:
	RenderObject();
	~RenderObject();

	inline bool IsEnabled() const { return m_isEnabled; }
	inline void SetEnabled(bool enabled) { m_isEnabled = enabled; }
	inline void SetCastsShadows(bool castsShadows) { m_castsShadows = castsShadows; }
	inline bool CastsShadows() const { return m_castsShadows; }

	inline float GetX() const { return m_transform.m_translation.x; }
	inline float GetY() const { return m_transform.m_translation.y; }
	inline float GetZ() const { return m_transform.m_translation.z; }

	inline const Transform& GetTransform() const { return m_transform; }

	inline float3 GetScale() const { return float3(m_transform.m_scale.x, m_transform.m_scale.y, m_transform.m_scale.z); }
	inline float3 GetRotation() const { return float3(m_transform.m_rotation.x, m_transform.m_rotation.y, m_transform.m_rotation.z); }
	inline float3 GetTranslation() const { return float3(GetX(), GetY(), GetZ()); }
	
	inline void SetScale(float x, float y, float z)
	{
		m_transform.m_scale.x = x;
		m_transform.m_scale.y = y;
		m_transform.m_scale.z = z;
		SetWorld();
	}

	inline void SetRotation(float x, float y, float z)
	{
		m_transform.m_rotation.x = x;
		m_transform.m_rotation.y = y;
		m_transform.m_rotation.z = z;
		SetWorld();
	}

	inline void SetTranslation(float x, float y, float z)
	{
		m_transform.m_translation.x = x;
		m_transform.m_translation.y = y;
		m_transform.m_translation.z = z;
		SetWorld();
	}

	inline void SetColor(float r, float g, float b)
	{
		m_color = float3(r, g, b);
	}

	inline void SetRoughness(float r)
	{
		m_roughness = r;
	}

	inline void SetMetalness(float m)
	{
		m_metalness = m;
	}

	inline void SetWorld()
	{
		XMMATRIX scale = XMMatrixScaling(m_transform.m_scale.x, m_transform.m_scale.y, m_transform.m_scale.z);
		XMMATRIX rotation = XMMatrixRotationX(m_transform.m_rotation.x) * XMMatrixRotationY(m_transform.m_rotation.y) * XMMatrixRotationZ(m_transform.m_rotation.z);
		XMMATRIX translation = XMMatrixTranslation(m_transform.m_translation.x, m_transform.m_translation.y, m_transform.m_translation.z);

		XMMATRIX transform = scale * rotation * translation;
		XMStoreFloat4x4(&m_world, transform);
	}

	inline const float4x4& GetWorld() const
	{
		return m_world;
	}

	bool		m_isEnabled;
	bool		m_castsShadows;
	Transform	m_transform;
	float4x4	m_world = MathHelper::Identity4x4();
	float3		m_color;
	float		m_roughness;
	float		m_metalness;
	std::shared_ptr<Mesh> m_mesh;
};
