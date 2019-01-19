#pragma once

enum ETimeOfYearSeason
{
	Spring,
	Summer,
	Autumn,
	Winter,
};

struct GlobalLightTrajectory
{
protected:
	float				m_latitude;
	ETimeOfYearSeason	m_timeOfYearSeason;

public:
	/// Class constructor
	GlobalLightTrajectory();

	/// Reset to default
	void Reset();

public:
	float GetLatitude() const { return m_latitude; }

	ETimeOfYearSeason GetTimeOfYearSeason() const { return m_timeOfYearSeason; }

	float3 GetSunDirection(float dayTime) const;
};