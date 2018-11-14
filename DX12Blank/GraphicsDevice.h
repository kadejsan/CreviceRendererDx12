#pragma once

#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"

class BaseWindow;

namespace GraphicsTypes
{
	class GraphicsDevice
	{
	protected:
		static const UINT32 st_frameCount = 2; // double buffering
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
		virtual void PresentEnd() = 0;

		virtual void BindVertexBuffers(GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets = nullptr) = 0;
		virtual void BindIndexBuffer(GPUBuffer* indexBuffer, const FORMAT format, UINT offset) = 0;
		virtual void BindConstantBuffer(SHADERSTAGE stage, GPUBuffer* buffer, int slot) = 0;
		virtual void BindGraphicsPSO(GraphicsPSO* pso) = 0;
		virtual void BindComputePSO(ComputePSO* pso) = 0;

		virtual void Draw(int vertexCount, UINT startVertexLocation) = 0;
		virtual void DrawIndexed(int indexCount, UINT startIndexLocation, UINT baseVertexLocation) = 0;
		virtual void DrawInstanced(int vertexCount, int instanceCount, UINT startVertexLocation, UINT startInstanceLocation) = 0;
		virtual void DrawIndexedInstanced(int indexCount, int instanceCount, UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) = 0;

		virtual void CreateBlob(UINT byteSize, CPUBuffer* buffer) = 0;
		virtual void CreateBuffer(const GPUBufferDesc& desc, const SubresourceData* initialData, GPUBuffer* buffer) = 0;
		virtual void CreateShader(const std::wstring& filename, BaseShader* shader) = 0;
		virtual void CreateInputLayout(const VertexInputLayoutDesc *inputElementDescs, UINT numElements, VertexLayout *inputLayout) = 0;
		virtual void CreateGraphicsPSO(const GraphicsPSODesc* pDesc, GraphicsPSO* pso) = 0;
		virtual void CreateComputePSO(const ComputePSODesc* pDesc, ComputePSO* pso) = 0;

		virtual void TransitionBarrier(GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) = 0;
		virtual void TransitionBarriers(GPUResource* const* resources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) = 0;

		virtual void UpdateBuffer(GPUBuffer* buffer, const void* data, int dataSize = -1) = 0;

		inline bool IsInitialized() const { return m_isInitialized; }
		inline UINT32 GetCurrentFrameIndex() const { return m_frameIndex; }

	protected:
		bool								m_isInitialized;
		UINT32								m_frameIndex;
	};
}