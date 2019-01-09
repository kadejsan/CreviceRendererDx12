#include "stdafx.h"
#include "Gizmo.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Camera.h"

const float Gizmo::st_threshold = 0.2f;

Gizmo::Gizmo()
	: m_isActive(false)
	, m_isDragging(false)
	, m_type(Translator)
{

}

Gizmo::~Gizmo()
{
}

void Gizmo::Initialize()
{
	m_axis[0].m_translator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[0].m_rotator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/ring.obj");
	m_axis[0].m_scaler.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/scaler.obj");
	m_axis[0].m_baseColor = float3(1, 0, 0);
	m_axis[0].m_translator.SetRotation(0, 0, -XM_PIDIV2);
	m_axis[0].m_rotator.SetRotation(0, 0, -XM_PIDIV2);
	m_axis[0].m_scaler.SetRotation(0, 0, -XM_PIDIV2);
	m_axis[0].m_translator.m_color = m_axis[0].m_baseColor;
	m_axis[0].m_rotator.m_color = m_axis[0].m_baseColor;
	m_axis[0].m_scaler.m_color = m_axis[0].m_baseColor;

	m_axis[1].m_translator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[1].m_rotator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/ring.obj");
	m_axis[1].m_scaler.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/scaler.obj");
	m_axis[1].m_baseColor = float3(0, 1, 0);
	m_axis[1].m_translator.SetRotation(XM_PIDIV2, 0, 0);
	m_axis[1].m_rotator.SetRotation(XM_PIDIV2, 0, 0);
	m_axis[1].m_scaler.SetRotation(XM_PIDIV2, 0, 0);
	m_axis[1].m_translator.m_color = m_axis[1].m_baseColor;
	m_axis[1].m_rotator.m_color = m_axis[1].m_baseColor;
	m_axis[1].m_scaler.m_color = m_axis[1].m_baseColor;

	m_axis[2].m_translator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/arrow.obj");
	m_axis[2].m_rotator.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/ring.obj");
	m_axis[2].m_scaler.m_mesh = Mesh::FromFile((*Renderer::GetDevice()), "Data/Meshes/scaler.obj");
	m_axis[2].m_baseColor = float3(0, 0, 1);
	m_axis[2].m_translator.m_color = m_axis[2].m_baseColor;
	m_axis[2].m_rotator.m_color = m_axis[2].m_baseColor;
	m_axis[2].m_scaler.m_color = m_axis[2].m_baseColor;
}

void Gizmo::Render(int i)
{
	switch (m_type)
	{
	case Translator:
		m_axis[i].m_translator.m_mesh->Draw((*Renderer::GetDevice()));
		break;
	case Rotator:
		m_axis[i].m_rotator.m_mesh->Draw((*Renderer::GetDevice()));
		break;
	case Scaler:
		m_axis[i].m_scaler.m_mesh->Draw((*Renderer::GetDevice()));
		break;
	}
	
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

	Ray	ray;
	XMStoreFloat3(&ray.m_origin, rayOrigin);
	XMStoreFloat3(&ray.m_end, rayEnd);
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

float Gizmo::ClosestDistanceBetweenLineAndCircle(const Ray& line, const Circle& circle, float3& p)
{
	Plane plane(circle);

	XMVECTOR lOrigin = XMLoadFloat3(&line.m_origin);
	XMVECTOR lEnd = XMLoadFloat3(&line.m_end);
	XMVECTOR lDir = XMVector3Normalize(lEnd - lOrigin);

	XMVECTOR pPoint = XMLoadFloat3(&plane.m_point);
	XMVECTOR pNormal = XMLoadFloat3(&plane.m_normal);

	//float d = XMVector3Dot(pPoint, -pNormal).m128_f32[0];
	//float t = -(d + lOrigin.m128_f32[2] * pNormal.m128_f32[2] + lOrigin.m128_f32[1] * pNormal.m128_f32[1] + lOrigin.m128_f32[0] * pNormal.m128_f32[0]) /
	//	lDir.m128_f32[2] * pNormal.m128_f32[2] + lDir.m128_f32[1] * pNormal.m128_f32[1] + lDir.m128_f32[0] * pNormal.m128_f32[0];

	float d = XMVector3Dot(pNormal, lDir).m128_f32[0];
	float t = 0.0f;
	if (std::abs(d) > FLT_MIN)
	{
		XMVECTOR p0l0 = pPoint - lOrigin;
		t = XMVector3Dot(p0l0, pNormal).m128_f32[0] / d;
		if (t >= 0)
		{
			XMVECTOR onPlane = lOrigin + t * lDir;
			XMVECTOR onCircle = pPoint + circle.m_radius * XMVector3Normalize(onPlane - pPoint);

			XMStoreFloat3(&p, onCircle);

			return XMVector3Length(onPlane - onCircle).m128_f32[0];
		}
	}

	return 2.0f;
}

void Gizmo::UpdateFocus(int mouseX, int mouseY, int width, int height, const Camera* cam)
{
	if (m_isActive && !m_isDragging)
	{
		Ray mouseRay = Unproject(mouseX, mouseY, width, height, cam);
		
		if (m_type == Translator)
		{
			float3 px, py, pz;

			// X:
			Ray xRay;
			xRay.m_origin = m_position;
			xRay.m_end = float3(xRay.m_origin.x + m_scale, xRay.m_origin.y, xRay.m_origin.z);

			float dx = ClosestDistanceBetweenSegments(mouseRay, xRay, px);

			// Y:
			Ray yRay;
			yRay.m_origin = m_position;
			yRay.m_end = float3(yRay.m_origin.x, yRay.m_origin.y, yRay.m_origin.z + m_scale);

			float dy = ClosestDistanceBetweenSegments(mouseRay, yRay, py);

			// Z:
			Ray zRay;
			zRay.m_origin = m_position;
			zRay.m_end = float3(zRay.m_origin.x, zRay.m_origin.y + m_scale, zRay.m_origin.z);

			float dz = ClosestDistanceBetweenSegments(mouseRay, zRay, pz);

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
		else if (m_type == Rotator)
		{
			float3 px, py, pz;

			// X:
			Circle xCircle;
			xCircle.m_point = m_position;
			xCircle.m_orientation = float3(1, 0, 0);
			xCircle.m_radius = m_scale;

			float dx = ClosestDistanceBetweenLineAndCircle(mouseRay, xCircle, px);

			// Y:
			Circle yCircle;
			yCircle.m_point = m_position;
			yCircle.m_orientation = float3(0, 0, 1);
			yCircle.m_radius = m_scale;

			float dy = ClosestDistanceBetweenLineAndCircle(mouseRay, yCircle, py);

			// Z:
			Circle zCircle;
			zCircle.m_point = m_position;
			zCircle.m_orientation = float3(0, 1, 0);
			zCircle.m_radius = m_scale;

			float dz = ClosestDistanceBetweenLineAndCircle(mouseRay, zCircle, pz);

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
		else if (m_type == Scaler)
		{
			float3 px, py, pz;

			// X:
			Ray xRay;
			xRay.m_origin = m_position;
			xRay.m_end = float3(xRay.m_origin.x + m_scale, xRay.m_origin.y, xRay.m_origin.z);

			float dx = ClosestDistanceBetweenSegments(mouseRay, xRay, px);

			// Y:
			Ray yRay;
			yRay.m_origin = m_position;
			yRay.m_end = float3(yRay.m_origin.x, yRay.m_origin.y, yRay.m_origin.z + m_scale);

			float dy = ClosestDistanceBetweenSegments(mouseRay, yRay, py);

			// Z:
			Ray zRay;
			zRay.m_origin = m_position;
			zRay.m_end = float3(zRay.m_origin.x, zRay.m_origin.y + m_scale, zRay.m_origin.z);

			float dz = ClosestDistanceBetweenSegments(mouseRay, zRay, pz);

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
}

void Gizmo::EditTransform(int mouseX, int mouseY, int width, int height, const Camera* cam, Transform& worldTransform)
{
	if (m_isActive && m_isDragging)
	{
		Ray mouseRay = Unproject(mouseX, mouseY, width, height, cam);
		float3 px, py, pz;
		XMVECTOR prev = XMLoadFloat3(&m_pick);
		XMVECTOR cur = prev;

		if (m_type == Translator || m_type == Scaler)
		{
			if (GetAxis(eAxis::X).IsActive())
			{
				Ray xRay;
				xRay.m_origin = m_position;
				xRay.m_end = float3(xRay.m_origin.x + 1, xRay.m_origin.y, xRay.m_origin.z);

				float dx = ClosestDistanceBetweenLines(mouseRay, xRay, px);

				cur = XMLoadFloat3(&px);
			}
			else if (GetAxis(eAxis::Y).IsActive())
			{
				Ray yRay;
				yRay.m_origin = m_position;
				yRay.m_end = float3(yRay.m_origin.x, yRay.m_origin.y, yRay.m_origin.z + 1);

				float dy = ClosestDistanceBetweenLines(mouseRay, yRay, py);

				cur = XMLoadFloat3(&py);
			}
			else if (GetAxis(eAxis::Z).IsActive())
			{
				Ray zRay;
				zRay.m_origin = m_position;
				zRay.m_end = float3(zRay.m_origin.x, zRay.m_origin.y + 1, zRay.m_origin.z);

				float dz = ClosestDistanceBetweenLines(mouseRay, zRay, pz);

				cur = XMLoadFloat3(&pz);
			}
		}
		else
		{
			if (GetAxis(eAxis::X).IsActive())
			{
				// X:
				Circle xCircle;
				xCircle.m_point = m_position;
				xCircle.m_orientation = float3(1, 0, 0);
				xCircle.m_radius = m_scale;

				float dx = ClosestDistanceBetweenLineAndCircle(mouseRay, xCircle, px);

				cur = XMLoadFloat3(&px);
			}
			else if (GetAxis(eAxis::Y).IsActive())
			{

				// Y:
				Circle yCircle;
				yCircle.m_point = m_position;
				yCircle.m_orientation = float3(0, 0, 1);
				yCircle.m_radius = m_scale;

				float dy = ClosestDistanceBetweenLineAndCircle(mouseRay, yCircle, py);

				cur = XMLoadFloat3(&py);
			}
			else if (GetAxis(eAxis::Z).IsActive())
			{
				// Z:
				Circle zCircle;
				zCircle.m_point = m_position;
				zCircle.m_orientation = float3(0, 1, 0);
				zCircle.m_radius = m_scale;

				float dz = ClosestDistanceBetweenLineAndCircle(mouseRay, zCircle, pz);

				cur = XMLoadFloat3(&pz);
			}
		}

		XMVECTOR d = cur - prev;
		if (m_type == Translator)
		{
			worldTransform.m_translation.x += d.m128_f32[0];
			worldTransform.m_translation.y += d.m128_f32[1];
			worldTransform.m_translation.z += d.m128_f32[2];
		}
		else if (m_type == Rotator)
		{
			XMVECTOR center = XMLoadFloat3(&m_position);
			XMVECTOR pP = prev - center;
			XMVECTOR pC = cur - center;

			XMVECTOR xPpPc = XMVector3Cross(pP, pC);

			float dot = XMVector3Dot(pP, pC).m128_f32[0];
			dot = MathHelper::Clamp(dot, -1.0f, 1.0f);
			float angle = std::acosf(dot) / 10.0f;
			if (std::isnan(angle)) angle = 0.0f;

			if (GetAxis(eAxis::X).IsActive())
			{
				float3 n = float3(1, 0, 0);
				XMVECTOR nV = XMLoadFloat3(&n);
				float xPpPcN = XMVector3Dot(xPpPc, nV).m128_f32[0];
				float sign = xPpPcN < 0.0f ? -1.0f : 1.0f;

				worldTransform.m_rotation.x += sign * angle;
			}
			else if (GetAxis(eAxis::Y).IsActive())
			{
				float3 n = float3(0, 0, 1);
				XMVECTOR nV = XMLoadFloat3(&n);
				float xPpPcN = XMVector3Dot(xPpPc, nV).m128_f32[0];
				float sign = xPpPcN < 0.0f ? -1.0f : 1.0f;

				worldTransform.m_rotation.z += sign * angle;
			}
			else if (GetAxis(eAxis::Z).IsActive())
			{
				float3 n = float3(0, 1, 0);
				XMVECTOR nV = XMLoadFloat3(&n);
				float xPpPcN = XMVector3Dot(xPpPc, nV).m128_f32[0];
				float sign = xPpPcN < 0.0f ? -1.0f : 1.0f;

				worldTransform.m_rotation.y += sign * angle;
			}
		}
		else if (m_type == Scaler)
		{
			if (GetAxis(eAxis::X).IsActive())
			{
				float distance = d.m128_f32[0];
				worldTransform.m_scale.x += distance;
			}
			else if (GetAxis(eAxis::Y).IsActive())
			{
				float distance = d.m128_f32[2];
				worldTransform.m_scale.z += distance;
			}
			else if (GetAxis(eAxis::Z).IsActive())
			{
				float distance = d.m128_f32[1];
				worldTransform.m_scale.y += distance;
			}
		}

		XMStoreFloat3(&m_pick, cur);
	}
}