#pragma once

#include "RenderObject.h"

class Grid
{
public:
	Grid();
	~Grid();

	void Initialize();
	void Render();

private:
	int m_gridVertexCount;
	GPUBuffer* m_vb;
};