#pragma once

#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

class BaseWindow;

namespace Graphics
{
	class GraphicsDevice
	{
	protected:
		static const UINT32 st_frameCount = 3; // triple buffering
		static const UINT   st_useVsync = 0;

	public:
		GraphicsDevice()
			: m_isInitialized(false)
			, m_frameIndex(0)
		{}
		virtual ~GraphicsDevice() {};

		virtual void Initialize(BaseWindow* window) = 0;

		virtual void Flush() = 0;

		virtual FORMAT GetBackBufferFormat() const = 0;
		virtual FORMAT GetDepthStencilFormat() const = 0;

		// Gpu Api Interface
		virtual void PresentBegin() = 0;
		virtual void SetBackBuffer() = 0;
		virtual void PresentEnd() = 0;

		virtual void BindViewports(UINT numViewports, const ViewPort *viewports) = 0;
		virtual void SetScissorRects(UINT numRects, const Rect* rects) = 0;
		virtual void BindRenderTargets(UINT numViews, Texture2D* const *renderTargets, Texture2D* depthStencilTexture, int arrayIndex = -1) = 0;

		virtual void ClearRenderTarget(Texture* texture, const FLOAT colorRGBA[4], int arrayIndex = -1) = 0;
		virtual void ClearDepthStencil(Texture2D* texture, UINT clearFlags, FLOAT depth, UINT8 stencil, int arrayIndex = -1) = 0;	

		virtual void BindVertexBuffers(GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets = nullptr) = 0;
		virtual void BindIndexBuffer(GPUBuffer* indexBuffer, const FORMAT format, UINT offset) = 0;
		virtual void BindConstantBuffer(SHADERSTAGE stage, GPUBuffer* buffer, int slot) = 0;
		virtual void BindGraphicsPSO(GraphicsPSO* pso) = 0;
		virtual void BindComputePSO(ComputePSO* pso) = 0;
		virtual void BindResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex = -1) = 0;
		virtual void BindResources(SHADERSTAGE stage, GPUResource *const* resources, int slot, int count) = 0;
		virtual void BindUnorderedAccessResource(GPUResource* resource, int slot, int arrayIndex = -1) = 0;
		virtual void BindSampler(SHADERSTAGE stage, Sampler* sampler, int slot) = 0;

		virtual void Draw(int vertexCount, UINT startVertexLocation) = 0;
		virtual void DrawIndexed(int indexCount, UINT startIndexLocation, UINT baseVertexLocation) = 0;
		virtual void DrawInstanced(int vertexCount, int instanceCount, UINT startVertexLocation, UINT startInstanceLocation) = 0;
		virtual void DrawIndexedInstanced(int indexCount, int instanceCount, UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) = 0;

		virtual void Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) = 0;

		virtual void CreateBlob(UINT64 byteSize, CPUBuffer* buffer) = 0;
		virtual void CreateBuffer(const GPUBufferDesc& desc, const SubresourceData* initialData, GPUBuffer* buffer) = 0;
		virtual void CreateTexture2D(const TextureDesc& desc, const SubresourceData* initialData, Texture2D** texture2D) = 0;
		virtual void CreateShader(const std::wstring& filename, BaseShader* shader) = 0;
		virtual void CreateInputLayout(const VertexInputLayoutDesc *inputElementDescs, UINT numElements, VertexLayout *inputLayout) = 0;
		virtual void CreateGraphicsPSO(const GraphicsPSODesc* pDesc, GraphicsPSO* pso) = 0;
		virtual void CreateComputePSO(const ComputePSODesc* pDesc, ComputePSO* pso) = 0;
		virtual void CreateSamplerState(const SamplerDesc *pSamplerDesc, Sampler *pSamplerState) = 0;

		virtual void TransitionBarrier(GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) = 0;
		virtual void TransitionBarriers(GPUResource* const* resources, UINT* subresources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) = 0;

		virtual void UpdateBuffer(GPUBuffer* buffer, const void* data, int dataSize = -1) = 0;
		virtual void* AllocateFromRingBuffer(GPURingBuffer* buffer, UINT dataSize, UINT& offsetIntoBuffer) = 0;
		virtual void InvalidateBufferAccess(GPUBuffer* buffer) = 0;

		virtual void CreateTextureFromFile(const std::string& fileName, Texture2D **ppTexture, bool mipMaps) = 0;
		
		virtual void GenerateMipmaps(Texture* texture) = 0;
		virtual void CopyTexture(Texture* dst, Texture* src) = 0;
		virtual void CopyTextureRegion(Texture* dst, UINT dstMip, UINT dstX, UINT dstY, UINT dstZ, Texture* src, UINT srcMip, UINT arraySlice) = 0;
		virtual void MSAAResolve(Texture2D* dst, Texture2D* src) = 0;

		virtual void BeginProfilerBlock(const char* name) = 0;
		virtual void EndProfilerBlock() = 0;
		virtual void SetMarker(const char* name) = 0;

		inline bool IsInitialized() const { return m_isInitialized; }
		inline UINT32 GetCurrentFrameIndex() const { return m_frameIndex; }

	protected:
		bool								m_isInitialized;
		UINT32								m_frameIndex;
	};
}