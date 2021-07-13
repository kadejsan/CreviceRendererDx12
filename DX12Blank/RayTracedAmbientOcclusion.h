#pragma once

#include "RenderTarget.h"
#include "Camera.h"

class Renderer;

class RayTracedAmbientOcclusion
{
public:
	RayTracedAmbientOcclusion();
	~RayTracedAmbientOcclusion();

	void Initialize(UINT width, UINT height);

	void UpdateConstants(Renderer* renderer, const Camera& camera);
	void ComputeAO(Renderer* renderer);
	void LowPassFilter(Renderer* renderer, const Camera& camera);

	inline Texture2D* GetAO() const { return m_finalAO.GetTexture(0); }

private:
	RenderTarget	m_ao[eFrame::Max];
	RenderTarget	m_tempAO;
	RenderTarget	m_finalAO;
	GPUBuffer*		m_sampleKernel;

	struct RTAOViewConstants
	{
		float4x4 View;
		float4x4 PrevView;
		float4x4 PrevViewProj;
		float4	 EyePos;
		float4	 ResolutionTanHalfFovYAndAspectRatio;
		float2	 CameraNearFar;
	};
	GPUBuffer*		m_viewCB;

	struct RTAOConstants 
	{
		float  AORadius;
		float  AOStrength;
		int	   FrameNo;
		int    SampleCount;
		int	   SampleStartIndex;
	};
	GPUBuffer*		m_rtaoCB;

	struct LowPassFilterConstants
	{
		float4x4 InvView;
		float2	 TexelSize;
		float2	 FilterDirection;
		int		 DoTemporalFilter;
	};
	GPUBuffer* m_lowPassFilterCB;

private:
	UINT			m_sampleKernelSize;
	UINT			m_frameNo;
};