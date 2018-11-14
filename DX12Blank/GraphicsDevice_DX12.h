#pragma once

#include "GraphicsDevice.h"

class GPUResource;

using namespace Microsoft::WRL;

namespace DX
{
	class GraphicsDevice_DX12 : public GraphicsDevice
	{
	public:
		GraphicsDevice_DX12();
		virtual ~GraphicsDevice_DX12();

		void Initialize( BaseWindow* window ) override;
		UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue);
		void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration);
		virtual void Flush() override;

	public:
		inline ComPtr<ID3D12CommandAllocator> GetCommandAllocator(){ return m_commandAllocator[m_frameIndex]; }
		inline ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
		inline ComPtr<ID3D12PipelineState> GetPipelineState() { return m_pipelineState; }
		inline ComPtr<ID3D12Resource> GetCurrentRenderTarget() { return m_renderTargets[m_frameIndex]; }
		inline ComPtr<ID3D12Resource> GetDepthStencilBuffer() { return m_depthStencil; }
		inline ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return m_rtvHeap; }
		inline ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return m_dsvHeap; }
		inline ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }
		inline ComPtr<IDXGISwapChain4> GetSwapChain() { return m_swapChain; }
		inline ComPtr<ID3D12Device> GetDevice() { return m_device; }

		inline UINT32 GetRTVDescriptorSize() const { return m_rtvDescriptorSize; }
		inline UINT32 GetDSVDescriptorSize() const { return m_dsvDescriptorSize; }
		inline UINT32 GetCbvSrvUavDescriptorSize() const { return m_cbvSrvUavDescriptorSize; }

		virtual HANDLE GetFenceEvent() const override { return m_fenceEvent; }

	public:
		// GpuApi interface
		virtual void PresentBegin() override;
		virtual void PresentEnd() override;

		virtual void TransitionBarrier(GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) override;
		virtual void TransitionBarriers(GPUResource* const* resources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) override;

	private:
		void GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
		bool CheckTearingSupport(ComPtr<IDXGIFactory4> factory4);
		void UpdateRenderTargetViews(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
		void UpdateDepthStencil(ComPtr<ID3D12Device> device, ComPtr <ID3D12DescriptorHeap> descriptorHeap, UINT backBufferWidth, UINT backBufferHeight);
		void UpdateViewportAndScissor(UINT backBufferWidth, UINT backBufferHeight);

		ComPtr<ID3D12Device> CreateDevice();
		ComPtr<ID3D12CommandQueue> CreateCommandQueue( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type );
		ComPtr<IDXGISwapChain4> CreateSwapChain( HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount );
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT32 numDescriptors );
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type );
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList( ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type );
		
		ComPtr<ID3D12Fence> CreateFence( ComPtr<ID3D12Device> device );
		HANDLE CreateEventHandle();
		
	private:
		// Pipeline objects.
		ComPtr<IDXGIFactory4>				m_dxgiFactory;
		ComPtr<ID3D12Device>				m_device;
		ComPtr<IDXGISwapChain4>				m_swapChain;
		ComPtr<ID3D12Resource>				m_renderTargets[st_frameCount];
		ComPtr<ID3D12Resource>				m_depthStencil;
		ComPtr<ID3D12CommandAllocator>		m_commandAllocator[st_frameCount];
		ComPtr<ID3D12CommandQueue>			m_commandQueue;
		ComPtr<ID3D12DescriptorHeap>		m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
		ComPtr<ID3D12PipelineState>			m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList>	m_commandList;

		D3D12_VIEWPORT						m_screenViewport;
		D3D12_RECT							m_scissorRect;

		// Synchronization objects.
		UINT32								m_rtvDescriptorSize;
		UINT32								m_dsvDescriptorSize;
		UINT32								m_cbvSrvUavDescriptorSize;
		HANDLE								m_fenceEvent;
		ComPtr<ID3D12Fence>					m_fence;
		UINT64								m_fenceValue;

		D3D_DRIVER_TYPE						m_driverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT							m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT							m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	};
}