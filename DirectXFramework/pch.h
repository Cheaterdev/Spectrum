#pragma once
#include "../Core/pch.h"
#include "../FileSystem/pch.h"

// DirectX includes
#define USE_D3D_COMPILER
#include <d3dcommon.h>

#include <DXGI1_2.h>
#include <d3d11.h>
#include <d3d11shader.h>


#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <pix.h>
using namespace Microsoft::WRL;


#ifdef USE_D3D_COMPILER
#include <d3dcompiler.h>
#include "D3DCompiler.inl"
#endif
#include "helper.h"
#include "d3dx12.h"

#include <DirectXTex.h>


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


//HRESULT TEST(HRESULT hr);
namespace DX11
{
    class NativeContext;
    class NativeContextAccessor
    {
        public:
            virtual ~NativeContextAccessor()
            {
            }

        protected:
            static DX11_DeviceContext get_native_context(NativeContext& context);
    };

    class NativeContext
    {
            friend class NativeContextAccessor;
        protected:
            DX11_DeviceContext native_context;
            NativeContext(DX11_DeviceContext _native_context);
            DX11_DeviceContext get_native();
        public:
            int layout_optimized_count;
            int geometry_optimized_count;
            int pixel_optimized_count;
            int vertex_optimized_count;
            int state_optimized_count;
            int srv_optimized_count;
            int rtv_optimized_count;
            int const_buffer_optimized_count;
            int sampler_optimized_count;
            int topology_optimized_count;
            int viewports_optimized_count;
            int total_pipeline_count;
            void clear_optimized_count();
            int get_total_optimizations();

    };

#include "Resource/Resource.h"
#include "ResourceViews/RenderTarget.h"
#include "ResourceViews/ShaderResourceView.h"
#include "Resource/Texture2D.h"
#include "Resource/InputLayout.h"

#include "SwapChain/SwapChain.h"

#include "Resource/Buffer.h"
#include "Shaders/Shader.h"
#include "States/States.h"

#include "States/RenderTargetState.h"
#include "States/ShaderResourceState.h"
#include "States/ConstBufferState.h"
#include "States/SamplerState.h"

#include "States/PipelineState.h"
#include "Context/Context.h"
#include "Device/Device.h"


}

namespace DX12
{
    class CommandList;
}

#include "InputLayouts.h"


#include "DX12/RootSignature.h"
#include "DX12/Resource.h"
#include "DX12/CommandList.h"

#include "DX12/Swapchain12.h"
#include "DX12/Device12.h"


#include "D3D/ShaderReflection.h"
#include "DX12/Shader.h"

#include "DX12/Texture.h"
#include "DX12/PipelineState.h"
#include "DX12/Buffer.h"
#include "DX12/GPUTimer.h"
#include "DX12/Samplers.h"
#include "DX12/SignatureCreator.h"



namespace Render
{
    using namespace DX12;

    typedef CommandList::ptr Context;
}



#include "../3rdparty/FW1FontWrapper/Source/FW1CompileSettings.h"
#include "../3rdparty/FW1FontWrapper/Source/FW1FontWrapper.h"


typedef CComPtr<IFW1Factory>			FW1_Factory;
typedef CComPtr<IFW1FontWrapper>		FW1_Font;
typedef CComPtr<IFW1TextGeometry>		FW1_TextGeometry;

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")

#pragma comment(lib, "DirectXFramework")
#ifdef USE_D3D_COMPILER
#pragma comment(lib, "d3dcompiler")
#endif