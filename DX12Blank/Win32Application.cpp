#include "stdafx.h"
#include "Win32Application.h"
#include "BaseWindow.h"
#include "StringUtils.h"

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run( BaseWindow* window, HINSTANCE hInstance, int nCmdShow )
{
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW( GetCommandLineW(), &argc );
	window->ParseCommandLineArgs( argv, argc );
	LocalFree( argv );

	// Initialize window class
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	windowClass.lpszClassName = "Crevice 3D DX12";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>( window->GetWidth() ), static_cast<LONG>( window->GetHeight() ) };
	AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		window->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		window );

	// Initialize window. OnInit is defined in each child-implementation of BaseWindow.
	window->OnInit();

	ShowWindow( m_hwnd, nCmdShow );
	UpdateWindow( m_hwnd );

	// Main app loop
	MSG msg = {};
	while( msg.message != WM_QUIT )
	{
		// Process any messages in the queue
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	window->OnDestroy();

	return static_cast<char>( msg.wParam );
}

LRESULT CALLBACK Win32Application::WindowProc( HWND hWnd, UINT32 message, WPARAM wParam, LPARAM lParam )
{
	BaseWindow* window = reinterpret_cast<BaseWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );

	switch( message )
	{
		case WM_CREATE:
		{
			// Save the BaseWindow* passed in to CreateWindow
			LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>( lParam );
			SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( createStruct->lpCreateParams ) );
		}
		return 0;

		case WM_KEYDOWN:
		{
			if( window )
			{
				window->OnKeyDown( static_cast<UINT8>( wParam ) );
			}
		}
		return 0;

		case WM_KEYUP:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			if( window )
			{
				window->OnKeyUp( static_cast<UINT8>( wParam ) );
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			if (window)
			{
				window->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
		}
		return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			if (window)
			{
				window->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			if (window)
			{
				window->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			if (window)
			{
				window->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			}
		}
		return 0;

		case WM_PAINT:
		{
			if( window )
			{
				window->OnUpdate();
				window->OnRender();
			}
		}
		return 0;

		case WM_SIZE:
		{
			RECT clientRect = {};
			::GetClientRect( m_hwnd, &clientRect );

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			// TODO: fix it
			// Resize( width, height );
		}
		return 0;

		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
		}
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc( hWnd, message, wParam, lParam );
}