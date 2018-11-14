#pragma once

#include "GraphicsDevice.h"

namespace GraphicsTypes
{
	class GraphicsPSO;
}

enum ePSO
{
	SimpleColorSolid,
	SimpleColorWireframe,

	PSO_MAX,
};

class PSOCache
{
public:
	PSOCache();
	~PSOCache();

	void Initialize( GraphicsTypes::GraphicsDevice& device );
	
	GraphicsTypes::GraphicsPSO* GetPSO(ePSO pso) const { return m_cache[pso].get(); }

private:
	std::vector<std::unique_ptr<GraphicsTypes::GraphicsPSO>> m_cache;
};