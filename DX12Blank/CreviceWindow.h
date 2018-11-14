#pragma once

#include "BaseWindow.h"
#include "DeviceResources.h"

class CreviceWindow : public BaseWindow
{
public:
	CreviceWindow( std::wstring name );

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	DX::DeviceResources m_deviceResources;
};