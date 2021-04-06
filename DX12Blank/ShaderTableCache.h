#pragma once

#include "GraphicsDevice.h"

namespace Graphics
{
	class ShaderTable;
}

enum eRTPSO;

class PSOCache;

class ShaderTableCache
{
public:
	ShaderTableCache();
	~ShaderTableCache();

	void Initialize( Graphics::GraphicsDevice& device, const PSOCache& psoCache );
	void Clean();

	inline Graphics::ShaderTable* GetSTB(eRTPSO pso) const { return m_stbCache[pso].get(); }

	std::vector<std::unique_ptr<Graphics::ShaderTable>> m_stbCache;
};