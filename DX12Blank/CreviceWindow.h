#pragma once

#include "BaseWindow.h"
#include "GraphicsDevice.h"
#include "MathHelper.h"
#include "PSOCache.h"
#include "RenderObject.h"

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
	GraphicsTypes::GraphicsDevice*	m_graphicsDevice;
	PSOCache						m_psoCache;


	// Temporary: objects list rendering
	// #TODO: Scene graph
	std::vector<RenderObject>		m_renderObjects;
	void InitializeRenderObjects();
	void UpdateConstantBuffer(const RenderObject& renderObject);

	struct ObjectConstants
	{
		float4x4 WorldViewProj = MathHelper::Identity4x4();
	};
	GraphicsTypes::GPUBuffer*		m_objCB;
};