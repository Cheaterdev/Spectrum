#include "pch_dx.h"



#include "helper.h"
import Resource;
import Debug;
 //import Memory;
 import Tiling;
 import Fence;
 import States;
 import Device;
import stl.core;
import stl.memory;

namespace DX12
{
	TrackedResource::~TrackedResource()
	{
		alloc_handle.Free();
	}

	static std::atomic_size_t counter[3] = { 0, 0, 0 };
	std::atomic_size_t counter_id;


	void Resource::init(const CD3DX12_RESOURCE_DESC& _desc, HeapType _heap_type, ResourceState state, vec4 clear_value)
	{
		CD3DX12_RESOURCE_DESC desc = _desc;
		ComPtr<ID3D12Resource> resource;

		auto t = CounterManager::get().start_count<Resource>();
		heap_type = _heap_type;
		D3D12_CLEAR_VALUE value;
		value.Format = to_srv(desc.Format);
		value.Color[0] = clear_value.x;
		value.Color[1] = clear_value.y;
		value.Color[2] = clear_value.z;
		value.Color[3] = clear_value.w;

		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			value.Format = to_dsv(desc.Format);
			value.DepthStencil.Depth = 1.0f;
			value.DepthStencil.Stencil = 0;
		}

		if (!is_shader_visible(desc.Format))
			desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		if (state == ResourceState::UNKNOWN)
		{
			if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				state = ResourceState::DEPTH_WRITE;
			else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				state = ResourceState::RENDER_TARGET;
			else if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
				state = ResourceState::PIXEL_SHADER_RESOURCE;

			else
				state = ResourceState::COMMON;
		}


		if (heap_type == HeapType::RESERVED)
		{
			if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
			{
				desc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
				desc.Alignment = 4*1024;
			}
			TEST(Device::get().get_native_device()->CreateReservedResource(
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&resource)));


		}
		else
		{
			auto info = Device::get().get_alloc_info(desc);

			tracked_info->alloc_handle = ResourceHeapPageManager::get().alloc(info.size, info.alignment, info.flags, heap_type);

			if (heap_type == HeapType::UPLOAD)
			{
				state = ResourceState::GEN_READ;
			}if (heap_type == HeapType::READBACK)
			{
				state = ResourceState::COPY_DEST;
			}


			TEST(Device::get().get_native_device()->CreatePlacedResource(
				tracked_info->alloc_handle.get_heap()->get_native().Get(),
				tracked_info->alloc_handle.get_offset(),
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&resource)));


		}

		alloc_info = Device::get().get_alloc_info(desc);
		tracked_info->set_resource(resource);

		id = counter_id.fetch_add(1);

		resource->SetName(std::to_wstring(id).c_str());
		//	Log::get() << "resource creation: " << id << " at:\n" << get_stack_trace().to_string() << Log::endl;
		this->desc = CD3DX12_RESOURCE_DESC(resource->GetDesc());

		if (heap_type != HeapType::READBACK && desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			gpu_adress = resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()), state);

		init_tilings();


		if (heap_type == HeapType::UPLOAD|| heap_type == HeapType::READBACK)
		{
			resource->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}
	Resource::Resource(const CD3DX12_RESOURCE_DESC& desc, HeapType heap_type, ResourceState state, vec4 clear_value)
	{
		init(desc, heap_type, state, clear_value);
	}

	Resource::Resource(const CD3DX12_RESOURCE_DESC& desc, ResourceHandle handle, bool own)
	{
		auto t = CounterManager::get().start_count<Resource>();

		ComPtr<ID3D12Resource> resource;


		//tmp_handle = handle;
		D3D12_CLEAR_VALUE value;

		value.Format = to_srv(desc.Format);

		value.Color[0] = 0;
		value.Color[1] = 0;
		value.Color[2] = 0;
		value.Color[3] = 0;

		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			value.Format = to_dsv(desc.Format);
			value.DepthStencil.Depth = 1.0f;
			value.DepthStencil.Stencil = 0;
		}
		ResourceState state;


		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
			state = ResourceState::DEPTH_WRITE;
		else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
			state = ResourceState::RENDER_TARGET;
		else  if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
			state = ResourceState::PIXEL_SHADER_RESOURCE;
		else state = ResourceState::COPY_DEST;

		heap_type = handle.get_heap()->get_type();

		if (heap_type == HeapType::UPLOAD)
		{
			state = ResourceState::GEN_READ;
		}if (heap_type == HeapType::READBACK)
		{
			state = ResourceState::COPY_DEST;
		}
		TEST(Device::get().get_native_device()->CreatePlacedResource(
			handle.get_heap()->get_native().Get(),
			handle.get_offset(),
			&desc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
			IID_PPV_ARGS(&resource)));

		tracked_info->set_resource(resource);
		id = counter_id.fetch_add(1);

		resource->SetName(std::to_wstring(id).c_str());
		this->desc = CD3DX12_RESOURCE_DESC(resource->GetDesc());

		if (heap_type != HeapType::READBACK && desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			gpu_adress = resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()), state);

		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			resource->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}

		if(own)
		{
			tracked_info->alloc_handle = handle;
		}
	}


	Resource::Resource(const ComPtr<ID3D12Resource>& resource, ResourceState state, bool own)
	{
		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		resource->GetHeapProperties(&HeapProperties, &HeapFlags);

		heap_type = (HeapType)HeapProperties.Type;
		tracked_info->set_resource(resource);
		desc = CD3DX12_RESOURCE_DESC(resource->GetDesc());
		force_delete = !own;
		id = counter_id.fetch_add(1);

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			gpu_adress = resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()), state);

		if (HeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD || HeapProperties.Type == D3D12_HEAP_TYPE_READBACK)
		{
			resource->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}
	void Resource::set_name(std::string name)
	{
		this->name = name;
		tracked_info->m_Resource->SetName(convert(name).c_str());

		debug = name == "gi_filtered_reflection";

		tracked_info->debug = debug;
	}

	Resource::~Resource()
	{
		if (buffer_data)
		{
			tracked_info->m_Resource->Unmap(0, nullptr);
		}
#ifdef DEV
		std::lock_guard<std::mutex> g(m);

		assert(lists.empty());
#endif 

	}

}
