#pragma once

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

private:
	double m_secondsPerCount;
	double m_deltaTime;

	UINT64 m_baseTime;
	UINT64 m_pausedTime;
	UINT64 m_stopTime;
	UINT64 m_prevTime;
	UINT64 m_currTime;

	bool   m_isStopped;
};