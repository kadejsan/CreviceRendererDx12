#include "stdafx.h"
#include "Gizmo.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Camera.h"

const float Gizmo::st_threshold = 0.2f;

Gizmo::Gizmo()
	: m_isActive(false)
	, m_isDragging(false)
{

}

Gizmo::~Gizmo()
{
}

void Gizmo::Initialize()
{
	m_axis[0].m_renderObject.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[0].m_baseColor = float3(1, 0, 0);
	XMStoreFloat4x4(&m_axis[0].m_baseWorld, XMMatrixRotationZ(-XM_PIDIV2));
	m_axis[0].m_renderObject.m_world = m_axis[0].m_baseWorld;
	m_axis[0].m_renderObject.m_color = m_axis[0].m_baseColor;

	m_axis[1].m_renderObject.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[1].m_baseColor = float3(0, 1, 0);
	XMStoreFloat4x4(&m_axis[1].m_baseWorld, XMMatrixRotationX(XM_PIDIV2));
	m_axis[1].m_renderObject.m_world = m_axis[1].m_baseWorld;
	m_axis[1].m_renderObject.m_color = m_axis[1].m_baseColor;

	m_axis[2].m_renderObject.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[2].m_baseColor = float3(0, 0, 1);
	m_axis[2].m_renderObject.m_world = m_axis[2].m_baseWorld;
	m_axis[2].m_renderObject.m_color = m_axis[2].m_baseColor;
}

void Gizmo::Render(int i)
{
	m_axis[i].m_renderObject.m_mesh->Draw((*Renderer::GetDevice()));
}

Ray Gizmo::Unproject(int mouseX, int mouseY, int width, int height, const Camera* cam)
{
	float pointX = ((2.0f * (float)mouseX) / (float)width) - 1.0f;
	float pointY = (((2.0f * (float)mouseY) / (float)height) - 1.0f) * -1.0f;

	float4 pointA(pointX, pointY, 0.0f, 1);
	XMVECTOR pointAV = XMLoadFloat4(&pointA);

	float4 pointB(pointX, pointY, 1.0f, 1);
	XMVECTOR pointBV = XMLoadFloat4(&pointB);

	XMMATRIX invViewProj = XMLoadFloat4x4(&cam->m_invViewProj);

	XMVECTOR rayOrigin = XMVector4Transform(pointAV, invViewProj);
	rayOrigin /= rayOrigin.m128_f32[3];
	XMVECTOR rayEnd = XMVector4Transform(pointBV, invViewProj);
	rayEnd /= rayEnd.m128_f32[3];

	XMVECTOR rayDirection = rayEnd - rayOrigin;
	rayDirection = XMVector3Normalize(rayDirection);

	Ray	ray;
	XMStoreFloat3(&ray.m_origin, rayOrigin);
	XMStoreFloat3(&ray.m_end, rayEnd);
	XMStoreFloat3(&ray.m_direction, rayDirection);
	ray.m_t = FLT_MAX;

	return ray;
}

float Gizmo::ClosestDistanceBetweenLines(Ray& l1, Ray& l2, float3& p)
{
	XMVECTOR l1Origin = XMLoadFloat3(&l1.m_origin);
	XMVECTOR l1End = XMLoadFloat3(&l1.m_end);

	XMVECTOR l2Origin = XMLoadFloat3(&l2.m_origin);
	XMVECTOR l2End = XMLoadFloat3(&l2.m_end);

	XMVECTOR u = l1End - l1Origin;
	XMVECTOR v = l2End - l2Origin;
	XMVECTOR w = l1Origin - l2Origin;

	float a = XMVector3Dot(u, u).m128_f32[0];
	float b = XMVector3Dot(u, v).m128_f32[0];
	float c = XMVector3Dot(v, v).m128_f32[0];
	float d = XMVector3Dot(u, w).m128_f32[0];
	float e = XMVector3Dot(v, w).m128_f32[0];
	float D = a * c - b * b;
	float sc, tc;
	if (D < FLT_MIN)
	{
		sc = 0.0f;
		tc = (b > c ? d / b : e / c);
	}
	else
	{
		sc = (b*e - c * d) / D;
		tc = (a*e - b * d) / D;
	}

	XMVECTOR dP = w + (sc*u) - (tc*v);
	XMStoreFloat3(&p, tc*v);
	return XMVector3Length(dP).m128_f32[0];
}

float Gizmo::ClosestDistanceBetweenSegments(Ray& l1, Ray& l2, float3& p)
{
	XMVECTOR l1Origin = XMLoadFloat3(&l1.m_origin);
	XMVECTOR l1End = XMLoadFloat3(&l1.m_end);

	XMVECTOR l2Origin = XMLoadFloat3(&l2.m_origin);
	XMVECTOR l2End = XMLoadFloat3(&l2.m_end);

	XMVECTOR u = l1End - l1Origin;
	XMVECTOR v = l2End - l2Origin;
	XMVECTOR w = l1Origin - l2Origin;

	float a = XMVector3Dot(u, u).m128_f32[0];
	float b = XMVector3Dot(u, v).m128_f32[0];
	float c = XMVector3Dot(v, v).m128_f32[0];
	float d = XMVector3Dot(u, w).m128_f32[0];
	float e = XMVector3Dot(v, w).m128_f32[0];
	float D = a * c - b * b;
	float sc, sN, sD = D;
	float tc, tN, tD = D;

	if (D < FLT_MIN)
	{
		sN = 0.0f;
		sD = 1.0f;
		tN = e;
		tD = c;
	}
	else
	{
		sN = (b*e - c * d);
		tN = (a*e - b * d);
		if (sN < 0.0f)
		{
			sN = 0.0f;
			tN = e;
			tD = c;
		}
		else if (sN > sD)
		{
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0f)
	{
		tN = 0.0f;
		if (-d < 0.0f)
			sN = 0.0f;
		else if (-d > a)
			sN = sD;
		else 
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)
	{
		tN = tD;
		if ((-d + b) < 0.0f)
			sN = 0.0f;
		else if ((-d + b) > a)
			sN = sD;
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}

	sc = (std::abs(sN) < FLT_MIN ? 0.0f : sN / sD);
	tc = (std::abs(tN) < FLT_MIN ? 0.0f : tN / tD);

	XMVECTOR dP = w + (sc*u) - (tc*v);
	XMStoreFloat3(&p, tc*v);
	return XMVector3Length(dP).m128_f32[0];
}

void Gizmo::UpdateFocus(int mouseX, int mouseY, int width, int height, const Camera* cam)
{
	if (m_isActive && !m_isDragging)
	{
		Ray mouseRay = Unproject(mouseX, mouseY, width, height, cam);
		float3 px, py, pz;

		// X:
		Ray xRay;
		xRay.m_origin = m_position;
		xRay.m_end = float3(xRay.m_origin.x + 1, xRay.m_origin.y, xRay.m_origin.z);
		xRay.m_direction = float3(1, 0, 0);

		float dx = ClosestDistanceBetweenLines(mouseRay, xRay, px);

		// Y:
		Ray yRay;
		yRay.m_origin = m_position;
		yRay.m_end = float3(yRay.m_origin.x, yRay.m_origin.y, yRay.m_origin.z + 1);
		yRay.m_direction = float3(0, 0, 1);

		float dy = ClosestDistanceBetweenLines(mouseRay, yRay, py);

		// Z:
		Ray zRay;
		zRay.m_origin = m_position;
		zRay.m_end = float3(zRay.m_origin.x, zRay.m_origin.y + 1, zRay.m_origin.z);
		zRay.m_direction = float3(0, 1, 0);

		float dz = ClosestDistanceBetweenLines(mouseRay, zRay, pz);

		ResetColors();
		if (dx < st_threshold && dx < dy && dx < dz)
		{
			GetAxis(eAxis::X).SetActive();
			m_pick = px;
		}
		else if (dy < st_threshold && dy < dx && dy < dz)
		{
			GetAxis(eAxis::Y).SetActive();
			m_pick = py;
		}
		else if (dz < st_threshold && dz < dx && dz < dy)
		{
			GetAxis(eAxis::Z).SetActive();
			m_pick = pz;
		}
	}
}

void Gizmo::EditTransform(int mouseX, int mouseY, int width, int height, const Camera* cam, float4x4& worldTransform)
{
	if (m_isActive && m_isDragging)
	{
		Ray mouseRay = Unproject(mouseX, mouseY, width, height, cam);
		float3 px, py, pz;
		XMVECTOR prev = XMLoadFloat3(&m_pick);
		XMVECTOR cur = prev;

		if (GetAxis(eAxis::X).IsActive())
		{
			Ray xRay;
			xRay.m_origin = m_position;
			xRay.m_end = float3(xRay.m_origin.x + 1, xRay.m_origin.y, xRay.m_origin.z);
			xRay.m_direction = float3(1, 0, 0);

			float dx = ClosestDistanceBetweenLines(mouseRay, xRay, px);

			cur = XMLoadFloat3(&px);
		}

		if (GetAxis(eAxis::Y).IsActive())
		{
			Ray yRay;
			yRay.m_origin = m_position;
			yRay.m_end = float3(yRay.m_origin.x, yRay.m_origin.y, yRay.m_origin.z + 1);
			yRay.m_direction = float3(0, 0, 1);

			float dy = ClosestDistanceBetweenLines(mouseRay, yRay, py);

			cur = XMLoadFloat3(&py);
		}

		if (GetAxis(eAxis::Z).IsActive())
		{
			Ray zRay;
			zRay.m_origin = m_position;
			zRay.m_end = float3(zRay.m_origin.x, zRay.m_origin.y + 1, zRay.m_origin.z);
			zRay.m_direction = float3(0, 1, 0);

			float dz = ClosestDistanceBetweenLines(mouseRay, zRay, pz);

			cur = XMLoadFloat3(&pz);
		}

		XMVECTOR d = cur - prev;
		worldTransform._41 += d.m128_f32[0];
		worldTransform._42 += d.m128_f32[1];
		worldTransform._43 += d.m128_f32[2];

		XMStoreFloat3(&m_pick, cur);
	}
}