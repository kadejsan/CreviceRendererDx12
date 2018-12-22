#pragma once

#include "MathHelper.h"
#include "Frustum.h"

enum CameraType
{
	ArcBall,
	Free,
};

class Camera
{
public:
	Camera(float aspectRatio)
		: m_aspectRatio(aspectRatio)
	{};
	~Camera() {};

	virtual void Update() = 0;
	virtual CameraType GetType() const = 0;

	void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; }

	float4x4 m_view			= MathHelper::Identity4x4();
	float4x4 m_invView		= MathHelper::Identity4x4();
	float4x4 m_proj			= MathHelper::Identity4x4();
	float4x4 m_invProj		= MathHelper::Identity4x4();
	float4x4 m_viewProj		= MathHelper::Identity4x4();
	float4x4 m_invViewProj	= MathHelper::Identity4x4();

	float3   m_eyePos		= float3(0.0f, 0.0f, 0.0f);
	float	 m_nearZ		= 1.0f;
	float	 m_farZ			= 1000.0f;
	float    m_fov			= MathHelper::Pi / 3.0f;

	Frustum	 m_frustum;

	float	 m_aspectRatio;
};

class CameraArcBall : public Camera
{
public:
	CameraArcBall( float aspectRatio )
		: Camera(aspectRatio)
	{
	}

	inline void SetRadius(float delta) 
	{
		m_radius += delta;
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 15.0f); // Restrict the radius.
	}
	inline void SetPhi(float delta) 
	{ 
		m_phi += delta; 
		m_phi = MathHelper::Clamp(m_phi, 0.1f, MathHelper::Pi - 0.1f); // Restrict the angle mPhi.
	}
	inline void SetTheta(float delta) 
	{ 
		m_theta += delta; 
	}

	inline float GetRadius() const { return m_radius; }
	inline float GetPhi() const { return m_phi; }
	inline float GetTheta() const { return m_theta; }

	virtual void Update() override;
	virtual CameraType GetType() const { return ArcBall; }

private:
	float m_radius = 5.0f;
	float m_phi = XM_PIDIV4;
	float m_theta = 1.5f*XM_PI;
};

class CameraFree : public Camera
{
public:
	CameraFree(float aspectRatio)
		: Camera(aspectRatio)
		, m_x(0)
		, m_y(0)
		, m_z(0)
		, m_angleX(0)
		, m_angleY(0)
		, m_translX(0.0f)
		, m_translY(0.0f)
		, m_translZ(-7.0f)
	{
		m_position = XMVectorSet(m_translX, m_translY, m_translZ, 0);
	}

	void TranslateX(float dx);
	void TranslateY(float dy);
	void TranslateZ(float dz);
	void Rotate(float dx, float dy);

	virtual void Update() override;
	virtual CameraType GetType() const { return Free; }

private:
	XMVECTOR m_position;
	XMVECTOR m_target;

	static const XMVECTOR DEFAULT_RIGHT;
	static const XMVECTOR DEFAULT_FORWARD;

	float m_angleX;
	float m_angleY;
	float m_distance;
	float m_minDistance;
	float m_maxDistance;

	float m_translX;
	float m_translY;
	float m_translZ;

	float m_x, m_y, m_z;
};