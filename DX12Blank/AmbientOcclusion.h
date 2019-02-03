#pragma once

#include "RenderTarget.h"
#include "Camera.h"

class Renderer;

class AmbientOcclusion
{
public:
	AmbientOcclusion();
	~AmbientOcclusion();

	void Initialize(UINT width, UINT height);

	void UpdateConstants(Renderer* renderer, const Camera& camera);
	void ComputeAO(Renderer* renderer);

	inline Texture2D* GetAO() const { return m_ao.GetTexture(0); }

private:
	RenderTarget	m_ao;
	GPUBuffer*		m_sampleKernel;
	Texture2D*		m_noiseTexture;

	struct AoVSConstants
	{
		float4x4 View;
		float4x4 Proj;
		float4x4 InvProj;
	};
	GPUBuffer*		m_vsCB;

	struct AoPSConstants 
	{
		float ScreenWidth;
		float ScreenHeight;
		UINT  SampleKernelSize;
		float NoiseTextureDimension;
		float OcclusionRadius;
		float OcclusionPower;
		float OcclusionFalloff;
		float OcclusionDarkness;
		float CamerNear;
		float CameraFar;
		float CameraNearInv;
		float CameraFarInv;
	};
	GPUBuffer*		m_psCB;

private:
	UINT			m_sampleKernelSize;
	UINT			m_noiseTextureDimension;
};