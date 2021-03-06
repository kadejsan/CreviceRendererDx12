#include "stdafx.h"
#include "BaseWindow.h"
#include "MathHelper.h"
#include "Camera.h"

using namespace std;

BaseWindow::BaseWindow( std::string windowName )
	: m_title( windowName )
	, m_useWarpDevice( false )
	, m_width( 1280 )
	, m_height( 720 )
{
	m_camera = new CameraFree(AspectRatio());
	m_camera->Update();
}

BaseWindow::~BaseWindow()
{
	delete m_camera;
}

void BaseWindow::OnUpdate()
{
	m_engineTimer.Tick();
	m_camera->Update();
	if (!m_isPaused)
	{
		CalculateFrameStats();
	}
}

void BaseWindow::OnKeyDown(UINT8 key)
{
	if (GetCamera()->GetType() == Free)
	{
		CameraFree* camera = static_cast<CameraFree*>(m_camera);
		if (key == 'W')
		{
			camera->TranslateZ(0.1f);
		}
		else if (key == 'S')
		{
			camera->TranslateZ(-0.1f);
		}
		if (key == 'A')
		{
			camera->TranslateX(-0.1f);
		}
		else if (key == 'D')
		{
			camera->TranslateX(0.1f);
		}
	}
}

void BaseWindow::OnKeyUp(UINT8 key)
{
	if (GetCamera()->GetType() == Free)
	{
		CameraFree* camera = static_cast<CameraFree*>(m_camera);
		if (key == 'W')
		{
			camera->TranslateZ(0.0f);
		}
		else if (key == 'S')
		{
			camera->TranslateZ(0.0f);
		}
		if (key == 'A')
		{
			camera->TranslateX(0.0f);
		}
		else if (key == 'D')
		{
			camera->TranslateX(0.0f);
		}
	}
}

void BaseWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	SetCapture(Win32Application::GetHwnd());
}

void BaseWindow::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BaseWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (GetCamera()->GetType() == ArcBall)
	{
		CameraArcBall* camera = GetCameraArcBall();
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
			float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));

			// Update angles based on input to orbit camera around box.
			camera->SetTheta(dx);
			camera->SetPhi(dy);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			// Make each pixel correspond to 0.005 unit in the scene.
			float dx = 0.005f*static_cast<float>(x - m_lastMousePos.x);
			float dy = 0.005f*static_cast<float>(y - m_lastMousePos.y);

			// Update the camera radius based on input.
			camera->SetRadius(dx - dy);
		}
	}
	else if (GetCamera()->GetType() == Free)
	{
		CameraFree* camera = GetCameraFree();
		if ((btnState & MK_RBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
			float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));

			camera->Rotate(dy, dx);
		}
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

void BaseWindow::OnMouseWheel(float delta)
{
	if (GetCamera()->GetType() == ArcBall)
	{
		// Update the camera radius based on input.
		GetCameraArcBall()->SetRadius(-0.005f*delta);
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
