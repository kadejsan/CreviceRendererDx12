#pragma once

#include "GraphicsDevice.h"

class Renderer
{
public:
	static Graphics::GraphicsDevice* GGraphicsDevice;
	static Graphics::GraphicsDevice* GetDevice() { assert(GGraphicsDevice != nullptr);  return GGraphicsDevice; }
};