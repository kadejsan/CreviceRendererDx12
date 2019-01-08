#pragma once

#include "BaseWindow.h"
#include "GraphicsDevice.h"
#include "MathHelper.h"
#include "PSOCache.h"
#include "SamplerCache.h"
#include "RenderObject.h"
#include "Grid.h"
#include "Gizmo.h"
#include "Renderer.h"
#include "RenderTarget.h"

class CubeMesh;
class Material;

class CreviceWindow : public BaseWindow
{
public:
	static const UINT st_objectsOffset = 5;

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

	void UpdateHDRSkybox();

	Graphics::GraphicsDevice& GetDevice() { return *Renderer::GetDevice(); }
	Renderer&				  GetRenderer() { assert(m_renderer != nullptr); return *m_renderer; }

private:
	Renderer*					m_renderer;

	int							m_skyboxID;
	RenderObject				m_skybox;

	// Temporary: objects list rendering
	// #TODO: Scene graph
	std::vector<RenderObject>		m_renderObjects;
	void InitializeRenderObjects();
	void InitializeTextures();
	void InitializeMesh();
	void InitializeConstantBuffers();
	void UpdateGlobalConstantBuffer();
	void UpdateObjectConstantBuffer(const RenderObject& renderObject, UINT id);

	struct ObjectConstantsVS
	{
		float4x4 World		   = MathHelper::Identity4x4();
		float4x4 Scene		   = MathHelper::Identity4x4();
		float4x4 WorldViewProj = MathHelper::Identity4x4();
	};
	GPUBuffer*		m_objVsCB;

	struct ObjectConstantsPS
	{
		float3 Color;
		float  Roughness;
		float  Metalness;
		UINT   ObjectID;
	};
	GPUBuffer*		m_objPsCB;

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
		float2 MousePos;
	};
	GPUBuffer*		m_shadingCB;

	struct BackgroundConstants
	{
		float4x4 ScreenToWorld = MathHelper::Identity4x4();
		float4x4 CubemapRotation = MathHelper::Identity4x4();
		float4   ScreenDimensions;
	};
	GPUBuffer*		m_backgroundCB;

	Texture2D*		m_textures[ETT_Max];
	enum EModelType
	{
		EMT_Sphere,
		EMT_Cerberus,
		EMT_Max
	};
	RenderObject	m_model[EMT_Max];
	Grid			m_grid;
	Gizmo			m_gizmo;

	bool			m_readHitProxy;
	int				m_hitProxyID;
};