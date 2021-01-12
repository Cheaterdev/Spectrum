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


#include <atlbase.h>

typedef ComPtr<IDXGISwapChain1>		DXGI_SwapChain;
typedef ComPtr<IDXGISurface2>			DXGI_Surface;
typedef ComPtr<IDXGIDevice2>			DXGI_Device;
typedef ComPtr<IDXGIAdapter2>			DXGI_Adapter;
typedef ComPtr<IDXGIFactory2>			DXGI_Factory;
typedef ComPtr<IDXGIOutput1>			DXGI_Output;



#define D3DCOMPILE_DEBUG                                (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                      (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION                    (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR                (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR             (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION                    (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT             (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT             (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                         (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL                   (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL                  (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS                    (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY       (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                      (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0                  (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1                  0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2                  ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3                  (1 << 15)
#define D3DCOMPILE_RESERVED16                           (1 << 16)
#define D3DCOMPILE_RESERVED17                           (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS                  (1 << 18)
#define D3DCOMPILE_RESOURCES_MAY_ALIAS                  (1 << 19)
#define D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES   (1 << 20)
#define D3DCOMPILE_ALL_RESOURCES_BOUND                  (1 << 21)
#define D3DCOMPILE_DEBUG_NAME_FOR_SOURCE                (1 << 22)
#define D3DCOMPILE_DEBUG_NAME_FOR_BINARY                (1 << 23)



// serialization

UINT BitsPerPixel(DXGI_FORMAT fmt);
DXGI_FORMAT to_srv(DXGI_FORMAT);
DXGI_FORMAT to_dsv(DXGI_FORMAT);
UINT get_default_mapping(DXGI_FORMAT);
DXGI_FORMAT to_linear(DXGI_FORMAT);
DXGI_FORMAT to_typeless(DXGI_FORMAT);

bool is_shader_visible(DXGI_FORMAT);
#define OP(x,y)\
if (l.x == r.x)\
{\
	y\
}\
else \
	return l.x < r.x;
#define OP_LAST(x,y)\
	return l.x < r.x;

namespace boost
{
	namespace serialization
	{

		template<class Archive>
		void serialize(Archive& ar, D3D12_DEPTH_STENCILOP_DESC& g, const unsigned int)
		{
			//	ar & g.DefaultValue;
			ar& g.StencilDepthFailOp;
			ar& g.StencilFailOp;
			ar& g.StencilFunc;
			ar& g.StencilPassOp;
		}

	}
}


/*
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
#include "DX12/Tiling.hpp"*/

namespace DX12 {
	typedef D3D12_VIEWPORT Viewport;
	class CommandList;
	class Resource;
	class Queue;
	class Device;
}
/*
namespace Render
{
	using namespace DX12;
}
*/
#define Render DX12