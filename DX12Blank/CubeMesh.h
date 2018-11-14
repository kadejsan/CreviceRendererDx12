#pragma once

#include "Mesh.h"

namespace GraphicsTypes
{
	class GraphicsDevice;
}

class CubeMesh : public Mesh
{
public:
	CubeMesh(GraphicsDevice* graphicsDevice );
	~CubeMesh();

private:
	void CreateBoxMesh(GraphicsDevice* graphicsDevice);
};