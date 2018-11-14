#include "stdafx.h"
#include "CreviceWindow.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	CreviceWindow app( "Crevice 3D DX12" );
	return Win32Application::Run( &app, hInstance, nCmdShow );
}