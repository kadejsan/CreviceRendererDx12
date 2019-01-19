#include "stdafx.h"
#include "Camera.h"
#include "GlobalLightCamera.h"
#include "UIContext.h"

GlobalLightCamera::GlobalLightCamera(float3 lightDir)
	: m_width(15.0f)
	, m_near(1.0f)
	, m_far(100.0f)
{
	XMVECTOR lightDirV = XMLoadFloat3(&lightDir);
	XMVECTOR lightPosV = -10.0f*lightDirV;
	XMVECTOR rightV = CameraFree::DEFAULT_RIGHT;
	XMVECTOR upV = XMVector3Cross(lightDirV, rightV);

	m_view = XMMatrixLookToLH(lightPosV, lightDirV, upV);
	m_proj = XMMatrixOrthographicLH(m_width, m_width, m_near, m_far);
	m_viewProj = m_view * m_proj;

	float4x4 view;
	XMMATRIX projV = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, m_near, m_far);
	float4x4 proj;
	XMStoreFloat4x4(&view, m_view);
	XMStoreFloat4x4(&proj, projV);

	m_frustum.ConstructFrustum(m_far, proj, view);
}
