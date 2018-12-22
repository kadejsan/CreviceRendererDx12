#include "stdafx.h"
#include "EngineTimer.h"
#include "GraphicsDevice.h"
#include "TextRenderer.h"

double EngineTimer::st_secondsPerCount = 0.0f;
double EngineTimer::st_milisecondsPerCount = 0.0f;

EngineTimer::EngineTimer()
	: m_deltaTime( -1.0f )
	, m_baseTime( 0 )
	, m_pausedTime( 0 )
	, m_prevTime( 0 )
	, m_currTime( 0 )
	, m_isStopped( false )
{
	UINT64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	st_secondsPerCount = 1.0f / (double)countsPerSec;
	st_milisecondsPerCount = st_secondsPerCount * 1000.0f;
}

float EngineTimer::TotalTime() const
{
	if (m_isStopped)
	{
		return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * st_secondsPerCount);
	}
	else
	{
		return (float)(((m_currTime - m_pausedTime) - m_baseTime) * st_secondsPerCount);
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
	m_deltaTime = (m_currTime - m_prevTime) * st_secondsPerCount;

	// Prepare for next frame
	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0f)
	{
		m_deltaTime = 0.0f;
	}
}

// ----------------------------------------------------------------------------------

std::unordered_map<std::string, double> ScopedTimer::st_Perf;
std::unordered_map<std::string, UINT> ScopedTimer::st_PerfCounters;

void ScopedTimer::RenderPerfCounters()
{
	UINT y = 24;
	for(auto c : st_Perf)
	{
		std::stringstream ss("");
		ss << c.first << ": " << c.second / st_PerfCounters[c.first] << "ms";
		TextRenderer::Font(ss.str(), TextRenderer::FontProps(4, y, -1, TextRenderer::WIFALIGN_LEFT, TextRenderer::WIFALIGN_TOP, 2, 1, Color(255, 255, 255, 255, 8), Color(0, 0, 0, 255, 8))).Draw();
		y += 15;
	}
}

ScopedTimer::ScopedTimer(const char* name, Graphics::GraphicsDevice* device)
	: m_name(name)
	, m_device(device)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startQuery);

	device->BeginProfilerBlock(name);

	st_Perf[m_name] = 0.0;
	st_PerfCounters[m_name] = 0;
}

ScopedTimer::~ScopedTimer()
{
	UINT64 endQuery;
	QueryPerformanceCounter((LARGE_INTEGER*)&endQuery);

	m_device->EndProfilerBlock();

	st_Perf[m_name] = EngineTimer::st_milisecondsPerCount*(endQuery - m_startQuery);
	st_PerfCounters[m_name]++;
}
