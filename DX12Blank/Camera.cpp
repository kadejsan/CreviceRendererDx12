#include "stdafx.h"
#include "Camera.h"
#include "UIContext.h"

void CameraArcBall::Update()
{
	m_fov = MathHelper::Deg2Rad((float)UIContext::FOV);
	m_nearZ = UIContext::Near;
	m_farZ = UIContext::Far;

	// Convert Spherical to Cartesian coordinates.
	m_eyePos.x = m_radius * sinf(m_phi) * cosf(m_theta);
	m_eyePos.z = m_radius * sinf(m_phi) * sinf(m_theta);
	m_eyePos.y = m_radius * cosf(m_phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_prevView = m_view;

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, view);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMStoreFloat4x4(&m_invView, invView);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, proj);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMStoreFloat4x4(&m_invProj, invProj);

	m_prevViewProj = m_viewProj;

	XMMATRIX viewProj = view * proj;
	XMStoreFloat4x4(&m_viewProj, viewProj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&m_invViewProj, invViewProj);

	m_frustum.ConstructFrustum(m_farZ, m_proj, m_view);
}

/* --------------------------- FREE CAMERA ------------------------------------------*/

const XMVECTOR CameraFree::DEFAULT_FORWARD = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const XMVECTOR CameraFree::DEFAULT_RIGHT = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

void CameraFree::TranslateX(float dx)
{
	m_x = dx;
}

void CameraFree::TranslateY(float dy)
{
	m_y = dy;
}

void CameraFree::TranslateZ(float dz)
{
	m_z = dz;
}

void CameraFree::Rotate(float dx, float dy)
{
	m_angleY += dy;
	if (m_angleY >= XM_2PI)
		m_angleY -= XM_2PI;
	if (m_angleY <= 0)
		m_angleY += XM_2PI;

	m_angleX += dx;
	if (m_angleX > XM_PIDIV2 - 0.1f)
		m_angleX = XM_PIDIV2 - 0.1f;
	if (m_angleX < -XM_PIDIV2 + 0.1f)
		m_angleX = -XM_PIDIV2 + 0.1f;
}

void CameraFree::Update()
{
	// Build the view matrix.
	m_translX += m_x;
	m_translY += m_y;
	m_translZ += m_z;

	m_fov = MathHelper::Deg2Rad((float)UIContext::FOV);
	m_nearZ = UIContext::Near;
	m_farZ = UIContext::Far;

	// Convert Spherical to Cartesian coordinates.
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(m_angleX, m_angleY, 0);
	m_target = XMVector3TransformCoord(DEFAULT_FORWARD, camRotationMatrix);
	m_target = XMVector3Normalize(m_target);

	XMVECTOR camRight = XMVector3TransformCoord(DEFAULT_RIGHT, camRotationMatrix);
	XMVECTOR camForward = XMVector3TransformCoord(DEFAULT_FORWARD, camRotationMatrix);
	XMVECTOR camUp = XMVector3Cross(camForward, camRight);
	m_position += m_translX * camRight;
	m_position += m_translZ * camForward;
	m_position += m_translY * camUp;

	m_eyePos = float3(m_position.m128_f32[0], m_position.m128_f32[1], m_position.m128_f32[2]);
	m_translX = m_translZ = m_translY = 0;
	m_target += m_position;

	XMMATRIX view = XMMatrixLookAtLH(m_position, m_target, camUp);
	XMStoreFloat4x4(&m_view, view);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMStoreFloat4x4(&m_invView, invView);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, proj);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMStoreFloat4x4(&m_invProj, invProj);

	XMMATRIX viewProj = view * proj;
	XMStoreFloat4x4(&m_viewProj, viewProj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&m_invViewProj, invViewProj);

	m_frustum.ConstructFrustum(m_farZ, m_proj, m_view);
}
