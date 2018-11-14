#include "stdafx.h"
#include "EngineTimer.h"

EngineTimer::EngineTimer()
	: m_secondsPerCount( 0.0f )
	, m_deltaTime( -1.0f )
	, m_baseTime( 0 )
	, m_pausedTime( 0 )
	, m_prevTime( 0 )
	, m_currTime( 0 )
	, m_isStopped( false )
{
	UINT64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0f / (double)countsPerSec;
}

float EngineTimer::TotalTime() const
{
	if (m_isStopped)
	{
		return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
	else
	{
		return (float)(((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
}

void EngineTimer::Reset()
{
	UINT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_isStopped = false;
}

void EngineTimer::Start()
{
	UINT64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_isStopped)
	{
		m_pausedTime += (startTime - m_stopTime);

		m_prevTime = startTime;
		m_stopTime = 0;
		m_isStopped = false;
	}
}

void EngineTimer::Stop()
{
	// If we are already stopped, then don't do anything.
	if (!m_isStopped)
	{
		UINT64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// Otherwise, save the time we stopped at, and set isStopped flag.
		m_stopTime = currTime;
		m_isStopped = true;
	}
}

void EngineTimer::Tick()
{
	if (m_isStopped)
	{
		m_deltaTime = 0.0f;
		return;
	}

	// Get the time this frame
	UINT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	// Time difference between this frame and the previous.
	m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;

	// Prepare for next frame
	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0f)
	{
		m_deltaTime = 0.0f;
	}
}
