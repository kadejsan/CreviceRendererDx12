#pragma once

#include "GraphicsAPI.h"

#include <vector>

class DepthTarget;

using namespace Graphics;

class RenderTarget
{
private:
	int m_numViews;
	std::vector<Texture2D*> m_renderTargets;
	std::vector<Texture2D*> m_renderTargetsResolvedMSAA;
	std::vector<int>		m_resolvedMSAAUpToDate;

public:
	ViewPort				m_viewport;
	Rect					m_scissors;
	DepthTarget*			m_depth;

	RenderTarget();
	RenderTarget(UINT width, UINT height, bool hasDepth = false, FORMAT format = FORMAT_R8G8B8A8_UNORM, UINT mipMapLevelCount = 1, UINT MSAAC = 1, bool depthOnly = false);
	~RenderTarget();
	void CleanUp();

	void Initialize(UINT width, UINT height, bool hasDepth = false, FORMAT format = FORMAT_R8G8B8A8_UNORM, UINT mipMapLevelCount = 1, UINT MSAAC = 1, bool depthOnly = false);
	void InitializeCube(UINT size, bool hasDepth, FORMAT format = FORMAT_R8G8B8A8_UNORM, UINT mipMapLevelCount = 1, bool depthOnly = false);
	void Add(FORMAT format = FORMAT_R8G8B8A8_UNORM);

	void Activate(bool disableColor = false, int viewID = -1);
	void Activate(float r, float g, float b, float a, bool disableColor = false, int viewID = -1);
	void Activate(DepthTarget* getDepth, float r, float g, float b, float a, bool disableColor = false, int viewID = -1);
	void Activate(DepthTarget* getDepth, bool disableColor = false, int viewID = -1);
	void Deactivate();

	void Set(bool disableColor = false, int viewID = -1);
	void Set(DepthTarget* getDepth, bool disableColor = false, int viewID = -1);

	Texture2D* GetTexture(int viewID = 0) const { return m_renderTargets[viewID]; }
	Texture2D* GetTextureResolvedMSAA(int viewID = 0);
	TextureDesc GetDesc(int viewID = 0) const { assert(viewID < m_numViews); return GetTexture(viewID)->m_desc; }
	UINT GetMipCount();
	bool IsInitialized() const { return (m_numViews > 0 || m_depth != nullptr); }
};