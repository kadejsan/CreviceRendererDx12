#pragma once

#include "GraphicsAPI.h"

using namespace Graphics;

class DepthTarget
{
private:
	Texture2D*  m_texture;
	Texture2D*  m_textureResolvedMSAA;
	bool		m_resolvedMSAAUpToDate;

public:
	DepthTarget();
	~DepthTarget();

	void Initialize(int width, int height, UINT MSAAC = 1, float clearDepth = 1.0f, UINT8 clearStencil = 0);
	void InitializeCube(int size, bool independentFaces = false);
	void Clear(float clearVal = 1.0f, UINT8 clearStencil = 0);
	void CopyFrom(const DepthTarget& from);

	Texture2D*	GetTexture() const { return m_texture; }
	Texture2D*	GetTextureResolvedMSAA();
	TextureDesc	GetDesc() const { return m_texture->m_desc; }
};