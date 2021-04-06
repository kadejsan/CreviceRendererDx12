#include "stdafx.h"
#include "ShaderTableCache.h"
#include "GraphicsDescriptors.h"
#include "GraphicsResource.h"
#include "Renderer.h"
#include "PSOCache.h"

using namespace Graphics;

ShaderTableCache::ShaderTableCache()
{

}

ShaderTableCache::~ShaderTableCache()
{
}

void ShaderTableCache::Initialize(Graphics::GraphicsDevice& device, const PSOCache& psoCache)
{
	m_stbCache.resize(RTPSO_MAX);

	m_stbCache[PBRSimpleSolidRT] = std::make_unique<ShaderTable>();
	device.CreateShaderTable(psoCache.GetPSO(PBRSimpleSolidRT), m_stbCache[PBRSimpleSolidRT].get());

	m_stbCache[PBRSolidRT] = std::make_unique<ShaderTable>();
	device.CreateShaderTable(psoCache.GetPSO(PBRSolidRT), m_stbCache[PBRSolidRT].get());
}


void ShaderTableCache::Clean()
{
	m_stbCache.clear();
}