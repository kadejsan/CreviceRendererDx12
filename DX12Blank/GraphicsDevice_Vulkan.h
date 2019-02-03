#pragma once

#include "Include_Vulkan.h"
#include "GraphicsDevice.h"
#include "ThreadSafeManager.h"

#include "GraphicsDevice_SharedInternals.h"

typedef void* Handle;
#define NULL_HANDLE nullptr

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	int copyFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0 && copyFamily >= 0;
	}
};

namespace Graphics
{
	class GraphicsDevice_Vulkan : public GraphicsDevice
	{
	public:
		GraphicsDevice_Vulkan();

		virtual ~GraphicsDevice_Vulkan();

		virtual void Initialize(BaseWindow* window) override;

		virtual void Flush() override;

		inline FORMAT GetBackBufferFormat() const { return m_backBufferFormat; }
		inline FORMAT GetDepthStencilFormat() const { return m_depthStencilFormat; }

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

		virtual void BindVertexBuffers(GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets = nullptr) override;
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
		VkInstance							m_instance;
		VkDebugReportCallbackEXT			m_callback;
		VkSurfaceKHR						m_surface;
		VkPhysicalDevice					m_physicalDevice = VK_NULL_HANDLE;
		VkDevice							m_device;
		QueueFamilyIndices					m_queueIndices;

		VkQueue								m_graphicsQueue;
		VkQueue								m_presentQueue;

		VkPhysicalDeviceProperties			m_physicalDeviceProperties;

		VkSemaphore							m_imageAvailableSemaphore;
		VkSemaphore							m_renderFinishedSemaphore;

		VkSwapchainKHR						m_swapChain;
		VkFormat							m_swapChainImageFormat;
		VkExtent2D							m_swapChainExtent;
		std::vector<VkImage>				m_swapChainImages;

		VkRenderPass						m_defaultRenderPass;
		VkPipelineLayout					m_defaultPipelineLayout_Graphics;
		VkPipelineLayout					m_defaultPipelineLayout_Compute;
		VkDescriptorSetLayout				m_defaultDescriptorSetlayouts[SHADERSTAGE_MAX];
		uint32_t							m_descriptorCount;

		VkBuffer							m_nullBuffer;
		VkBufferView						m_nullBufferView;
		VkImage								m_nullImage;
		VkImageView							m_nullImageView;
		VkSampler							m_nullSampler;

		struct RenderPassManager
		{
			bool dirty = true;

			VkImageView attachments[9] = {};
			uint32_t attachmentCount = 0;
			VkExtent2D attachmentsExtents = {};
			uint32_t attachmentLayers = 0;
			VkClearValue clearColor[9] = {};

			struct RenderPassAndFramebuffer
			{
				VkRenderPass renderPass = VK_NULL_HANDLE;
				VkFramebuffer frameBuffer = VK_NULL_HANDLE;
			};
			// RTFormats hash <-> renderpass+framebuffer
			std::unordered_map<uint64_t, RenderPassAndFramebuffer> renderPassCollection;
			uint64_t activeRTHash = 0;
			GraphicsPSODesc* pDesc = nullptr;

			VkRenderPass overrideRenderPass = VK_NULL_HANDLE;
			VkFramebuffer overrideFramebuffer = VK_NULL_HANDLE;

			struct ClearRequest
			{
				VkImageView attachment = VK_NULL_HANDLE;
				VkClearValue clearValue = {};
				uint32_t clearFlags = 0;
			};
			std::vector<ClearRequest> clearRequests;

			void reset();
			void disable(VkCommandBuffer commandBuffer);
			void validate(VkDevice device, VkCommandBuffer commandBuffer);
		};
		RenderPassManager renderPass;


		struct FrameResources
		{
			VkFence frameFence;
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
			VkImageView swapChainImageView;
			VkFramebuffer swapChainFramebuffer;

			struct DescriptorTableFrameAllocator
			{
				GraphicsDevice_Vulkan* device;
				VkDescriptorPool descriptorPool;
				VkDescriptorSet descriptorSet_CPU[SHADERSTAGE_MAX];
				std::vector<VkDescriptorSet> descriptorSet_GPU[SHADERSTAGE_MAX];
				UINT ringOffset[SHADERSTAGE_MAX];
				bool dirty[SHADERSTAGE_MAX];

				// default descriptor table contents:
				VkDescriptorBufferInfo bufferInfo[GPU_RESOURCE_HEAP_SRV_COUNT] = {};
				VkDescriptorImageInfo imageInfo[GPU_RESOURCE_HEAP_SRV_COUNT] = {};
				VkBufferView bufferViews[GPU_RESOURCE_HEAP_SRV_COUNT] = {};
				VkDescriptorImageInfo samplerInfo[GPU_SAMPLER_HEAP_COUNT] = {};
				std::vector<VkWriteDescriptorSet> initWrites[SHADERSTAGE_MAX];

				// descriptor table rename guards:
				std::vector<Handle> boundDescriptors[SHADERSTAGE_MAX];

				DescriptorTableFrameAllocator(GraphicsDevice_Vulkan* device, UINT maxRenameCount);
				~DescriptorTableFrameAllocator();

				void reset();
				void update(SHADERSTAGE stage, UINT slot, VkBuffer descriptor, VkCommandBuffer commandList);
				void validate(VkCommandBuffer commandList);
			};
			DescriptorTableFrameAllocator*		ResourceDescriptorsGPU;


			struct ResourceFrameAllocator
			{
				VkDevice				device;
				VkBuffer				resource;
				VkDeviceMemory			resourceMemory;
				uint8_t*				dataBegin;
				uint8_t*				dataCur;
				uint8_t*				dataEnd;

				ResourceFrameAllocator(VkPhysicalDevice physicalDevice, VkDevice device, size_t size);
				~ResourceFrameAllocator();

				uint8_t* allocate(size_t dataSize, size_t alignment);
				void clear();
				uint64_t calculateOffset(uint8_t* address);
			};
			ResourceFrameAllocator* resourceBuffer;
		};
		FrameResources Frames[st_frameCount];
		inline FrameResources& GetFrameResources() { return Frames[m_frameIndex]; }
		inline VkCommandBuffer GetCommandList() { return Frames[m_frameIndex].commandBuffer; }

		struct UploadBuffer : ThreadSafeManager
		{
			VkDevice				device;
			VkBuffer				resource;
			VkDeviceMemory			resourceMemory;
			uint8_t*				dataBegin;
			uint8_t*				dataCur;
			uint8_t*				dataEnd;

			UploadBuffer(VkPhysicalDevice physicalDevice, VkDevice device, const QueueFamilyIndices& queueIndices, size_t size);
			~UploadBuffer();

			uint8_t* allocate(size_t dataSize, size_t alignment);
			void clear();
			uint64_t calculateOffset(uint8_t* address);
		};
		UploadBuffer* bufferUploader;
		UploadBuffer* textureUploader;

	private:
		bool								m_fullscreen;
		UINT								m_screenWidth;
		UINT								m_screenHeight;

		FORMAT								m_backBufferFormat = FORMAT_R8G8B8A8_UNORM;
		FORMAT								m_depthStencilFormat = FORMAT_D24_UNORM_S8_UINT;
	};
}