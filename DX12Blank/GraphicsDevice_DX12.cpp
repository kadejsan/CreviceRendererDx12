#include "stdafx.h"
#include "DXHelper.h"
#include "BaseWindow.h"
#include "GraphicsDevice_DX12.h"
#include "GraphicsResource.h"

namespace DX
{
	inline D3D12_RESOURCE_STATES ConvertResourceStates(RESOURCE_STATES value)
	{
		return static_cast<D3D12_RESOURCE_STATES>(value);
	}

	GraphicsDevice_DX12::GraphicsDevice_DX12()
	{
	}

	GraphicsDevice_DX12::~GraphicsDevice_DX12()
	{
		Flush();
	}

	void GraphicsDevice_DX12::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		UINT32 destAdapterIndex = 0;
		UINT64 videoMemory = 0;
		for (UINT32 adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				LOG( "%ls", desc.Description );
				if (desc.DedicatedVideoMemory > videoMemory)
				{
					videoMemory = desc.DedicatedVideoMemory;
					destAdapterIndex = adapterIndex;
				}
			}
		}

		pFactory->EnumAdapters1(0, ppAdapter);
	}

	bool GraphicsDevice_DX12::CheckTearingSupport(ComPtr<IDXGIFactory4> factory4)
	{
		bool allowTearing = false;

		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = false;
			}
		}

		return allowTearing;
	}

	ComPtr<ID3D12Device> GraphicsDevice_DX12::CreateDevice()
	{
		// Create the D3D graphics device
		ComPtr<IDXGIAdapter1> pAdapter;
		ComPtr<ID3D12Device> pDevice;

		DXGI_ADAPTER_DESC1 adapterDesc = {};
		SIZE_T maxSize = 0;

		for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (desc.DedicatedVideoMemory > maxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice))))
			{
				pAdapter->GetDesc1(&desc);
				maxSize = desc.DedicatedVideoMemory;
				adapterDesc = desc;
			}
		}

		if (maxSize > 0)
		{
			LOG("GpuApi: D3D12-capable hardware found!");
			LOG("GpuApi: Adapter details:");
			LOG("GpuApi:   Description: %ls", adapterDesc.Description);
			LOG("GpuApi:   Vendor ID: %u", adapterDesc.VendorId);
			LOG("GpuApi:   Device ID: %u", adapterDesc.DeviceId);
			LOG("GpuApi:   SubSys ID: %u", adapterDesc.SubSysId);
			LOG("GpuApi:   Revision: %u", adapterDesc.Revision);
			LOG("GpuApi:   Dedicated Video Memory: %uMB", UINT32(adapterDesc.DedicatedVideoMemory / size_t(1024 * 1024)));
			LOG("GpuApi:   Dedicated System Memory: %uMB", UINT32(adapterDesc.DedicatedSystemMemory / size_t(1024 * 1024)));
			LOG("GpuApi:   Shared System Memory: %uMB", UINT32(adapterDesc.SharedSystemMemory / size_t(1024 * 1024)));
		}
		else
		{
			LOG("GpuApi: Unable to create Device.");
			return nullptr;
		}

		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		{
			bool DeveloperModeEnabled = false;

			// Look in the Windows Registry to determine if Developer Mode is enabled
			HKEY hKey;
			LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
			if (result == ERROR_SUCCESS)
			{
				DWORD keyValue, keySize = sizeof(DWORD);
				result = RegQueryValueEx(hKey, "AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
				if (result == ERROR_SUCCESS && keyValue == 1)
					DeveloperModeEnabled = true;
				RegCloseKey(hKey);
			}

			if (!DeveloperModeEnabled)
			{
				LOG("Enable Developer Mode on Windows 10 to get consistent profiling results");
			}

			// Prevent the GPU from overclocking or underclocking to get consistent timings
			if (DeveloperModeEnabled)
				pDevice->SetStablePowerState(TRUE);
		}

		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(pDevice.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				// I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

				// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,

				// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,

				// This occurs when there are uninitialized descriptors in a descriptor table, even when a
				// shader does not access the missing descriptors.  I find this is common when switching
				// shader permutations and not wanting to change much code to reorder resources.
				D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

				// Triggered when a shader does not export all color components of a render target, such as
				// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
				D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

				// This occurs when a descriptor table is unbound even when a shader does not access the missing
				// descriptors.  This is common with a root signature shared between disparate shaders that
				// don't all need the same types of resources.
				D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

				// RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
				(D3D12_MESSAGE_ID)1008,
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
		}
#endif

		return pDevice.Detach();
	}

	ComPtr<ID3D12CommandQueue> GraphicsDevice_DX12::CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

		return d3d12CommandQueue;
	}

	ComPtr<IDXGISwapChain4> GraphicsDevice_DX12::CreateSwapChain(HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount)
	{
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = CheckTearingSupport(factory) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
		ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

		return dxgiSwapChain4;
	}

	ComPtr<ID3D12DescriptorHeap> GraphicsDevice_DX12::CreateDescriptorHeap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT32 numDescriptors)
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = flags;

		ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

		return descriptorHeap;
	}

	ComPtr<ID3D12CommandAllocator> GraphicsDevice_DX12::CreateCommandAllocator(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList> GraphicsDevice_DX12::CreateCommandList(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
	{
		ComPtr<ID3D12GraphicsCommandList> commandList;
		ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

		ThrowIfFailed(commandList->Close());

		return commandList;
	}

	Microsoft::WRL::ComPtr<ID3D12Fence> GraphicsDevice_DX12::CreateFence(ComPtr<ID3D12Device> device)
	{
		ComPtr<ID3D12Fence> fence;

		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		return fence;
	}

	HANDLE GraphicsDevice_DX12::CreateEventHandle()
	{
		HANDLE fenceEvent;

		fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent && "Failed to create fence event.");

		return fenceEvent;
	}

	void GraphicsDevice_DX12::UpdateRenderTargetViews(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr <ID3D12DescriptorHeap> descriptorHeap)
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame
		for (UINT32 n = 0; n < st_frameCount; ++n)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	void GraphicsDevice_DX12::UpdateDepthStencil(ComPtr<ID3D12Device> device, ComPtr <ID3D12DescriptorHeap> descriptorHeap, UINT backBufferWidth, UINT backBufferHeight)
	{
		if (m_depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			// Allocate 2-D surface as the depth/stencil buffer and create a depth/stencil view on this surface
			CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( m_depthStencilFormat, backBufferWidth, backBufferHeight, 1, 1 );
			
			depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = m_depthStencilFormat;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;

			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(m_depthStencil.GetAddressOf())));

			// Create descriptor to mip level 0 of entire resource using the format of the resource
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = m_depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	void GraphicsDevice_DX12::UpdateViewportAndScissor(UINT backBufferWidth, UINT backBufferHeight)
	{
		// Update the viewport transform to cover the client area.
		m_screenViewport.TopLeftX = 0;
		m_screenViewport.TopLeftY = 0;
		m_screenViewport.Width = static_cast<float>(backBufferWidth);
		m_screenViewport.Height = static_cast<float>(backBufferHeight);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;

		m_scissorRect = { 0, 0, (LONG)backBufferWidth, (LONG)backBufferHeight };
	}

	void GraphicsDevice_DX12::Initialize(BaseWindow* window)
	{
		// Load rendering pipeline dependencies
		UINT32 dxgiFactoryFlags = 0;

#ifdef _DEBUG
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

		if (window->UseWarpDevice())
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
		}
		else
		{
			m_device = CreateDevice();
			m_commandQueue = CreateCommandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_swapChain = CreateSwapChain(Win32Application::GetHwnd(), m_dxgiFactory, m_commandQueue, window->GetWidth(), window->GetHeight(), st_frameCount);
			m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
			
			m_rtvHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, st_frameCount);
			m_dsvHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1);
			
			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			
			for (int i = 0; i < st_frameCount; ++i)
				m_commandAllocator[i] = CreateCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_commandList = CreateCommandList(m_device, m_commandAllocator[m_frameIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

			UpdateRenderTargetViews(m_device, m_swapChain, m_rtvHeap);
			UpdateDepthStencil(m_device, m_dsvHeap, window->GetWidth(), window->GetHeight());
			UpdateViewportAndScissor(window->GetWidth(), window->GetHeight());

			m_fence = CreateFence(m_device);
			m_fenceEvent = CreateEventHandle();

			m_isInitialized = true;
		}
	}

	UINT64 GraphicsDevice_DX12::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue)
	{
		uint64_t fenceValueForSignal = ++fenceValue;
		ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

		return fenceValueForSignal;
	}

	void GraphicsDevice_DX12::WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration)
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
		}
	}

	void GraphicsDevice_DX12::Flush()
	{
		uint64_t fenceValueForSignal = Signal(m_commandQueue, m_fence, m_fenceValue);
		WaitForFenceValue(m_fence, fenceValueForSignal, m_fenceEvent, std::chrono::milliseconds::max());
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::PresentBegin()
	{
		ThrowIfFailed(GetCommandAllocator()->Reset());

		ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), GetPipelineState().Get()));

		GetCommandList()->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentRenderTarget().Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		{
			// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
			GetCommandList()->RSSetViewports(1, &m_screenViewport);
			GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
		}

		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), GetCurrentFrameIndex(), GetRTVDescriptorSize());

			// Record commands.
			const float clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
			GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			GetCommandList()->ClearDepthStencilView(GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}

		GetCommandList()->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentRenderTarget().Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT));
	}

	void GraphicsDevice_DX12::PresentEnd()
	{
		ThrowIfFailed(GetCommandList()->Close());

		// Execute command list
		ID3D12CommandList* ppCommandLists[] = { GetCommandList().Get() };
		GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present
		ThrowIfFailed(GetSwapChain()->Present(st_useVsync, 0));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		Flush();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_DX12::TransitionBarrier(GPUResource* resource, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{
		if (resource != nullptr)
		{
			GetCommandList()->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					resource->m_resourceDX12.Get(),
					ConvertResourceStates(stateBefore),
					ConvertResourceStates(stateAfter)));
		}
	}

	void GraphicsDevice_DX12::TransitionBarriers(GPUResource* const* resources, UINT numBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{
		if (resources != nullptr)
		{
			D3D12_RESOURCE_BARRIER barriers[8];
			for (UINT i = 0; i < numBarriers; ++i)
			{
				barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[i].Transition.pResource = resources[i]->m_resourceDX12.Get();
				barriers[i].Transition.StateBefore = ConvertResourceStates(stateBefore);
				barriers[i].Transition.StateAfter = ConvertResourceStates(stateAfter);
				barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			GetCommandList()->ResourceBarrier(numBarriers, barriers);
		}

	}
}