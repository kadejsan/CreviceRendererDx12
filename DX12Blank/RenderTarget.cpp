
#include "stdafx.h"
#include "RenderTarget.h"
#include "DepthTarget.h"
#include "Renderer.h"

RenderTarget::RenderTarget()
	: m_numViews(0)
	, m_depth(nullptr)
{
}

RenderTarget::RenderTarget(UINT width, UINT height, bool hasDepth /* = false */, FORMAT format /* = FORMAT_R8G8B8A8_UNORM */, UINT mipMapLevelCount /* = 1 */, UINT MSAAC /* = 1 */, bool depthOnly /* = false */)
	: m_numViews(0)
	, m_depth(nullptr)
{
	Initialize(width, height, hasDepth, format, mipMapLevelCount, MSAAC);
}

RenderTarget::~RenderTarget()
{
	CleanUp();
}

void RenderTarget::CleanUp() {
	for (size_t i = 0; i < m_renderTargets.size(); ++i)
	{
		SAFE_DELETE(m_renderTargets[i]);
	}
	for (size_t i = 0; i < m_renderTargetsResolvedMSAA.size(); ++i)
	{
		SAFE_DELETE(m_renderTargetsResolvedMSAA[i]);
	}
	m_renderTargets.clear();
	m_renderTargetsResolvedMSAA.clear();
	SAFE_DELETE(m_depth);
	m_resolvedMSAAUpToDate.clear();
}

void RenderTarget::Initialize(UINT width, UINT height, bool hasDepth /*= false*/, FORMAT format /*= FORMAT_R8G8B8A8_UNORM*/, UINT mipMapLevelCount /*= 1*/, UINT MSAAC /*= 1*/, bool depthOnly /*= false*/)
{
	CleanUp();

	if (!depthOnly)
	{
		TextureDesc textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = mipMapLevelCount;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = MSAAC;
		//textureDesc.SampleDesc.Quality = 0; // auto-fill in device to maximum
		textureDesc.Usage = USAGE_DEFAULT;
		textureDesc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		textureDesc.ClearDepth = 1.0f;
		textureDesc.ClearStencil = 0;

		m_numViews = 1;
		Texture2D* texture = new Texture2D;
		if (mipMapLevelCount != 1)
		{
			texture->RequestIndependentShaderResourcesForMIPs(true);
			texture->RequestIndependentUnorderedAccessResourcesForMIPs(true);
			textureDesc.BindFlags |= BIND_UNORDERED_ACCESS;
			textureDesc.MiscFlags = RESOURCE_MISC_GENERATE_MIPS;
		}
		m_renderTargets.push_back(texture);
		Renderer::GetDevice()->CreateTexture2D(textureDesc, nullptr, &m_renderTargets[0]);
		if (MSAAC > 1)
		{
			textureDesc.SampleDesc.Count = 1;
			m_renderTargetsResolvedMSAA.push_back(nullptr);
			Renderer::GetDevice()->CreateTexture2D(textureDesc, nullptr, &m_renderTargetsResolvedMSAA[0]);
			m_resolvedMSAAUpToDate.push_back(false);
		}
		else
		{
			m_resolvedMSAAUpToDate.push_back(true);
		}
	}

	m_viewport.Width = (FLOAT)width;
	m_viewport.Height = (FLOAT)height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_scissors.left = 0;
	m_scissors.top = 0;
	m_scissors.right = width;
	m_scissors.bottom = height;

	if (hasDepth) 
	{
		m_depth = new DepthTarget();
		m_depth->Initialize(width, height, MSAAC);
	}
}

void RenderTarget::InitializeCube(UINT size, bool hasDepth, FORMAT format /*= FORMAT_R8G8B8A8_UNORM*/, UINT mipMapLevelCount /*= 1*/, bool depthOnly /*= false*/)
{
	CleanUp();

	if (!depthOnly)
	{
		TextureDesc textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = size;
		textureDesc.Height = size;
		textureDesc.MipLevels = mipMapLevelCount;
		textureDesc.ArraySize = 6;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = USAGE_DEFAULT;
		textureDesc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = RESOURCE_MISC_TEXTURECUBE;
		if (mipMapLevelCount != 1)
		{
			textureDesc.MiscFlags |= RESOURCE_MISC_GENERATE_MIPS;
		}

		m_numViews = 1;
		m_renderTargets.push_back(nullptr);
		Renderer::GetDevice()->CreateTexture2D(textureDesc, nullptr, &m_renderTargets[0]);
		m_resolvedMSAAUpToDate.push_back(true);
	}

	m_viewport.Width = (FLOAT)size;
	m_viewport.Height = (FLOAT)size;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	if (hasDepth) {
		m_depth = new DepthTarget();
		m_depth->InitializeCube(size);
	}
}

void RenderTarget::Add(FORMAT format /*= FORMAT_R8G8B8A8_UNORM*/)
{
	TextureDesc desc = GetTexture(0)->m_desc;
	desc.Format = format;

	if (!m_renderTargets.empty())
	{
		m_numViews++;
		m_renderTargets.push_back(nullptr);

		Renderer::GetDevice()->CreateTexture2D(desc, nullptr, &m_renderTargets.back());
		
		if (desc.SampleDesc.Count > 1)
		{
			desc.SampleDesc.Count = 1;
			m_renderTargetsResolvedMSAA.push_back(nullptr);

			Renderer::GetDevice()->CreateTexture2D(desc, nullptr, &m_renderTargets.back());

			m_resolvedMSAAUpToDate.push_back(false);
		}
		else
		{
			m_resolvedMSAAUpToDate.push_back(true);
		}
	}
	else
	{
		assert(0 && "Rendertarget Add failed because it's not properly initialized!");
	}
}

void RenderTarget::Activate(bool disableColor /*= false*/, int viewID /*= -1*/)
{
	Activate(0, 0, 0, 0, disableColor, viewID);
}

void RenderTarget::Activate(float r, float g, float b, float a, bool disableColor /*= false*/, int viewID /*= -1*/)
{
	Set(disableColor, viewID);
	float clearColor[4] = { r, g, b, a };
	if (viewID >= 0)
	{
		Renderer::GetDevice()->ClearRenderTarget(GetTexture(viewID), clearColor);
	}
	else
	{
		for (int i = 0; i < m_numViews; ++i)
		{
			Renderer::GetDevice()->ClearRenderTarget(GetTexture(i), clearColor);
		}
	}
	if (m_depth)
	{
		m_depth->Clear();
	}
}

void RenderTarget::Activate(DepthTarget* getDepth, float r, float g, float b, float a, bool disableColor /*= false*/, int viewID /*= -1*/)
{
	Set(getDepth, disableColor, viewID);
	float clearColor[4] = { r, g, b, a };
	if (viewID >= 0)
	{
		Renderer::GetDevice()->ClearRenderTarget(GetTexture(viewID), clearColor);
	}
	else
	{
		for (int i = 0; i < m_numViews; ++i)
		{
			Renderer::GetDevice()->ClearRenderTarget(GetTexture(i), clearColor);
		}
	}
}

void RenderTarget::Activate(DepthTarget* getDepth, bool disableColor /*= false*/, int viewID /*= -1*/)
{
	Activate(getDepth, 0, 0, 0, 0, disableColor, viewID);
}

void RenderTarget::Deactivate()
{
	Renderer::GetDevice()->BindRenderTargets(0, nullptr, nullptr);
}

void RenderTarget::Set(bool disableColor /*= false*/, int viewID /*= -1*/)
{

	Renderer::GetDevice()->BindViewports(1, &m_viewport);
	Renderer::GetDevice()->SetScissorRects(1, &m_scissors);
	if (viewID >= 0)
	{
		Renderer::GetDevice()->BindRenderTargets(disableColor ? 0 : 1, disableColor ? nullptr : (Texture2D**)&m_renderTargets[viewID], (m_depth ? m_depth->GetTexture() : nullptr));
		m_resolvedMSAAUpToDate[viewID] = false;
	}
	else
	{
		Renderer::GetDevice()->BindRenderTargets(disableColor ? 0 : m_numViews, disableColor ? nullptr : (Texture2D**)m_renderTargets.data(), (m_depth ? m_depth->GetTexture() : nullptr));
		for (auto& x : m_resolvedMSAAUpToDate)
		{
			x = false;
		}
	}
}

void RenderTarget::Set(DepthTarget* getDepth, bool disableColor /*= false*/, int viewID /*= -1*/)
{
	Renderer::GetDevice()->BindViewports(1, &m_viewport);
	Renderer::GetDevice()->SetScissorRects(1, &m_scissors);
	if (viewID >= 0)
	{
		Renderer::GetDevice()->BindRenderTargets(disableColor ? 0 : 1, disableColor ? nullptr : (Texture2D**)&m_renderTargets[viewID], (getDepth ? getDepth->GetTexture() : nullptr));
		m_resolvedMSAAUpToDate[viewID] = false;
	}
	else
	{
		Renderer::GetDevice()->BindRenderTargets(disableColor ? 0 : m_numViews, disableColor ? nullptr : (Texture2D**)m_renderTargets.data(), (getDepth ? getDepth->GetTexture() : nullptr));
		for (auto& x : m_resolvedMSAAUpToDate)
		{
			x = false;
		}
	}
}

void RenderTarget::Clear()
{
	float clearColor[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < m_numViews; ++i)
	{
		Renderer::GetDevice()->ClearRenderTarget(GetTexture(i), clearColor);
	}

	if (m_depth)
	{
		m_depth->Clear();
	}
}

Graphics::Texture2D* RenderTarget::GetTextureResolvedMSAA(int viewID /*= 0*/)
{
	if (GetDesc(viewID).SampleDesc.Count > 1)
	{
		if (m_resolvedMSAAUpToDate[viewID] == false)
		{
			Renderer::GetDevice()->MSAAResolve(m_renderTargetsResolvedMSAA[viewID], m_renderTargets[viewID]);
			m_resolvedMSAAUpToDate[viewID] = true;
		}
		return m_renderTargetsResolvedMSAA[viewID];
	}

	return m_renderTargets[viewID];
}

UINT RenderTarget::GetMipCount()
{
	TextureDesc desc = GetDesc();

	if (desc.MipLevels > 0)
		return desc.MipLevels;

	UINT maxDim = std::max(desc.Width, desc.Height);
	return static_cast<UINT>(log2(static_cast<double>(maxDim)));
}
