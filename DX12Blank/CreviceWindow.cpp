
#include "stdafx.h"
#include "CreviceWindow.h"
#include "DXHelper.h"
#include "GraphicsDevice_DX12.h"

CreviceWindow::CreviceWindow( std::string name )
	: BaseWindow( name )
{
}

void CreviceWindow::OnInit()
{
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
	
	m_engineTimer.Reset();

	m_graphicsDevice = new DX::GraphicsDevice_DX12();
		
	GetDevice().Initialize( this );
}

void CreviceWindow::OnUpdate()
{
	BaseWindow::OnUpdate();
}

void CreviceWindow::OnRender()
{
	GetDevice().PresentBegin();

	// Render stuff...

	GetDevice().PresentEnd();
}

void CreviceWindow::OnDestroy()
{
	GetDevice().Flush();

	CloseHandle( GetDevice().GetFenceEvent() );

	delete m_graphicsDevice;
}