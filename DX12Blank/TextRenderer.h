#pragma once

#include "MathHelper.h"
#include "Color.h"

namespace Graphics
{
	class GraphicsDevice;
	class GPURingBuffer;
	class GPUBuffer;
	class VertexLayout;
	class VertexShader;
	class PixelShader;
	class BlendState;
	class RasterizerState;
	class DepthStencilState;
	class GraphicsPSO;
	class Texture2D;
}

namespace TextRenderer
{
	// Do not alter order because it is bound to lua manually
	enum FontAlign
	{
		WIFALIGN_LEFT,
		// same as mid
		WIFALIGN_CENTER,
		// same as center
		WIFALIGN_MID,
		WIFALIGN_RIGHT,
		WIFALIGN_TOP,
		WIFALIGN_BOTTOM,
		WIFALIGN_COUNT,
	};

	class FontProps
	{
	public:
		int m_size;
		int m_spacingX, m_spacingY;
		int m_posX, m_posY;
		FontAlign h_align, v_align;
		Color m_color;
		Color m_shadowColor;

		FontProps(int posX = 0, int posY = 0, int size = -1, FontAlign h_align = WIFALIGN_LEFT, FontAlign v_align = WIFALIGN_TOP
			, int spacingX = 2, int spacingY = 1, const Color& color = Color(255, 255, 255, 255, 8), const Color& shadowColor = Color(0, 0, 0, 0, 8))
			: m_posX(posX)
			, m_posY(posY)
			, m_size(size)
			, h_align(h_align)
			, v_align(v_align)
			, m_spacingX(spacingX)
			, m_spacingY(spacingY)
			, m_color(color)
			, m_shadowColor(shadowColor)
		{}
	};

	class Font
	{
	public:
		static std::string FONTPATH;
	protected:
		struct Vertex
		{
			XMFLOAT2 Pos;
			XMHALF2 Tex;
		};
		static Graphics::GPURingBuffer      *VertexBuffer;
		static Graphics::GPUBuffer          *IndexBuffer;

		static Graphics::VertexLayout		*VertexLayout;
		static Graphics::VertexShader		*VertexShader;
		static Graphics::PixelShader		*PixelShader;
		static Graphics::BlendState			*BlendState;
		static Graphics::RasterizerState	*RasterizerState;
		static Graphics::DepthStencilState	*DepthStencilState;
		static Graphics::GraphicsPSO		*PSO;
		
		static XMMATRIX						ProjectionMatrix;

	private:
		static void SetUpStates();
		static void LoadShaders();
		static void LoadVertexBuffer();
		static void LoadIndices();

		struct FontStyle 
		{
			std::string				m_name;
			Graphics::Texture2D*	m_texture;

			struct LookUp 
			{
				int ascii;
				char character;
				float left;
				float right;
				int pixelWidth;
			};
			LookUp m_lookup[128];
			int m_texWidth, m_texHeight;
			int m_lineHeight;

			FontStyle() {}
			FontStyle(const std::string& newName);
			void CleanUp();
		};
		static std::vector<FontStyle> FontStyles;

		void ModifyGeo(volatile Vertex* vertexList, const std::wstring& text, FontProps props, int style);

	public:
		static void Initialize(UINT screenWidth, UINT screenHeigh);
		static void SetUpStaticComponents();
		static void SetupConstantBuffer();
		static void CleanUpStatic();

		std::wstring text;
		FontProps props;
		int style;

		Font(const std::string& text = "", FontProps props = FontProps(), int style = 0);
		Font(const std::wstring& text, FontProps props = FontProps(), int style = 0);
		~Font();
		
		void Draw();

		int textWidth();
		int textHeight();

		static void addFontStyle(const std::string& toAdd);
		static int getFontStyleByName(const std::string& get);

		void SetText(const std::string& text);
		void SetText(const std::wstring& text);
		std::wstring GetText();
		std::string GetTextA();

		void CleanUp();

		struct FontCB
		{
			XMMATRIX m_transform;
			XMFLOAT4 m_color;
		};
		static Graphics::GPUBuffer*		m_fontCB;
	};
}