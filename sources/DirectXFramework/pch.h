#pragma once
#include "../Core/pch.h"

// DirectX includes
#include <d3dcompiler.h>

#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>
#include <pix.h>
using namespace Microsoft::WRL;

#include "helper.h"
#include "d3dx12.h"

#include <DirectXTex.h>

#include "dxc/dxcapi.use.h"

//#include "../3rdparty/DirectXTex/ScreenGrab/ScreenGrab.h"

HRESULT device_fail();
#ifdef  DEBUG
#define TEST(x)\
	assert(SUCCEEDED(x))
#else
#define TEST(x)\
	([&](){\
	HRESULT hr = x; \
	if(hr==0x887a0005) device_fail(); \
	 test(hr, STRINGIZE(x)); \
if (FAILED(hr)) \
	{__debugbreak(); \
	assert(false); }\
	return hr; \
	})()

#endif //  DEBUG

namespace DX12
{
	
}
namespace Render
{
	using namespace DX12;
}





