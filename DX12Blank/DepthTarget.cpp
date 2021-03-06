#include "stdafx.h"
#include "DepthTarget.h"
#include "Renderer.h"

DepthTarget::DepthTarget()
	: m_texture( nullptr )
	, m_textureResolvedMSAA( nullptr )
	, m_resolvedMSAAUpToDate( false )
{
}

DepthTarget::~DepthTarget()
{
	SAFE_DELETE(m_texture);
	SAFE_DELETE(m_textureResolvedMSAA);
}

void DepthTarget::Initialize(int width, int height, UINT MSAAC, float clearDepth, UINT8 clearStencil)
{
	SAFE_DELETE(m_texture);
	SAFE_DELETE(m_textureResolvedMSAA);
	m_resolvedMSAAUpToDate = false;

	TextureDesc depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));

	// Setup description of the depth buffer
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = Renderer::DSFormat_FullAlias;
	depthDesc.SampleDesc.Count = MSAAC;
	// depthDesc.SampleDesc.Quality = 0; // auto-fill in device
	depthDesc.Usage = USAGE_DEFAULT;
	depthDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	depthDesc.ClearDepth = clearDepth;
	depthDesc.ClearStencil = clearStencil;

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

	Renderer::GetDevice()->CreateTexture2D(depthDesc, nullptr, &m_texture);
	Renderer::GetDevice()->TransitionBarrier(m_texture, RESOURCE_STATE_COMMON, RESOURCE_STATE_DEPTH_WRITE);

	if (MSAAC > 1)
	{
		depthDesc.SampleDesc.Count = 1;
		depthDesc.Format = Renderer::RTFormat_DepthResolve;
		depthDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

		Renderer::GetDevice()->CreateTexture2D(depthDesc, nullptr, &m_textureResolvedMSAA);
	}
}

void DepthTarget::InitializeCube(int size, bool indepdendentFaces)
{
	SAFE_DELETE(m_texture);
	SAFE_DELETE(m_textureResolvedMSAA);
	m_resolvedMSAAUpToDate = false;

	TextureDesc depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));

	// Setup description of the depth buffer
	depthDesc.Width = size;
	depthDesc.Height = size;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 6;
	depthDesc.Format = Renderer::DSFormat_SmallAlias;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = USAGE_DEFAULT;
	depthDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = RESOURCE_MISC_TEXTURECUBE;

	m_texture = new Texture2D();
	m_texture->RequestIndependentRenderTargetCubemapFaces(indepdendentFaces);
	
	Renderer::GetDevice()->CreateTexture2D(depthDesc, nullptr, &m_texture);
	Renderer::GetDevice()->TransitionBarrier(m_texture, RESOURCE_STATE_COMMON, RESOURCE_STATE_DEPTH_WRITE);
}

void DepthTarget::Clear(float clearDepth /*=1.0f*/, UINT8 clearStencil /*=0*/ )
{
	Renderer::GetDevice()->ClearDepthStencil(GetTexture(), CLEAR_DEPTH | CLEAR_STENCIL, clearDepth, clearStencil );
	m_resolvedMSAAUpToDate = false;
}

void DepthTarget::CopyFrom(const DepthTarget& from)
{
	Renderer::GetDevice()->CopyTexture(GetTexture(), from.GetTexture());
}

void DepthTarget::Set()
{
	Renderer::GetDevice()->BindViewports(1, &m_viewport);
	Renderer::GetDevice()->SetScissorRects(1, &m_scissors);
}

Texture2D* DepthTarget::GetTextureResolvedMSAA()
{
	if (m_textureResolvedMSAA != nullptr)
	{
		if (!m_resolvedMSAAUpToDate)
		{
			// TODO: implement support for depth resolve
		}
	}

	return nullptr;
}