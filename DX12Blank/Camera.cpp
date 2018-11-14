#include "stdafx.h"
#include "Camera.h"

void CameraArcBall::Update()
{
	// Convert Spherical to Cartesian coordinates.
	m_eyePos.x = m_radius * sinf(m_phi) * cosf(m_theta);
	m_eyePos.z = m_radius * sinf(m_phi) * sinf(m_theta);
	m_eyePos.y = m_radius * cosf(m_phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, view);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMStoreFloat4x4(&m_invView, invView);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, m_aspectRatio, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, proj);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMStoreFloat4x4(&m_invProj, invProj);

	XMMATRIX viewProj = view * proj;
	XMStoreFloat4x4(&m_viewProj, viewProj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&m_invViewProj, invViewProj);

	m_frustum.ConstructFrustum(m_farZ, m_proj, m_view);
}
