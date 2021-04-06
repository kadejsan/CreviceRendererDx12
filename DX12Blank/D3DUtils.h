#pragma once

#include "Include_DX12.h"

using namespace Microsoft::WRL;

struct SHADER_DEFINE
{
	std::string Name;
	std::string Value;
};

class D3DUtils
{
public:
	enum EShaderType
	{
		Rasterize,
		RayTrace
	};

	static bool InitializeDXCShaderCompiler();
	static void Destroy();

	static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const std::vector<SHADER_DEFINE>& defines,
		const std::string& entrypoint,
		const std::string& target,
		const EShaderType type = EShaderType::Rasterize);

public:
	static IDxcCompiler* st_compiler;
	static IDxcLibrary* st_library;
	static IDxcIncludeHandler* st_dxcIncludeHandler;
};