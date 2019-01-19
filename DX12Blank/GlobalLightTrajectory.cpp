#include "stdafx.h"
#include "GlobalLightTrajectory.h"
#include "MathHelper.h"

GlobalLightTrajectory::GlobalLightTrajectory()
	: m_latitude(0.0f)
	, m_timeOfYearSeason(Summer)
{
	Reset();
}

void GlobalLightTrajectory::Reset()
{
}

namespace
{
	float GetDeclinationAngleRad(ETimeOfYearSeason season)
	{
		float angles[] = { 0.0f, MathHelper::Deg2Rad(23.75f), 0.0f, MathHelper::Deg2Rad(-23.75f) };
		return angles[(UINT)season];
	}

	float GetHourAngle(float dayTime /* [0,24] */)
	{
		return MathHelper::Deg2Rad(15.0f * (dayTime - 12.0f));
	}

	UINT GetMonth(ETimeOfYearSeason season)
	{
		UINT months[] = { 3, 6, 9, 12 };
		return months[(UINT)season];
	}

	void ConvertToHorizonCoordinates(float latitudeRad, float declinationAngleRad, float ha, float& outElevation, float& outAzimuth)
	{
		float latSin = sinf(latitudeRad);
		float decSin = sinf(declinationAngleRad);
		float latCos = cosf(latitudeRad);
		float decCos = cosf(declinationAngleRad);
		float haCos = cosf(ha);

		outElevation = asinf(MathHelper::Clamp(decSin * latSin + decCos * latCos * haCos, -1.0f, 1.0f));

		float cosElevation = cosf(outElevation);

		outAzimuth = acosf(MathHelper::Clamp((decSin * latCos - decCos * latSin * haCos) / cosElevation, -1.0f, 1.0f));
		//if (ha > 0)
		//	outAzimuth = XM_PIDIV2 - outAzimuth;
	}

	void GetSunElevationAndAzimuth(float dayTime, float latitude, ETimeOfYearSeason season, float& outElevation, float& outAzimuth)
	{
		float latitudeRad = MathHelper::Deg2Rad(latitude);
		float declinationAngleRad = GetDeclinationAngleRad(season);
		float ha = GetHourAngle(dayTime);

		ConvertToHorizonCoordinates(latitudeRad, declinationAngleRad, ha, outElevation, outAzimuth);
	}

	float JulianDay(ETimeOfYearSeason season)
	{
		double year = 2019.0;
		double month = (double)GetMonth(season);
		double date = 21.0f; // day of the month (~solstice)

		//if (month <= 2) { month = month + 12; year = year - 1; }
		return (float)((365.25*year) + (30.6001*month) - 15.0 + 1720996.5 + date);
	}

	float3 CalcDir(float pitch, float yaw)
	{
		float3 dir;
		dir = float3(cosf(pitch), 0.f, sinf(pitch));
		dir = float3(dir.x * sinf(yaw), dir.x * cosf(yaw), dir.z);
		return dir;
	}
}

float3 GlobalLightTrajectory::GetSunDirection(float dayTime) const
{
#if 1
	float angle = 2.0f * XM_PI * (dayTime - 6.0f) / 24.0f;
	float3 sunDir = float3(0.0f, sinf(angle), cosf(angle));
#else
	float elevation = 0.0f, azimuth = 0.0f;
	GetSunElevationAndAzimuth(dayTime, m_latitude, m_timeOfYearSeason, elevation, azimuth);
	float3 sunDir = CalcDir(elevation, azimuth);
	XMVECTOR dirV = XMLoadFloat3(&sunDir);
	dirV = XMVector3Normalize(dirV);
	XMStoreFloat3(&sunDir, dirV);
#endif
	return float3(-sunDir.x, -sunDir.y, -sunDir.z);
}