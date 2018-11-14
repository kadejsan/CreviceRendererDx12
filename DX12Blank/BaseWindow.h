#pragma once

#include "Win32Application.h"
#include "EngineTimer.h"

class BaseWindow
{
public:
	BaseWindow( std::string windowName );
	virtual ~BaseWindow();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	virtual void OnKeyDown( UINT8 key ) {}
	virtual void OnKeyUp( UINT8 key ) {}

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

	inline UINT32 GetWidth() const { return m_width; }
	inline UINT32 GetHeight() const { return m_height; }
	inline float AspectRatio() const { return static_cast<float>(m_width) / m_height; }
	inline const CHAR* GetTitle() const { return m_title.c_str(); }
	inline bool UseWarpDevice() const { return m_useWarpDevice; }

	inline const EngineTimer& GetTimer() const { return m_engineTimer; }

	void ParseCommandLineArgs( _In_reads_(argc) WCHAR* argv[], int argc );

	void CalculateFrameStats();

protected:
	EngineTimer		m_engineTimer;

protected:
	float m_theta = 1.5f*XM_PI;
	float m_phi = XM_PIDIV4;
	float m_radius = 5.0f;

protected:
	UINT32 m_width;
	UINT32 m_height;
	float  m_aspectRatio;

	bool   m_isPaused = false;			// is the application paused?
	bool   m_isMinimized = false;		// is the application minimized?
	bool   m_isMaximized = false;		// is the application maximized?
	bool   m_isResizing = false;		// are the resize bars being dragged?
	bool   m_isFullscreenState = false;	// fullscreen enabled

	bool   m_useWarpDevice;

	POINT  m_lastMousePos;

	std::string m_title;
};