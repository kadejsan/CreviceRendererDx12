#pragma once

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently.

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif

#include <Windows.h>
#include <WindowsX.h>
#include <shellapi.h>

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <d3dcompiler.h>

#include "Types.h"

// std
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <atomic>
#include <array>
#include <unordered_map>
#include <comdef.h>
#include <fstream>

#define LOG( str, ... ) { char buffer[500]; sprintf_s(buffer, 500, str, __VA_ARGS__); OutputDebugString(buffer); }

#define SAFE_INIT(a) (a) = nullptr;
#define SAFE_RELEASE(a) if((a)!=nullptr){(a)->Release();(a)=nullptr;}
#define SAFE_DELETE(a) if((a)!=nullptr){delete (a);(a)=nullptr;}
#define SAFE_DELETE_ARRAY(a) if((a)!=nullptr){delete[](a);(a)=nullptr;}