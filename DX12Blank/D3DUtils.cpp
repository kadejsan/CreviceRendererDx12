#include "stdafx.h"
#include "stringUtils.h"
#include "D3DUtils.h"
#include "DXHelper.h"

IDxcCompiler* D3DUtils::st_compiler = nullptr;
IDxcLibrary* D3DUtils::st_library = nullptr;
IDxcIncludeHandler* D3DUtils::st_dxcIncludeHandler = nullptr;

bool D3DUtils::InitializeDXCShaderCompiler()
{
	if (!st_compiler)
	{
		ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&st_compiler)));
		ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&st_library)));
		ThrowIfFailed(st_library->CreateIncludeHandler(&st_dxcIncludeHandler));
	}
	return true;
}

void D3DUtils::Destroy()
{
	SAFE_RELEASE(st_compiler);
	SAFE_RELEASE(st_library);
	SAFE_RELEASE(st_dxcIncludeHandler);
}

Microsoft::WRL::ComPtr<ID3DBlob> D3DUtils::LoadBinary(const std::wstring& filename)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	std::ifstream::pos_type size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	ComPtr<ID3DBlob> blob;
	ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return blob;
}

ComPtr<ID3DBlob> D3DUtils::CompileShader(
	const std::wstring& filename,
	const std::vector<SHADER_DEFINE>& defines,
	const std::string& entrypoint,
	const std::string& target,
	const EShaderType type /* = EShaderType::Rasterize */ )
{
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> byteCode = nullptr;

	if (type == Rasterize)
	{	
		// Defines
		std::vector<D3D_SHADER_MACRO> D3DShaderDefines(defines.size()*2);
		for (int i = 0; i < defines.size(); ++i)
		{
			int j = 2 * i;
			D3DShaderDefines[j] = D3D_SHADER_MACRO{ defines[j].Name.c_str(), defines[j].Value.c_str() };
		}

		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

		ComPtr<ID3DBlob> errors;
		hr = D3DCompileFromFile(filename.c_str(), D3DShaderDefines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

		if (errors != nullptr)
			LOG((char*)errors->GetBufferPointer());

		ThrowIfFailed(hr);
	}
	else if(type == RayTrace)
	{
		UINT32 code(0);
		IDxcBlobEncoding* pShaderText = nullptr;

		// Load and encode the shader file
		hr = st_library->CreateBlobFromFile(filename.c_str(), &code, &pShaderText);
		if (FAILED(hr)) LOG("Error - ShaderCompiler: Failed to create blob from shader file!");
		ThrowIfFailed(hr);

		// Create the compiler include handler
		ComPtr<IDxcIncludeHandler> dxcIncludeHandler;
		hr = st_library->CreateIncludeHandler(&dxcIncludeHandler);
		if (FAILED(hr)) LOG("Error - ShaderCompiler: Failed to create include handler!");
		ThrowIfFailed(hr);

		// Defines
		std::vector<DxcDefine> dxcDefines(defines.size());
		std::vector<std::pair<std::wstring, std::wstring>> definesW(defines.size());
		for (int i = 0; i < defines.size(); i += 2)
		{
			utf8ToUTF16(defines[i].Name, definesW[i].first);
			utf8ToUTF16(defines[i].Value, definesW[i].second);
			dxcDefines[i].Name = definesW[i].first.c_str();
			dxcDefines[i].Value = definesW[i].second.c_str();
		}

		std::wstring entrypointW; utf8ToUTF16(entrypoint, entrypointW);
		std::wstring targetW; utf8ToUTF16(target, targetW);

		const wchar_t* pArgs[] =
		{
#ifdef _DEBUG
			L"-WX",				//Warnings as errors
			L"-Zi",				//Debug info
			L"-Qembed_debug",	//Embed debug info into the shader
			L"-Od",				//Disable optimization
#else
			L"-O3",				//Optimization level 3
#endif
		};

		IDxcOperationResult* pResult;
		hr = st_compiler->Compile(pShaderText, filename.c_str(), entrypointW.c_str(), targetW.c_str(),
			&pArgs[0], sizeof(pArgs) / sizeof(pArgs[0]), dxcDefines.data(), (UINT32)dxcDefines.size(), dxcIncludeHandler.Get(), &pResult);
		if (FAILED(hr)) LOG("Error - Shader Compiler: Failed to compile shader!");
		pResult->GetStatus(&hr);
		if (FAILED(hr))
		{
			IDxcBlobEncoding* errors;
			hr = pResult->GetErrorBuffer(&errors);
			if(errors != nullptr)
				LOG((char*)errors->GetBufferPointer());
		}

		IDxcBlob* blob = nullptr;
		pResult->GetResult(&blob);
		if (FAILED(hr)) LOG("Error - Shader Compiler: Failed to get shader blob result!");

		byteCode = (ID3DBlob*)blob;
	}

	return byteCode;
}