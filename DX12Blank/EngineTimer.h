#pragma once

namespace Graphics
{
	class GraphicsDevice;
}

class EngineTimer
{
public:
	EngineTimer();

	//! Returns the total time elapsed since Reset() was called, NOT counting any
	//! time when the clock is stopped.
	float TotalTime() const; // in seconds

	inline float DeltaTime() const { return (float)m_deltaTime; } // in seconds

	void Reset(); // call before message loop
	void Start(); // call when unpaused
	void Stop();  // call when paused
	void Tick();  // call every frame

	static double st_secondsPerCount;
	static double st_milisecondsPerCount;

private:
	double m_deltaTime;

	UINT64 m_baseTime;
	UINT64 m_pausedTime;
	UINT64 m_stopTime;
	UINT64 m_prevTime;
	UINT64 m_currTime;

	bool   m_isStopped;
};

class ScopedTimer
{
public:
	ScopedTimer(const char* name, Graphics::GraphicsDevice* device);
	~ScopedTimer();

	static void RenderPerfCounters();

private:
	UINT64 m_startQuery;
	std::string m_name;
	Graphics::GraphicsDevice* m_device;

	static std::unordered_map<std::string, double> st_Perf;
	static std::unordered_map<std::string, UINT> st_PerfCounters;
};