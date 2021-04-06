#pragma once

#include "stdafx.h"

static std::string utf16ToUTF8(const std::wstring &s, std::string& out)
{
	const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, 0, NULL);

	out.resize(size);
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &out[0], size, 0, NULL);

	return std::string(&out[0]);
}

static std::wstring utf8ToUTF16(const std::string& s, std::wstring& out)
{
	const int size = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);

	out.resize(size);
	::MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &out[0], size);

	return std::wstring(&out[0]);
}