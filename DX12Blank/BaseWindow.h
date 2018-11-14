#pragma once

#include "Win32Application.h"

class BaseWindow
{
public:
	BaseWindow( std::wstring windowName );
	virtual ~BaseWindow();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	virtual void OnKeyDown( UINT8 /* key */ ) {}
	virtual void OnKeyUp( UINT8 /* key */ ) {}

	UINT32 GetWidth() const { return m_width; }
	UINT32 GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }
	bool UseWarpDevice() const { return m_useWarpDevice; }

	void ParseCommandLineArgs( _In_reads_(argc) WCHAR* argv[], int argc );

protected:
	UINT32 m_width;
	UINT32 m_height;
	float  m_aspectRatio;

	bool m_useWarpDevice;

private:
	std::wstring m_title;
};