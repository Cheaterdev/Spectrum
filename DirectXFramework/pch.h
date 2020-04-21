#pragma once
#include "../Core/pch.h"
#include "../FileSystem/pch.h"

// DirectX includes
#define USE_D3D_COMPILER
#include <d3dcommon.h>

#include <d3d11.h>
#include <d3d11shader.h>


#include <d3d12.h>
#include <dxgi1_5.h>
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

	typedef D3D12_VIEWPORT Viewport;
    class CommandList;


}
enum class Layouts : int
{
	UNKNOWN = -1,
	FrameLayout = 0,
	DefaultLayout = 1,
	TOTAL = 2
};


#include "InputLayouts.h"


#include "DX12/RootSignature.h"
#include "DX12/Descriptors.h"

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

#include "DX12/RayTracingAS.h"



namespace Render
{
    using namespace DX12;

    typedef CommandList::ptr Context;
	using Bindless = std::vector<Handle>;

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



template<class Slot>
struct CompiledData
{
	D3D12_GPU_VIRTUAL_ADDRESS cb = 0;
	Render::HandleTableLight table_srv;
	Render::HandleTableLight table_uav;
	Render::HandleTableLight table_smp;

	void set(Render::SignatureDataSetter& graphics, bool use_transitions = true)
	{
		if (use_transitions) {
			for (int i = 0; i < table_srv.get_count(); ++i)
			{
				auto h = table_srv[i];
				if (h.resource_ptr && *h.resource_ptr)
					if ((*h.resource_ptr)->get_heap_type() == Render::HeapType::DEFAULT)
						graphics.get_base().transition(*h.resource_ptr, Render::ResourceState::PIXEL_SHADER_RESOURCE | Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

			}

			for (int i = 0; i < table_uav.get_count(); ++i)
			{
				auto h = table_uav[i];
				if (h.resource_ptr && *h.resource_ptr)
					if ((*h.resource_ptr)->get_heap_type() == Render::HeapType::DEFAULT)
						graphics.get_base().transition(*h.resource_ptr, Render::ResourceState::UNORDERED_ACCESS);
			}

		}
		if (table_srv.get_count() > 0) graphics.set(Slot::SRV_ID, table_srv);
		if (table_smp.get_count() > 0) graphics.set(Slot::SMP_ID, table_smp);
		if (table_uav.get_count() > 0) graphics.set(Slot::UAV_ID, table_uav);
		if (cb)
			graphics.set_const_buffer(Slot::CB_ID, cb);

	}

};


struct Empty
{};

using DefaultCB = D3D12_GPU_VIRTUAL_ADDRESS;// std::vector<std::byte>;

template<typename T> concept HasSRV =
requires (T t){
	t.srv;
};


template<typename T> concept HasBindless =
requires (T t) {
	t.bindless;
};



template<typename T> concept HasUAV =
requires (T t) {
	t.uav;
};


template<typename T> concept HasSMP =
requires (T t) {
	t.smp;
};


template<typename T> concept HasCB =
requires (T t) {
	t.cb;
};

template<class Table, class Slot = Table::Slot>
struct DataHolder : public Table
{
	using Table::Table;


	template<class SRV>
	void place_srv(CompiledData<Slot> & compiled, Render::SignatureDataSetter& context, SRV& srv)
	{
		compiled.table_srv = context.get_base().srv_descriptors.place(sizeof(srv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&srv);
		for (int i = 0; i < compiled.table_srv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_srv[i].place(*handle);
		}
	}

	template<class SRV>
	void place_srv(CompiledData<Slot>& compiled, Render::SignatureDataSetter& context, SRV& srv, Render::Bindless &bindless)
	{
		compiled.table_srv = context.get_base().srv_descriptors.place(sizeof(srv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&srv);
		for (int i = 0; i < compiled.table_srv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_srv[i].place(*handle);
		}
	}

	template<class UAV>
	void place_uav(CompiledData<Slot>& compiled, Render::SignatureDataSetter& context, UAV& uav)
	{
		compiled.table_uav = context.get_base().srv_descriptors.place(sizeof(uav) / sizeof(Render::Handle));

		auto ptr = reinterpret_cast<Render::Handle*>(&uav);
		for (int i = 0; i < sizeof(uav) / sizeof(Render::Handle); i++)
		{
			Render::Handle* handle = ptr + i;
			if (ptr[i].cpu.ptr != 0)
			compiled.table_uav[i].place(*handle);
		}
	}
	
	template<class SMP>
	void place_smp(CompiledData<Slot>& compiled, Render::SignatureDataSetter& context, SMP& smp)
	{
		compiled.table_smp = context.get_base().smp_descriptors.place(sizeof(smp) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&srv);
		for (int i = 0; i < compiled.table_smp.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_smp[i].place(*handle);
		}
	}

	
	template<class CB>
	void place_cb(CompiledData<Slot>& compiled, Render::SignatureDataSetter & context, CB& cb)
	{
		compiled.cb = context.get_base().place_raw(cb).get_address();
	}
	
	template<>
	void place_cb<D3D12_GPU_VIRTUAL_ADDRESS>(CompiledData<Slot>& compiled, Render::SignatureDataSetter& context, D3D12_GPU_VIRTUAL_ADDRESS& cb)
	{
		compiled.cb = cb;
	}
	CompiledData<Slot> set(Render::SignatureDataSetter& context, bool use_transitions = true)
	{
		CompiledData<Slot> compiled;
		

		if constexpr (HasSRV<Table>|| HasBindless<Table>)
		{
			int srv_count = 0;
			if constexpr (HasSRV<Table>) srv_count += sizeof(srv) / sizeof(Render::Handle);
			if constexpr (HasBindless<Table>) srv_count += bindless.size();


			compiled.table_srv = context.get_base().srv_descriptors.place(srv_count);
			int offset = 0;
			if constexpr (HasSRV<Table>) {		
				auto ptr = reinterpret_cast<Render::Handle*>(&srv);
				for (int i = 0; i < sizeof(srv) / sizeof(Render::Handle); i++)
				{
					if (ptr[i].cpu.ptr != 0)
						compiled.table_srv[offset++].place(ptr[i]);
					else
						offset++;
					//else

				}
			}

			if constexpr (HasBindless<Table>) {
				for (int j = 0; j < bindless.size(); j++)
				{
					compiled.table_srv[offset++].place(bindless[j]);
				}
			}
		}

		if constexpr (HasUAV<Table>)
			place_uav(compiled, context, Table::uav);

		if constexpr (HasSMP<Table>)
			place_smp(compiled, context, Table::smp);

		if constexpr (HasCB<Table>)
			place_cb(compiled, context, Table::cb);

		compiled.set(context,use_transitions);
		return compiled;
	}


};

template<class T>
struct AutoGenSignatureDesc
{
	Render::RootSignatureDesc desc;

	
	template<class T>
	void process_one()
	{
		if constexpr (T::SRV)	desc[T::SRV_ID] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, -1, T::ID);
		if constexpr (T::UAV)	desc[T::UAV_ID] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, T::UAV, T::ID);
		if constexpr (T::SMP)	desc[T::SMP_ID] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, T::SMP, T::ID);
		desc[T::CB_ID] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL, T::ID);
	}

	template< class ...A>
	void process(std::initializer_list< Render::Samplers::SamplerDesc> samplers)
	{

		(process_one<A>(), ...);
		for (auto& s : samplers)
		{
			desc.add_sampler(0, Render::ShaderVisibility::ALL, s);
		}
	}

	AutoGenSignatureDesc()
	{
		T::for_each(*this);

	}


	Render::RootLayout::ptr create_signature(Layouts layout)
	{
		return std::make_shared<Render::RootLayout>(desc, layout);
	}
};


using uint = UINT;
using float4x4 = mat4x4;

#include "../RenderSystem/autogen/includes.h"
