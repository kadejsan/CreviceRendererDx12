#pragma once

#include "BaseWindow.h"
#include "GraphicsDevice.h"
#include "MathHelper.h"
#include "PSOCache.h"
#include "SamplerCache.h"
#include "RenderObject.h"
#include "Renderer.h"
#include "RenderTarget.h"

class CubeMesh;
class Material;

class CreviceWindow : public BaseWindow
{
public:
	CreviceWindow( std::string name );

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

	virtual void OnKeyDown(UINT8 /* key */) override;
	virtual void OnKeyUp(UINT8 /* key */) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	Graphics::GraphicsDevice& GetDevice() { return *Renderer::GetDevice(); }

private:
	PSOCache					m_psoCache;
	SamplerCache				m_samplerCache;

	// Temporary: objects list rendering
	// #TODO: Scene graph
	std::vector<RenderObject>		m_renderObjects;
	void InitializeRenderObjects();
	void InitializeTextures();
	void InitializeMesh();
	void InitializeConstantBuffers();
	void UpdateConstantBuffer(const RenderObject& renderObject);

	struct ObjectConstants
	{
		float4x4 World		   = MathHelper::Identity4x4();
		float4x4 Scene		   = MathHelper::Identity4x4();
		float4x4 WorldViewProj = MathHelper::Identity4x4();
	};
	GPUBuffer*		m_objCB;

	static const int MaxLights = 3;
	struct Light
	{
		float3 LightDirection;
		float  LightRadiance;
	};
	struct ShadingConstants
	{
		float4 EyePosition = float4(0, 0, 0, 0);
		Light  Lights[MaxLights];
		UINT   LightsCount;
		float3 Color;
		float  Roughness;
		float  Metalness;
	};
	GPUBuffer*		m_shadingCB;

	struct SpecularMapFilterConstants
	{
		float Roughness;
	};
	GPUBuffer*		m_specularMapFilterCB;

	enum ETextureType
	{
		ETT_Albedo,
		ETT_Normal,
		ETT_Roughness,
		ETT_Metalness,
		ETT_Max
	};
	Texture2D*		m_textures[ETT_Max];
	enum EModelType
	{
		EMT_Sphere,
		EMT_Cerberus,
		EMT_Max
	};
	RenderObject	m_model[EMT_Max];
	RenderObject	m_skybox;

	Texture2D*		m_envTexture;
	Texture2D*		m_envTextureUnfiltered;
	Texture2D*		m_envTextureEquirect;
	Texture2D*		m_irradianceMap;
	Texture2D*		m_spBRDFLut;

	RenderTarget	m_frameBuffer;
};