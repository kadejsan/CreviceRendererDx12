#pragma once

#include "GraphicsDevice.h"

namespace Graphics
{
	class GraphicsPSO;
	class ComputePSO;
}

enum eGPSO
{
	SimpleColorSolid,
	SimpleColorWireframe,
	PBRSimpleSolid,
	PBRSimpleWireframe,
	PBRSolid,
	PBRWireframe,
	SkyboxSolid,
	SkyboxWireframe,
	TonemappingReinhard,
	LightingPass,
	Background,

	GPSO_MAX,
};

enum eCPSO
{
	Equirect2Cube,
	SpecularEnvironmentMap,
	IrradianceMap,
	SpecularBRDFLut,

	CPSO_MAX,
};

class PSOCache
{
public:
	PSOCache();
	~PSOCache();

	void Initialize( Graphics::GraphicsDevice& device );
	
	inline Graphics::GraphicsPSO* GetPSO(eGPSO pso) const { return m_cacheGraphics[pso].get(); }
	inline Graphics::ComputePSO* GetPSO(eCPSO pso) const { return m_cacheCompute[pso].get(); }

private:
	void InitializeGraphics( Graphics::GraphicsDevice& device );
	void InitializeCompute( Graphics::GraphicsDevice& device );

	std::vector<std::unique_ptr<Graphics::GraphicsPSO>> m_cacheGraphics;
	std::vector<std::unique_ptr<Graphics::ComputePSO>> m_cacheCompute;
};