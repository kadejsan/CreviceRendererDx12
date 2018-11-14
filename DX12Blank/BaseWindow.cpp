#include "stdafx.h"
#include "BaseWindow.h"

BaseWindow::BaseWindow( std::wstring windowName )
	: m_title( windowName )
	, m_useWarpDevice( false )
{
}

BaseWindow::~BaseWindow()
{

}

void BaseWindow::ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc)
{
	for( int i = 1; i < argc; ++i )
	{
		if( _wcsnicmp( argv[i], L"-warp", wcslen( argv[i] ) ) == 0 || 
			_wcsnicmp( argv[i], L"/warp", wcslen( argv[i] ) ) == 0 )
		{
			m_useWarpDevice = true;
			m_title = m_title + L" (WARP)";
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