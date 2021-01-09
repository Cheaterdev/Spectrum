#pragma once
#include "../Core/pch.h"
#include "../FileSystem/pch.h"

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
	HRESULT hr = test(x, __FUNCSIG__); \
	if(hr==0x887a0005) \
	hr=device_fail(); \
if (FAILED(hr)) \
	assert(false); \
	return hr; \
	})()

#endif //  DEBUG

#define shader_struct __declspec(align(16)) struct


#include "D3D\Shaders.h"
#include "D3D\ShaderReflection.h"


#include "Resource\TextureData\Header.h"
#include "D3D/TextureLoader.h"


namespace DX12
{

	typedef D3D12_VIEWPORT Viewport;
	class CommandList;
}

#include "../RenderSystem/autogen/enums.h"


#include "InputLayouts.h"

#include "SIG/Concepts.h"

#include "DX12/RootSignature.h"
#include "DX12/Descriptors.h"

#include "DX12/Memory.h"
#include "DX12/Tiling.h"

#include "DX12/Fence.h"
#include "DX12/States.h"
#include "DX12/Resource.h"
#include "DX12/ResourceViews.h"


#include "DX12/IndirectCommand.h"
#include "DX12/CommandList.h"

#include "DX12/Swapchain12.h"

#include "DX12/Queue.h"
#include "DX12/Device12.h"


#include "D3D/ShaderReflection.h"
#include "DX12/Shader.h"

#include "DX12/Texture.h"
#include "DX12/PipelineState.h"
#include "DX12/Buffer.h"
#include "DX12/GPUTimer.h"
#include "DX12/Samplers.h"

#include "DX12/RayTracingAS.h"



namespace Render
{
	using namespace DX12;

	typedef CommandList::ptr Context;
}


#include "FW1FontWrapper/Source/FW1CompileSettings.h"
#include "FW1FontWrapper/Source/FW1FontWrapper.h"


typedef CComPtr<IFW1Factory>			FW1_Factory;
typedef CComPtr<IFW1FontWrapper>		FW1_Font;
typedef CComPtr<IFW1TextGeometry>		FW1_TextGeometry;


#include "SIG/SIG.h"
#include "SIG/RT.h"
#include "SIG/Layout.h"
#include "SIG/Slots.h"
#include "SIG/PSO.h"

#include "../RenderSystem/autogen/includes.h"


#include "DX12/PipelineState.hpp"
#include "DX12/Tiling.hpp"