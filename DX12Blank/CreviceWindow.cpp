
#include "stdafx.h"
#include "GraphicsDevice_DX12.h"
#include "CreviceWindow.h"
#include "DXHelper.h"
#include "CubeMesh.h"
#include "Material.h"

CreviceWindow::CreviceWindow( std::string name )
	: BaseWindow( name )
{
}

void CreviceWindow::OnInit()
{
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
	
	m_engineTimer.Reset();

	m_graphicsDevice = new GraphicsTypes::GraphicsDevice_DX12();
	m_graphicsDevice->Initialize( this );
}

void CreviceWindow::OnUpdate()
{
	BaseWindow::OnUpdate();
}

void CreviceWindow::OnRender()
{
	GetDevice().PresentBegin();

	// #TODO: Initialize with dedicated copy queue out of OnRender method
	InitializeRenderObject();

	// Update buffer
	UpdateConstantBuffer();

	GPUBuffer* vertexBufs[] = { &m_object.m_mesh->m_vertexBufferGPU };
	const UINT strides[] = { m_object.m_mesh->m_vertexBufferGPU.m_desc.StructureByteStride };
	GetDevice().BindVertexBuffers(vertexBufs, 0, 1, strides);
	GetDevice().BindIndexBuffer(&(m_object.m_mesh->m_indexBufferGPU), m_object.m_mesh->m_indexBufferGPU.m_desc.Format, 0);
	GetDevice().BindGraphicsPSO(m_object.m_pso);
	GetDevice().BindConstantBuffer(SHADERSTAGE::VS, m_object.m_objCB, 0);

	GetDevice().DrawIndexed(m_object.m_mesh->m_drawArgs["box"].IndexCount, 0, 0);

	GetDevice().PresentEnd();
}

void CreviceWindow::OnDestroy()
{
	delete m_graphicsDevice;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CreviceWindow::OnKeyDown(UINT8 key)
{
	BaseWindow::OnKeyDown(key);
}
void CreviceWindow::OnKeyUp(UINT8 key)
{
	BaseWindow::OnKeyUp(key);
}

void CreviceWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseDown(btnState, x, y);
}

void CreviceWindow::OnMouseUp(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseUp(btnState, x, y);
}

void CreviceWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
	BaseWindow::OnMouseMove(btnState, x, y);
}

void CreviceWindow::InitializeRenderObject()
{
	// Initialize box mesh
	if (m_object.m_mesh == nullptr)
		m_object.m_mesh = new CubeMesh(m_graphicsDevice);

	// Build PSO
	if(m_object.m_pso == nullptr)
	{
		GraphicsPSODesc psoDesc = {};
		psoDesc.VS = new VertexShader();
		psoDesc.PS = new PixelShader();
		m_graphicsDevice->CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.VS);
		m_graphicsDevice->CreateShader(L"Shaders\\SimpleColor.hlsl", psoDesc.PS);
		{
			VertexInputLayoutDesc layoutDesc[2] =
			{
				{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, FORMAT_R32G32B32A32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 }
			};
			psoDesc.IL = new VertexLayout();
			m_graphicsDevice->CreateInputLayout(layoutDesc, 2, psoDesc.IL);
		}
		psoDesc.RS = new RasterizerState();
		psoDesc.BS = new BlendState();
		psoDesc.DSS = new DepthStencilState();
		psoDesc.PT = PRIMITIVETOPOLOGY::TRIANGLELIST;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.NumRTs = 1;
		psoDesc.RTFormats[0] = m_graphicsDevice->GetBackBufferFormat();
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSFormat = m_graphicsDevice->GetDepthStencilFormat();

		m_object.m_pso = new GraphicsPSO();
		m_graphicsDevice->CreateGraphicsPSO(&psoDesc, m_object.m_pso);
	}

	if (m_object.m_objCB == nullptr)
	{
		m_object.m_objCB = new GraphicsTypes::GPUBuffer();

		XMMATRIX world = XMLoadFloat4x4(&m_object.m_world);
		XMMATRIX view = XMLoadFloat4x4(&m_object.m_view);
		XMMATRIX proj = XMLoadFloat4x4(&m_object.m_proj);
		XMMATRIX worldViewProj = world * view * proj;

		ObjectConstants objCB;
		ZeroMemory(&objCB, sizeof(objCB));
		// Update constant buffer with the latest worldViewProj matrix
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(ObjectConstants);
		SubresourceData initData;
		initData.SysMem = &objCB;
		GetDevice().CreateBuffer(bd, &initData, m_object.m_objCB);
		GetDevice().TransitionBarrier(m_object.m_objCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void CreviceWindow::UpdateConstantBuffer()
{
	if (m_object.m_objCB != nullptr)
	{
		// Convert Spherical to Cartesian coordinates.
		float x = m_radius * sinf(m_phi)*cosf(m_theta);
		float z = m_radius * sinf(m_phi)*sinf(m_theta);
		float y = m_radius * cosf(m_phi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&m_object.m_view, view);

		// The window resized, so update the aspect ratio and recompute the projection matrix.
		XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&m_object.m_proj, proj);

		XMMATRIX world = XMLoadFloat4x4(&m_object.m_world);
		XMMATRIX worldViewProj = world * view * proj;

		ObjectConstants objCB;
		ZeroMemory(&objCB, sizeof(objCB));
		// Update constant buffer with the latest worldViewProj matrix
		XMStoreFloat4x4(&objCB.WorldViewProj, XMMatrixTranspose(worldViewProj));
		GetDevice().UpdateBuffer(m_object.m_objCB, &objCB, sizeof(ObjectConstants));
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

CreviceWindow::RenderObject::~RenderObject()
{
	SAFE_DELETE(m_mesh);
	SAFE_DELETE(m_pso);
}
