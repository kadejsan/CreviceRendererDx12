#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#include <wrl.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")

#ifdef _DEBUG
#	include "Initguid.h"
#	include "DXGIDebug.h"
#endif // _DEBUG