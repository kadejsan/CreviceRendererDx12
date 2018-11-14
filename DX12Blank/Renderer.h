#pragma once

#include "GraphicsDevice.h"

class Renderer
{
public:
	static Graphics::GraphicsDevice* GGraphicsDevice;
	static Graphics::GraphicsDevice* GetDevice() { assert(GGraphicsDevice != nullptr);  return GGraphicsDevice; }

	static const Graphics::FORMAT RTFormat_LDR = Graphics::FORMAT_R8G8B8A8_UNORM;
	static const Graphics::FORMAT RTFormat_HDR = Graphics::FORMAT_R16G16B16A16_FLOAT;
	static const Graphics::FORMAT RTFormat_DepthResolve = Graphics::FORMAT_R32_FLOAT;

	static const Graphics::FORMAT DSFormat_Full = Graphics::FORMAT_D24_UNORM_S8_UINT;
	static const Graphics::FORMAT DSFormat_FullAlias = Graphics::FORMAT_R24G8_TYPELESS;
	static const Graphics::FORMAT DSFormat_Small = Graphics::FORMAT_D16_UNORM;
	static const Graphics::FORMAT DSFormat_SmallAlias = Graphics::FORMAT_R16_TYPELESS;
};