#pragma once

#include "BaseWindow.h"
#include "GraphicsDevice.h"
#include "MathHelper.h"

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

	GraphicsTypes::GraphicsDevice& GetDevice() { assert(m_graphicsDevice != nullptr); return *m_graphicsDevice; }

private:
	GraphicsTypes::GraphicsDevice* m_graphicsDevice;

	// Temporary: single object rendering
	// #TODO: Scene graph
	void InitializeRenderObject();
	void UpdateConstantBuffer();

	struct ObjectConstants
	{
		float4x4 WorldViewProj = MathHelper::Identity4x4();
	};

	struct RenderObject
	{
	public:
		RenderObject()
			: m_mesh(nullptr)
			, m_pso(nullptr)
		{}

		~RenderObject();

		float4x4 m_world = MathHelper::Identity4x4();
		float4x4 m_view = MathHelper::Identity4x4();
		float4x4 m_proj = MathHelper::Identity4x4();

		CubeMesh*						m_mesh;
		GraphicsTypes::GraphicsPSO*		m_pso;
		GraphicsTypes::GPUBuffer*		m_objCB;
	} m_object;
};