#include "stdafx.h"
#include "Types.h"
#include "TextRenderer.h"
#include "GraphicsResource.h"
#include "GraphicsDevice.h"
#include "GraphicsDescriptors.h"
#include "Renderer.h"

using namespace std;
using namespace Graphics;

#define MAX_TEXT 10000

#define WHITESPACE_SIZE 3

namespace TextRenderer
{
	std::string			Font::FONTPATH = "Fonts/";
	GPUBuffer			*Font::m_fontCB = nullptr;
	GPURingBuffer		*Font::VertexBuffer = nullptr;
	GPUBuffer			*Font::IndexBuffer = nullptr;
	VertexLayout		*Font::VertexLayout = nullptr;
	VertexShader		*Font::VertexShader = nullptr;
	PixelShader			*Font::PixelShader = nullptr;
	BlendState			*Font::BlendState = nullptr;
	RasterizerState		*Font::RasterizerState = nullptr;
	DepthStencilState	*Font::DepthStencilState = nullptr;
	GraphicsPSO			*Font::PSO = nullptr;
	XMMATRIX			Font::ProjectionMatrix;
	std::vector<Font::FontStyle> Font::FontStyles;

	Font::Font(const std::string& text, FontProps props, int style) : props(props), style(style)
	{
		this->text = wstring(text.begin(), text.end());
	}
	Font::Font(const std::wstring& text, FontProps props, int style) : text(text), props(props), style(style)
	{

	}
	Font::~Font()
	{

	}

	void Font::Initialize(UINT screenWidth, UINT screenHeight)
	{
		ProjectionMatrix = XMMatrixOrthographicOffCenterLH(0, (float)screenWidth, (float)screenHeight, 0, -1, 1);
		SetUpStaticComponents();
		SetupConstantBuffer();
	}

	void Font::LoadVertexBuffer()
	{
		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_DYNAMIC;
		bd.ByteWidth = 256 * 1024; // just allocate 256KB to font renderer ring buffer..
		bd.BindFlags = BIND_VERTEX_BUFFER;
		bd.CpuAccessFlags = CPU_ACCESS_WRITE;

		VertexBuffer = new GPURingBuffer;
		Renderer::GGraphicsDevice->CreateBuffer(bd, nullptr, VertexBuffer);
	}

	void Font::LoadIndices()
	{
		uint16_t indices[MAX_TEXT * 6];
		for (uint16_t i = 0; i < MAX_TEXT * 4; i += 4) {
			indices[i / 4 * 6 + 0] = i + 0;
			indices[i / 4 * 6 + 1] = i + 2;
			indices[i / 4 * 6 + 2] = i + 1;
			indices[i / 4 * 6 + 3] = i + 1;
			indices[i / 4 * 6 + 4] = i + 2;
			indices[i / 4 * 6 + 5] = i + 3;
		}

		GPUBufferDesc bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = BIND_INDEX_BUFFER;
		bd.CpuAccessFlags = 0;
		SubresourceData InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.SysMem = indices;
		
		IndexBuffer = new GPUBuffer;
		Renderer::GGraphicsDevice->CreateBuffer(bd, &InitData, IndexBuffer);
	}

	void Font::SetUpStates()
	{
		RasterizerStateDesc rs;
		rs.FillMode = FILL_SOLID;
		rs.CullMode = CULL_BACK;
		rs.FrontCounterClockwise = true;
		rs.DepthBias = 0;
		rs.DepthBiasClamp = 0;
		rs.SlopeScaledDepthBias = 0;
		rs.DepthClipEnable = false;
		rs.MultisampleEnable = false;
		rs.AntialiasedLineEnable = false;
		RasterizerState = new Graphics::RasterizerState;
		RasterizerState->m_desc = rs;

		DepthStencilStateDesc dsd;
		dsd.DepthEnable = false;
		dsd.StencilEnable = false;
		DepthStencilState = new Graphics::DepthStencilState;
		DepthStencilState->m_desc = dsd;

		BlendStateDesc bd;
		bd.RenderTarget[0].BlendEnable = true;
		bd.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bd.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
		BlendState = new Graphics::BlendState;
		BlendState->m_desc = bd;
	}

	void Font::LoadShaders()
	{
		VertexInputLayoutDesc layout[] =
		{
			{ "POSITION", 0, FORMAT_R32G32_FLOAT, 0, APPEND_ALIGNED_ELEMENT, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R16G16_FLOAT, 0, APPEND_ALIGNED_ELEMENT, INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		VertexLayout = new Graphics::VertexLayout();
		Renderer::GGraphicsDevice->CreateInputLayout(layout, 2, VertexLayout);

		VertexShader = new Graphics::VertexShader();
		PixelShader = new Graphics::PixelShader();
		Renderer::GGraphicsDevice->CreateShader(L"Shaders\\Font.hlsl", VertexShader);
		Renderer::GGraphicsDevice->CreateShader(L"Shaders\\Font.hlsl", PixelShader);

		GraphicsPSODesc desc;
		desc.VS = VertexShader;
		desc.PS = PixelShader;
		desc.IL = VertexLayout;
		desc.BS = BlendState;
		desc.RS = RasterizerState;
		desc.DSS = DepthStencilState;
		desc.NumRTs = 1;
		desc.RTFormats[0] = Renderer::GGraphicsDevice->GetBackBufferFormat();
		SAFE_DELETE(PSO);
		PSO = new GraphicsPSO;
		Renderer::GGraphicsDevice->CreateGraphicsPSO(&desc, PSO);
	}

	void Font::SetUpStaticComponents()
	{
		SetUpStates();
		LoadShaders();
		LoadVertexBuffer();
		LoadIndices();

		// add default font:
		addFontStyle("default_font");
	}

	void Font::SetupConstantBuffer()
	{
		m_fontCB = new Graphics::GPUBuffer();

		FontCB fontCB;
		ZeroMemory(&fontCB, sizeof(fontCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(FontCB);
		SubresourceData initData;
		initData.SysMem = &fontCB;
		Renderer::GGraphicsDevice->CreateBuffer(bd, &initData, m_fontCB);
		Renderer::GGraphicsDevice->TransitionBarrier(m_fontCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void Font::CleanUpStatic()
	{
		for (unsigned int i = 0; i < FontStyles.size(); ++i)
			FontStyles[i].CleanUp();
		FontStyles.clear();

		SAFE_DELETE(VertexBuffer);
		SAFE_DELETE(IndexBuffer);
		SAFE_DELETE(VertexLayout);
		SAFE_DELETE(VertexShader);
		SAFE_DELETE(PixelShader);
		SAFE_DELETE(BlendState);
		SAFE_DELETE(RasterizerState);
		SAFE_DELETE(DepthStencilState);
		SAFE_DELETE(m_fontCB);
	}

	void Font::ModifyGeo(volatile Vertex* vertexList, const std::wstring& text, FontProps props, int style)
	{
		size_t vertexCount = text.length() * 4;

		const int lineHeight = (props.m_size < 0 ? FontStyles[style].m_lineHeight : props.m_size);
		const float relativeSize = (props.m_size < 0 ? 1 : (float)props.m_size / (float)FontStyles[style].m_lineHeight);

		int line = 0;
		int pos = 0;
		for (unsigned int i = 0; i < vertexCount; i += 4)
		{
			bool compatible = false;
			FontStyle::LookUp lookup;

			if (text[i / 4] == '\n')
			{
				line += lineHeight + props.m_spacingY;
				pos = 0;
			}
			else if (text[i / 4] == ' ')
			{
				pos += WHITESPACE_SIZE + props.m_spacingX;
			}
			else if (text[i / 4] == '\t')
			{
				pos += (WHITESPACE_SIZE + props.m_spacingX) * 5;
			}
			else if (text[i / 4] < ARRAYSIZE(FontStyles[style].m_lookup) && FontStyles[style].m_lookup[text[i / 4]].character == text[i / 4])
			{
				lookup = FontStyles[style].m_lookup[text[i / 4]];
				compatible = true;
			}

			HALF h0 = XMConvertFloatToHalf(0.0f);

			if (compatible)
			{
				int characterWidth = (int)(lookup.pixelWidth * relativeSize);

				HALF h1 = XMConvertFloatToHalf(1.0f);
				HALF hl = XMConvertFloatToHalf(lookup.left);
				HALF hr = XMConvertFloatToHalf(lookup.right);

				vertexList[i + 0].Pos.x = (float)pos;
				vertexList[i + 0].Pos.y = (float)line;
				vertexList[i + 1].Pos.x = (float)pos + (float)characterWidth;
				vertexList[i + 1].Pos.y = (float)line;
				vertexList[i + 2].Pos.x = (float)pos;
				vertexList[i + 2].Pos.y = (float)line + (float)lineHeight;
				vertexList[i + 3].Pos.x = (float)pos + (float)characterWidth;
				vertexList[i + 3].Pos.y = (float)line + (float)lineHeight;

				vertexList[i + 0].Tex.x = hl;
				vertexList[i + 0].Tex.y = h0;
				vertexList[i + 1].Tex.x = hr;
				vertexList[i + 1].Tex.y = h0;
				vertexList[i + 2].Tex.x = hl;
				vertexList[i + 2].Tex.y = h1;
				vertexList[i + 3].Tex.x = hr;
				vertexList[i + 3].Tex.y = h1;

				pos += characterWidth + props.m_spacingX;
			}
			else
			{
				vertexList[i + 0].Pos.x = 0;
				vertexList[i + 0].Pos.y = 0;
				vertexList[i + 0].Tex.x = h0;
				vertexList[i + 0].Tex.y = h0;
				vertexList[i + 1].Pos.x = 0;
				vertexList[i + 1].Pos.y = 0;
				vertexList[i + 1].Tex.x = h0;
				vertexList[i + 1].Tex.y = h0;
				vertexList[i + 2].Pos.x = 0;
				vertexList[i + 2].Pos.y = 0;
				vertexList[i + 2].Tex.x = h0;
				vertexList[i + 2].Tex.y = h0;
				vertexList[i + 3].Pos.x = 0;
				vertexList[i + 3].Pos.y = 0;
				vertexList[i + 3].Tex.x = h0;
				vertexList[i + 3].Tex.y = h0;
			}
		}
	}

	void Font::Draw()
	{
		if (text.length() <= 0)
		{
			return;
		}

		FontProps newProps = props;

		if (props.h_align == WIFALIGN_CENTER || props.h_align == WIFALIGN_MID)
			newProps.m_posX -= textWidth() / 2;
		else if (props.h_align == WIFALIGN_RIGHT)
			newProps.m_posX -= textWidth();
		if (props.v_align == WIFALIGN_CENTER || props.h_align == WIFALIGN_MID)
			newProps.m_posY -= textHeight() / 2;
		else if (props.v_align == WIFALIGN_BOTTOM)
			newProps.m_posY -= textHeight();

		Renderer::GGraphicsDevice->BindGraphicsPSO(PSO);

		UINT vboffset;
		volatile Vertex* textBuffer = (volatile Vertex*)Renderer::GGraphicsDevice->AllocateFromRingBuffer(VertexBuffer, sizeof(Vertex) *  (UINT)text.length() * 4, vboffset);
		if (textBuffer == nullptr)
		{
			return;
		}
		ModifyGeo(textBuffer, text, newProps, style);
		Renderer::GGraphicsDevice->InvalidateBufferAccess(VertexBuffer);

		GPUBuffer* vbs[] = {
			VertexBuffer,
		};
		const UINT strides[] = {
			sizeof(Vertex),
		};
		const UINT offsets[] = {
			vboffset,
		};
		Renderer::GGraphicsDevice->BindVertexBuffers(vbs, 0, ARRAYSIZE(vbs), strides, offsets);

		assert(text.length() * 4 < 65536 && "The index buffer currently only supports so many characters!");
		Renderer::GGraphicsDevice->BindIndexBuffer(IndexBuffer, FORMAT_R16_UINT, 0);

		Renderer::GGraphicsDevice->BindResource(PS, FontStyles[style].m_texture, 0);

		Renderer::GGraphicsDevice->BindConstantBuffer(SHADERSTAGE::VS, m_fontCB, 0);
		Renderer::GGraphicsDevice->BindConstantBuffer(SHADERSTAGE::PS, m_fontCB, 0);

		FontCB cb;

		if (newProps.m_shadowColor.A() > 0)
		{
			// font shadow render:
			cb.m_transform = XMMatrixTranspose(
				XMMatrixTranslation((float)newProps.m_posX + 1, (float)newProps.m_posY + 1, 0) * ProjectionMatrix
			);
			cb.m_color = XMFLOAT4(newProps.m_shadowColor.R(), newProps.m_shadowColor.G(), newProps.m_shadowColor.B(), newProps.m_shadowColor.A());
			Renderer::GGraphicsDevice->UpdateBuffer(m_fontCB, &cb);

			Renderer::GGraphicsDevice->DrawIndexed((int)text.length() * 6, 0, 0);
		}

		// font base render:
		cb.m_transform = XMMatrixTranspose(
			XMMatrixTranslation((float)newProps.m_posX, (float)newProps.m_posY, 0)
			* ProjectionMatrix
		);
		cb.m_color = XMFLOAT4(newProps.m_color.R(), newProps.m_color.G(), newProps.m_color.B(), newProps.m_color.A());
		Renderer::GGraphicsDevice->UpdateBuffer(m_fontCB, &cb);

		Renderer::GGraphicsDevice->DrawIndexed((int)text.length() * 6, 0, 0);
	}

	int Font::textWidth()
	{
		int maxWidth = 0;
		int currentLineWidth = 0;
		const float relativeSize = (props.m_size < 0 ? 1.0f : (float)props.m_size / (float)FontStyles[style].m_lineHeight);
		for (size_t i = 0; i < text.length(); ++i)
		{
			if (text[i] == '\n')
			{
				currentLineWidth = 0;
			}
			else if (text[i] == ' ')
			{
				currentLineWidth += WHITESPACE_SIZE + props.m_spacingX;
			}
			else if (text[i] == '\t')
			{
				currentLineWidth += (WHITESPACE_SIZE + props.m_spacingX) * 5;
			}
			else
			{
				int characterWidth = (int)(FontStyles[style].m_lookup[text[i]].pixelWidth * relativeSize);
				currentLineWidth += characterWidth + props.m_spacingX;
			}
			maxWidth = max(maxWidth, currentLineWidth);
		}

		return maxWidth;
	}
	int Font::textHeight()
	{
		int i = 0;
		int lines = 1;
		int len = (int)text.length();
		while (i < len)
		{
			if (text[i] == '\n')
			{
				lines++;
			}
			i++;
		}

		const int lineHeight = (props.m_size < 0 ? FontStyles[style].m_lineHeight : props.m_size);
		return lines * (lineHeight + props.m_spacingY);
	}

	void Font::SetText(const std::string& text)
	{
		this->text = wstring(text.begin(), text.end());
	}
	void Font::SetText(const std::wstring& text)
	{
		this->text = text;
	}
	wstring Font::GetText()
	{
		return text;
	}
	string Font::GetTextA()
	{
		return string(text.begin(), text.end());
	}

	Font::FontStyle::FontStyle(const std::string& newName)
	{
		m_name = newName;

		ZeroMemory(m_lookup, sizeof(m_lookup));

		std::stringstream ss(""), ss1("");
		ss << FONTPATH << m_name << ".Font";
		ss1 << FONTPATH << m_name << ".dds";
		std::ifstream file(ss.str());
		if (file.is_open())
		{
			Renderer::GGraphicsDevice->CreateTextureFromFile(ss1.str().c_str(), &m_texture, false);
			if (m_texture == nullptr)
			{
				return;
			}
			m_texWidth = m_texture->m_desc.Width;
			m_texHeight = m_texture->m_desc.Height;

			string voidStr;
			file >> voidStr >> m_lineHeight;
			while (!file.eof())
			{
				int code = 0;
				file >> code;
				m_lookup[code].ascii = code;
				file >> m_lookup[code].character >> m_lookup[code].left >> m_lookup[code].right >> m_lookup[code].pixelWidth;
			}


			file.close();
		}
		else
		{
			LOG("Could not load Font Data: %s.", ss.str().c_str());
		}
	}

	void Font::FontStyle::CleanUp() 
	{
		SAFE_DELETE(m_texture);
	}

	void Font::addFontStyle(const std::string& toAdd) {
		for (auto& x : FontStyles)
		{
			if (!x.m_name.compare(toAdd))
				return;
		}
		FontStyles.push_back(FontStyle(toAdd));
	}

	int Font::getFontStyleByName(const std::string& get) {
		for (unsigned int i = 0; i < FontStyles.size(); i++)
			if (!FontStyles[i].m_name.compare(get))
				return i;
		return 0;
	}

	void Font::CleanUp()
	{
		SAFE_DELETE(m_fontCB);
	}
}