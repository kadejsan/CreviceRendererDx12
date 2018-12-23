#pragma once

#include "Include_DX12.h"
#include "GraphicsDevice.h"

using namespace Microsoft::WRL;

namespace Graphics
{
	class GraphicsDevice_DX12 : public GraphicsDevice
	{
	public:
		GraphicsDevice_DX12();
		virtual ~GraphicsDevice_DX12();

		void Initialize( BaseWindow* window ) override;

		// Prepare to render the next frame.
		void MoveToNextFrame();
		
		// Wait for pending GPU work to complete.
		virtual void Flush() override;

	public:
		inline ComPtr<ID3D12Resource> GetDepthStencilBuffer() { return m_depthStencil; }
		inline ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return m_rtvHeap; }
		inline ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return m_dsvHeap; }
		inline ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }
		inline ComPtr<IDXGISwapChain4> GetSwapChain() { return m_swapChain; }
		inline ComPtr<ID3D12Device> GetDevice() { return m_device; }

		inline UINT32 GetRTVDescriptorSize() const { return m_rtvDescriptorSize; }
		inline UINT32 GetDSVDescriptorSize() const { return m_dsvDescriptorSize; }
		inline UINT32 GetCbvSrvUavDescriptorSize() const { return m_cbvSrvUavDescriptorSize; }

		inline FORMAT GetBackBufferFormat() const override { return m_backBufferFormat; }
		inline FORMAT GetDepthStencilFormat() const override { return m_depthStencilFormat; }

		inline HANDLE GetFenceEvent() const { return m_fenceEvent; }

	public:
		// GpuApi interface
		virtual void PresentBegin() override;
		virtual void SetBackBuffer() override;
		virtual void PresentEnd() override;

		virtual void BindViewports(UINT numViewports, const ViewPort *viewports) override;
		virtual void SetScissorRects(UINT numRects, const Rect* rects) override;
		virtual void BindRenderTargets(UINT numViews, Texture2D* const *renderTargets, Texture2D* depthStencilTexture, int arrayIndex = -1) override;

		virtual void ClearRenderTarget(Texture* texture, const FLOAT colorRGBA[4], int arrayIndex = -1) override;
		virtual void ClearDepthStencil(Texture2D* texture, UINT clearFlags, FLOAT depth, UINT8 stencil, int arrayIndex = -1) override;

		virtual void BindVertexBuffers( GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets = nullptr) override;
		virtual void BindIndexBuffer(GPUBuffer* indexBuffer, const FORMAT format, UINT offset) override;
		virtual void BindConstantBuffer(SHADERSTAGE stage, GPUBuffer* buffer, int slot) override;
		virtual void BindGraphicsPSO(GraphicsPSO* pso) override;
		virtual void BindComputePSO(ComputePSO* pso) override;
		virtual void BindResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex = -1) override;
		virtual void BindResources(SHADERSTAGE stage, GPUResource *const* resources, int slot, int count) override;
		virtual void BindUnorderedAccessResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex = -1) override;
		virtual void BindSampler(SHADERSTAGE stage, Sampler* sampler, int slot) override;

		virtual void Draw(int vertexCount, UINT startVertexLocation) override;
		virtual void DrawIndexed(int indexCount, UINT startIndexLocation, UINT baseVertexLocation) override;
		virtual void DrawInstanced(int vertexCount, int instanceCount, UINT startVertexLocation, UINT startInstanceLocation) override;
		virtual void DrawIndexedInstanced(int indexCount, int instanceCount, UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) override;

		virtual void Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) override;

		virtual void CreateBlob(UINT64 byteSize, CPUBuffer* buffer) override;
		virtual void CreateBuffer(const GPUBufferDesc& desc, const SubresourceData* initialData, GPUBuffer* buffer) override;
		virtual void CreateTexture2D(const TextureDesc& desc, const SubresourceData* initialData, Texture2D** texture2D) override;
		virtual void CreateShader(const std::wstring& filename, BaseShader* shader) override;
		virtual void CreateInputLayout(const VertexInputLayoutDesc *inputElementDescs, UINT numElements, VertexLayout *inputLayout) override;
		virtual void CreateGraphicsPSO(const GraphicsPSODesc* pDesc, GraphicsPSO* pso) override;
		virtual void CreateComputePSO(const ComputePSODesc* pDesc, ComputePSO* pso) override;
		virtual void CreateSamplerState(const SamplerDesc *pSamplerDesc, Sampler *pSamplerState) override;

		virtual void TransitionBarrier(GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) override;
		virtual void TransitionBarriers(GPUResource* const* resources, UINT* subresources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) override;

		virtual void UpdateBuffer(GPUBuffer* buffer, const void* data, int dataSize = -1) override;
		virtual void* AllocateFromRingBuffer(GPURingBuffer* buffer, UINT dataSize, UINT& offsetIntoBuffer) override;
		virtual void InvalidateBufferAccess(GPUBuffer* buffer) override;

		virtual void CreateTextureFromFile(const std::string& fileName, Texture2D **ppTexture, bool mipMaps) override;
		
		virtual void GenerateMipmaps(Texture* texture) override;
		virtual void CopyTexture(Texture* dst, Texture* src) override;
		virtual void CopyTextureRegion(Texture* dst, UINT dstMip, UINT dstX, UINT dstY, UINT dstZ, Texture* src, UINT srcMip, UINT arraySlice) override; 
		virtual void CopyBuffer(GPUBuffer* dest, GPUBuffer* src) override;
		virtual void MSAAResolve(Texture2D* dst, Texture2D* src) override;

		virtual void* Map(const GPUBuffer* buffer) override;
		virtual void Unmap(const GPUBuffer* buffer) override;

		virtual void BeginProfilerBlock(const char* name);
		virtual void EndProfilerBlock();
		virtual void SetMarker(const char* name);

		virtual void FlushUI() override;

	private:
		void GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
		bool CheckTearingSupport(ComPtr<IDXGIFactory4> factory4);
		void UpdateRenderTargetViews(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap, UINT n);
		void UpdateDepthStencil(ComPtr<ID3D12Device> device, ComPtr <ID3D12DescriptorHeap> descriptorHeap, UINT backBufferWidth, UINT backBufferHeight);
		void UpdateViewportAndScissor(UINT backBufferWidth, UINT backBufferHeight);

		ComPtr<ID3D12Device> CreateDevice();
		ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
		ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount);
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT32 numDescriptors);
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);
		ComPtr<ID3D12RootSignature>	CreateGraphicsRootSignature(ComPtr<ID3D12Device> device);
		ComPtr<ID3D12RootSignature>	CreateComputeRootSignature(ComPtr<ID3D12Device> device);

		ComPtr<ID3D12Fence> CreateFence( ComPtr<ID3D12Device> device );
		HANDLE CreateEventHandle();

		void CreateNullResources(ComPtr<ID3D12Device> device);
		
		void SetupForDraw();
		void SetupForDispatch();

		inline ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return Frames[m_frameIndex].GetCommandList(); }
		inline UINT64 GetFenceValue() { return Frames[m_frameIndex].GetFenceValue(); }

	private:
		// Pipeline objects.
		ComPtr<IDXGIFactory4>				m_dxgiFactory;
		ComPtr<ID3D12Device>				m_device;
		ComPtr<IDXGISwapChain4>				m_swapChain;
		ComPtr<ID3D12Resource>				m_depthStencil;
		ComPtr<ID3D12CommandQueue>			m_commandQueue;
		ComPtr<ID3D12DescriptorHeap>		m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap>		m_srvUIHeap;

		ComPtr<ID3D12RootSignature>			m_graphicsRootSig;
		ComPtr<ID3D12RootSignature>			m_computeRootSig;

		D3D12_CPU_DESCRIPTOR_HANDLE*		m_nullSampler;
		D3D12_CPU_DESCRIPTOR_HANDLE*		m_nullCBV;
		D3D12_CPU_DESCRIPTOR_HANDLE*		m_nullSRV;
		D3D12_CPU_DESCRIPTOR_HANDLE*		m_nullUAV;

		D3D12_VIEWPORT						m_screenViewport;
		D3D12_RECT							m_scissorRect;

		// Synchronization objects.
		HANDLE								m_fenceEvent;
		ComPtr<ID3D12Fence>					m_fence;
		UINT64								m_currentFence;

		D3D_DRIVER_TYPE						m_driverType = D3D_DRIVER_TYPE_HARDWARE;
		FORMAT								m_backBufferFormat = FORMAT_R8G8B8A8_UNORM;
		FORMAT								m_depthStencilFormat = FORMAT_D24_UNORM_S8_UINT;

		// Descriptors infrastructure
		UINT32								m_rtvDescriptorSize;
		UINT32								m_dsvDescriptorSize;
		UINT32								m_cbvSrvUavDescriptorSize;

		struct FrameResources
		{
			ComPtr<ID3D12Resource>				m_backBuffer;
			ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
			ComPtr<ID3D12GraphicsCommandList>	m_commandList;
			UINT64								m_fenceValue;

			inline ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return m_commandAllocator; }
			inline ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
			inline ComPtr<ID3D12Resource> GetRenderTarget() { return m_backBuffer; }
			inline UINT64 GetFenceValue() { return m_fenceValue; }

			struct DescriptorTableFrameAllocator
			{
				ComPtr<ID3D12DescriptorHeap>	m_heapCPU;
				ComPtr<ID3D12DescriptorHeap>	m_heapGPU;
				UINT							m_descriptorType;
				UINT							m_itemSize;
				UINT							m_itemCount;
				UINT							m_ringOffset;
				bool							m_isDirty[SHADERSTAGE_MAX];
				D3D12_CPU_DESCRIPTOR_HANDLE**	m_boundDescriptors;

				DescriptorTableFrameAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT maxRenameCount);
				~DescriptorTableFrameAllocator();

				void Reset(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE* nullDescriptorsSamplerCBVSRVUAV);
				void Update(SHADERSTAGE stage, UINT slot, D3D12_CPU_DESCRIPTOR_HANDLE* descriptor, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
				void Validate(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
			};
			DescriptorTableFrameAllocator*		ResourceDescriptorsGPU;
			DescriptorTableFrameAllocator*		SamplerDescriptorsGPU;

			struct ResourceFrameAllocator
			{
				ComPtr<ID3D12Resource>	m_resource;
				UINT8*					m_dataBegin;
				UINT8*					m_dataCur;
				UINT8*					m_dataEnd;

				ResourceFrameAllocator(ComPtr<ID3D12Device> device, size_t size);
				~ResourceFrameAllocator();

				UINT8* Allocate(size_t dataSize, size_t alignment);
				void Clear();
				UINT64 CalculateOffset(UINT8* address);
			};
			ResourceFrameAllocator* ResourceBuffer;
		};
		FrameResources Frames[st_frameCount];
		FrameResources& GetFrameResources() { return Frames[m_frameIndex]; }

		struct DescriptorAllocator
		{
			ComPtr< ID3D12DescriptorHeap >	m_heap;
			std::atomic<UINT32>				m_itemCount;
			UINT							m_maxCount;
			UINT							m_itemSize;

			DescriptorAllocator(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT maxCount);
			~DescriptorAllocator();

			UINT64 Allocate();
		};
		DescriptorAllocator*				RTAllocator;
		DescriptorAllocator*				DSAllocator;
		DescriptorAllocator*				ResourceAllocator;
		DescriptorAllocator*				SamplerAllocator;

		// Upload buffer infrastructure
		struct UploadBuffer
		{
			ComPtr< ID3D12Resource >		m_resource;
			UINT8*							m_dataBegin;
			UINT8*							m_dataCurrent;
			UINT8*							m_dataEnd;

			UploadBuffer(ID3D12Device* device, UINT64 size);
			~UploadBuffer();

			UINT8* Allocate(UINT64 dataSize, UINT64 alignment);
			void Clear();
			UINT64 CalculateOffset(UINT8* address);
		};
		UploadBuffer*						BufferUploader;
		UploadBuffer*						TextureUploader;

		ComputePSO*							m_linearDownsamplePSO;
		ComputePSO*							m_gammaDownsamplePSO;
		ComputePSO*							m_arrayDownsamplePSO;

		void InitializeDownsamplePSOs();
		void CreateTexture(UINT width, UINT height, UINT depth, DXGI_FORMAT format, UINT levels);
		void CreateTextureSRV(Texture* texture, D3D12_SRV_DIMENSION dimension, UINT mostDetailedMip, UINT mipLevels);
		void CreateTextureUAV(Texture* texture, UINT mipSlice);
	};
}