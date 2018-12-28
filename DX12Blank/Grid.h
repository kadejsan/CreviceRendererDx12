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
	struct GridCB
	{
		XMMATRIX mTransform;
		XMFLOAT4 mColor;
	};
	int m_gridVertexCount;
	GPUBuffer* m_vb;
	GPUBuffer* m_cb;
};