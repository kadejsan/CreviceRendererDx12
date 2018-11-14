#pragma once

class BaseWindow;

class Win32Application
{
public:
	static int Run(BaseWindow* window, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT32 message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};