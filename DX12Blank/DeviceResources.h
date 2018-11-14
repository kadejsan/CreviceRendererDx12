#pragma once

class BaseWindow;

using namespace Microsoft::WRL;

namespace DX
{
	static const UINT32 st_frameCount = 2; // triple buffering

	class DeviceResources
	{
	public:
		DeviceResources();

		void LoadPipeline( BaseWindow* window );
		UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue);
		void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration);
		void Flush();

	public:
		inline HANDLE GetFenceEvent() const { return m_fenceEvent; }
		inline ComPtr<ID3D12CommandAllocator> GetCommandAllocator(){ return m_commandAllocator[m_frameIndex]; }
		inline ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
		inline ComPtr<ID3D12PipelineState> GetPipelineState() { return m_pipelineState; }
		inline ComPtr<ID3D12Resource> GetCurrentRenderTarget() { return m_renderTargets[m_frameIndex]; }
		inline ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return m_rtvHeap; }
		inline ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }
		inline ComPtr<IDXGISwapChain4> GetSwapChain() { return m_swapChain; }
		inline ComPtr<ID3D12Device2> GetDevice() { return m_device; }

		inline UINT32 GetCurrentFrameIndex() const { return m_frameIndex; }
		inline UINT32 GetRTVDescriptorSize() const { return m_rtvDescriptorSize; }
		
	private:
		void GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
		bool CheckTearingSupport(ComPtr<IDXGIFactory4> factory4);
		void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);

		ComPtr<ID3D12Device2> CreateDevice( ComPtr<IDXGIAdapter1> adapter );
		ComPtr<ID3D12CommandQueue> CreateCommandQueue( ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type );
		ComPtr<IDXGISwapChain4> CreateSwapChain( HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount );
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT32 numDescriptors );
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator( ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type );
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList( ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type );
		
		ComPtr<ID3D12Fence> CreateFence( ComPtr<ID3D12Device2> device );
		HANDLE CreateEventHandle();

	private:
		// Pipeline objects.
		ComPtr<ID3D12Device2>				m_device;
		ComPtr<IDXGISwapChain4>				m_swapChain;
		ComPtr<ID3D12Resource>				m_renderTargets[st_frameCount];
		ComPtr<ID3D12CommandAllocator>		m_commandAllocator[st_frameCount];
		ComPtr<ID3D12CommandQueue>			m_commandQueue;
		ComPtr<ID3D12DescriptorHeap>		m_rtvHeap;
		ComPtr<ID3D12PipelineState>			m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList>	m_commandList;

		// Synchronization objects.
		UINT32								m_rtvDescriptorSize;
		UINT32								m_frameIndex;
		HANDLE								m_fenceEvent;
		ComPtr<ID3D12Fence>					m_fence;
		UINT64								m_fenceValue;

		bool								m_isInitialized;
	};
}