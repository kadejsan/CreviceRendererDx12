#pragma once

#include "GraphicsDevice.h"

namespace Graphics
{
	class GraphicsPSO;
}

enum ePSO
{
	SimpleColorSolid,
	SimpleColorWireframe,
	PBRSolid,
	PBRWireframe,

	PSO_MAX,
};

class PSOCache
{
public:
	PSOCache();
	~PSOCache();

	void Initialize( Graphics::GraphicsDevice& device );
	
	Graphics::GraphicsPSO* GetPSO(ePSO pso) const { return m_cache[pso].get(); }

private:
	std::vector<std::unique_ptr<Graphics::GraphicsPSO>> m_cache;
};