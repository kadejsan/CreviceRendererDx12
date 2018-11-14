#pragma once

#include "BaseWindow.h"
#include "GraphicsDevice.h"

class CreviceWindow : public BaseWindow
{
public:
	CreviceWindow( std::string name );

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

	GraphicsDevice& GetDevice() { assert(m_graphicsDevice != nullptr); return *m_graphicsDevice; }

private:
	GraphicsDevice* m_graphicsDevice;
};