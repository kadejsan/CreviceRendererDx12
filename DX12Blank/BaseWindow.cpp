#include "stdafx.h"
#include "BaseWindow.h"

using namespace std;

BaseWindow::BaseWindow( std::string windowName )
	: m_title( windowName )
	, m_useWarpDevice( false )
	, m_width( 1280 )
	, m_height( 720 )
{
}

BaseWindow::~BaseWindow()
{

}

void BaseWindow::OnUpdate()
{
	m_engineTimer.Tick();
	if (!m_isPaused)
	{
		CalculateFrameStats();
	}
}

void BaseWindow::ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc)
{
	for( int i = 1; i < argc; ++i )
	{
		if( _wcsnicmp( argv[i], L"-warp", wcslen( argv[i] ) ) == 0 || 
			_wcsnicmp( argv[i], L"/warp", wcslen( argv[i] ) ) == 0 )
		{
			m_useWarpDevice = true;
			m_title = m_title + " (WARP)";
		}
		else if( _wcsnicmp( argv[i], L"-width", wcslen( argv[i] ) ) == 0 ||
				 _wcsnicmp( argv[i], L"/width", wcslen( argv[i] ) ) == 0 )
		{
			m_width = _wtoi( argv[i + 1] ); ++i;
		}
		else if( _wcsnicmp( argv[i], L"-height", wcslen( argv[i] ) ) == 0 ||
				 _wcsnicmp( argv[i], L"/height", wcslen( argv[i] ) ) == 0 )
		{
			m_height = _wtoi( argv[i + 1] ); ++i;
		}
	}
}

void BaseWindow::CalculateFrameStats()
{
	// Code computes the average fps and average time
	// to render one frame. Stats are then appended to 
	// the window caption bar.

	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	// Compute averages over one second period
	if ((m_engineTimer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		string fpsStr = to_string(fps);
		string mspfStr = to_string(mspf);

		string windowText = m_title +
			"    fps: " + fpsStr +
			"   mspf: " + mspfStr;

		SetWindowText(Win32Application::GetHwnd(), windowText.c_str());

		// Reset for next average
		frameCount = 0;
		timeElapsed += 1.0f;
	}
}
