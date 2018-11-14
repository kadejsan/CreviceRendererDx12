
#include "stdafx.h"
#include "CreviceWindow.h"
#include "DXHelper.h"

CreviceWindow::CreviceWindow( std::wstring name )
	: BaseWindow( name )
{
}

void CreviceWindow::OnInit()
{
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

	m_deviceResources.LoadPipeline( this );
}

void CreviceWindow::OnUpdate()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 1.0)
	{
		auto fps = frameCounter / elapsedSeconds;
		LOG( "FPS: %f\n", fps );

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}
}

void CreviceWindow::OnRender()
{
	ThrowIfFailed( m_deviceResources.GetCommandAllocator()->Reset() );

	ThrowIfFailed( m_deviceResources.GetCommandList()->Reset( m_deviceResources.GetCommandAllocator().Get(), m_deviceResources.GetPipelineState().Get() ) );

	m_deviceResources.GetCommandList()->ResourceBarrier( 
		1, 
		&CD3DX12_RESOURCE_BARRIER::Transition( 
			m_deviceResources.GetCurrentRenderTarget().Get(), 
			D3D12_RESOURCE_STATE_PRESENT, 
			D3D12_RESOURCE_STATE_RENDER_TARGET ) 
	);

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_deviceResources.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), m_deviceResources.GetCurrentFrameIndex(), m_deviceResources.GetRTVDescriptorSize() );

		// Record commands.
		const float clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		m_deviceResources.GetCommandList()->ClearRenderTargetView( rtvHandle, clearColor, 0, nullptr );
	}

	m_deviceResources.GetCommandList()->ResourceBarrier( 
		1, 
		&CD3DX12_RESOURCE_BARRIER::Transition( 
			m_deviceResources.GetCurrentRenderTarget().Get(), 
			D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_PRESENT ) 
	);

	ThrowIfFailed( m_deviceResources.GetCommandList()->Close() );

	// Execute command list
	ID3D12CommandList* ppCommandLists[] = { m_deviceResources.GetCommandList().Get() };
	m_deviceResources.GetCommandQueue()->ExecuteCommandLists( _countof(ppCommandLists), ppCommandLists );

	// Present
	ThrowIfFailed( m_deviceResources.GetSwapChain()->Present(1, 0) );

	m_deviceResources.Flush();
}

void CreviceWindow::OnDestroy()
{
	m_deviceResources.Flush();

	CloseHandle(m_deviceResources.GetFenceEvent());
}