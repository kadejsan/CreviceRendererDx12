#pragma once

#include "Mesh.h"

namespace GraphicsTypes
{
	class GraphicsDevice;
}

class MeshGenerator
{
public:
	MeshGenerator(GraphicsDevice& graphicsDevice) {}
	~MeshGenerator() {}

private:
	///
	Mesh CreateBox(GraphicsDevice& graphicsDevice);
};