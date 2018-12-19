#pragma once

#include "GraphicsDevice.h"

namespace Graphics
{
	class GraphicsPSO;
}

enum eSamplerState
{
	LinearClamp,
	LinearWrap,
	AnisotropicClamp,
	AnisotropicWrap,

	SS_MAX,
};

class SamplerCache
{
public:
	SamplerCache();
	~SamplerCache();

	void Initialize(Graphics::GraphicsDevice& device);

	inline Graphics::Sampler* GetSamplerState(eSamplerState state) const { return m_cache[state].get(); }

private:
	std::vector<std::unique_ptr<Graphics::Sampler>> m_cache;
};