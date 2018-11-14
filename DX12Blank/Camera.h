#pragma once

#include "MathHelper.h"

class Camera
{
public:
	Camera(float aspectRatio)
		: m_aspectRatio(aspectRatio)
	{};
	~Camera() {};

	virtual void Update() = 0;

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

private:
	float m_radius = 5.0f;
	float m_phi = XM_PIDIV4;
	float m_theta = 1.5f*XM_PI;
};