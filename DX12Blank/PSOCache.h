#pragma once

#include "GraphicsDevice.h"

namespace Graphics
{
	class GraphicsPSO;
	class ComputePSO;
}

enum eGPSO
{
	GridSolid,
	GizmoSolid,
	SimpleDepthSelection,
	SimpleDepthShadow,
	PBRSimpleSolid,
	PBRSimpleWireframe,
	PBRSolid,
	PBRWireframe,
	SkyboxSolid,
	SkyboxWireframe,
	TonemappingReinhard,
	LightingPass,
	Background,
	SobelFilter,
	LinearizeDepth,
	AmbientOcclusionPass,
	Blur,
	LowPassFilter,

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

enum eRTPSO
{
	PBRSimpleSolidRT,
	PBRSolidRT,
	RTAO,

	RTPSO_MAX,
};

class PSOCache
{
public:
	PSOCache();
	~PSOCache();

	void Initialize( Graphics::GraphicsDevice& device );
	void Clean();

	inline Graphics::GraphicsPSO* GetPSO(eGPSO pso) const { return m_cacheGraphics[pso].get(); }
	inline Graphics::ComputePSO* GetPSO(eCPSO pso) const { return m_cacheCompute[pso].get(); }
	inline Graphics::RayTracePSO* GetPSO(eRTPSO pso) const { return m_cacheRayTrace[pso].get(); }

private:
	void InitializeGraphics( Graphics::GraphicsDevice& device );
	void InitializeCompute( Graphics::GraphicsDevice& device );
	void InitializeRayTrace(Graphics::GraphicsDevice& device);

	std::vector<std::unique_ptr<Graphics::GraphicsPSO>> m_cacheGraphics;
	std::vector<std::unique_ptr<Graphics::ComputePSO>> m_cacheCompute;
	std::vector<std::unique_ptr<Graphics::RayTracePSO>> m_cacheRayTrace;
};