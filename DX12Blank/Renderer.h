#pragma once

#include "GraphicsDevice.h"
#include "GraphicsEnums.h"
#include "PSOCache.h"
#include "SamplerCache.h"
#include "RenderTarget.h"

class Renderer
{
public:
	Renderer(GpuAPI gpuApi, BaseWindow* window);
	~Renderer();
	
	static Graphics::GraphicsDevice* GGraphicsDevice;
	static Graphics::GraphicsDevice* GetDevice() { assert(GGraphicsDevice != nullptr);  return GGraphicsDevice; }

	PSOCache					  m_psoCache;
	SamplerCache				  m_samplerCache;

	inline Graphics::GraphicsPSO* GetPSO(eGPSO pso, bool wireframe = false) const { return m_psoCache.GetPSO((eGPSO)(pso + (wireframe ? 1 : 0))); }
	inline Graphics::ComputePSO* GetPSO(eCPSO pso) const { return m_psoCache.GetPSO(pso); }

	inline Graphics::Sampler* GetSamplerState(eSamplerState state) const { return m_samplerCache.GetSamplerState(state); }

	inline void SetFrameBuffer(bool set = true) { set ? m_frameBuffer.Activate() : m_frameBuffer.Deactivate(); }

	void InitializeIBLTextures(const std::string& name);

	void BindIBL();
	void BindEnvTexture(SHADERSTAGE stage, int slot);

	void RenderBackground();
	void DoPostProcess();

private:
	void InitializeConstantBuffers();

	RenderTarget				  m_frameBuffer;

	Texture2D*					  m_envTexture;
	Texture2D*					  m_envTextureEquirect;
	Texture2D*					  m_envTextureUnfiltered;
	Texture2D*					  m_irradianceMap;
	Texture2D*					  m_spBRDFLut;

	struct SpecularMapFilterConstants
	{
		float Roughness;
	};
	GPUBuffer*					  m_specularMapFilterCB;

public:
	static const Graphics::FORMAT RTFormat_LDR = Graphics::FORMAT_R8G8B8A8_UNORM;
	static const Graphics::FORMAT RTFormat_HDR = Graphics::FORMAT_R16G16B16A16_FLOAT;
	static const Graphics::FORMAT RTFormat_DepthResolve = Graphics::FORMAT_R32_FLOAT;

	static const Graphics::FORMAT DSFormat_Full = Graphics::FORMAT_D24_UNORM_S8_UINT;
	static const Graphics::FORMAT DSFormat_FullAlias = Graphics::FORMAT_R24G8_TYPELESS;
	static const Graphics::FORMAT DSFormat_Small = Graphics::FORMAT_D16_UNORM;
	static const Graphics::FORMAT DSFormat_SmallAlias = Graphics::FORMAT_R16_TYPELESS;
};