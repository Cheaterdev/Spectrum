module;

#include "helper.h"
module Graphics:Resource;

import :Tiling;
import :Fence;
import :States;
import :Device;

import Debug;
import Log;

import stl.core;
import stl.memory;
import HAL;
using namespace HAL;


namespace Graphics
{
	TrackedResource::~TrackedResource()
	{
		alloc_handle.Free();
	}

	static std::atomic_size_t counter[3] = { 0, 0, 0 };
	std::atomic_size_t counter_id;


	void Resource::init(const ResourceDesc& desc, HeapType _heap_type, ResourceState state, vec4 clear_value)
	{
		auto t = CounterManager::get().start_count<Resource>();
		heap_type = _heap_type;
		D3D12_CLEAR_VALUE value;

		if (desc.is_texture())
		{

			auto texture_desc = desc.as_texture();
			value.Format = to_srv(to_native(texture_desc.Format));
			value.Color[0] = clear_value.x;
			value.Color[1] = clear_value.y;
			value.Color[2] = clear_value.z;
			value.Color[3] = clear_value.w;

			if (check(desc.Flags & HAL::ResFlags::DepthStencil))
			{
				value.Format = to_dsv(to_native(texture_desc.Format));
				value.DepthStencil.Depth = 1.0f;
				value.DepthStencil.Stencil = 0;
			}


			if (!texture_desc.Format.is_shader_visible())
			{
				assert(false);
				//desc.Flags = desc.Flags & (~HAL::ResFlags::ShaderResource);
			}
		}



		/**/


		if (state == ResourceState::UNKNOWN)
		{
			if (check(desc.Flags & HAL::ResFlags::DepthStencil))
				state = ResourceState::DEPTH_WRITE;
			else if (check(desc.Flags & HAL::ResFlags::RenderTarget))
				state = ResourceState::RENDER_TARGET;
			else if (check(desc.Flags & HAL::ResFlags::ShaderResource))
				state = ResourceState::PIXEL_SHADER_RESOURCE;

			else
				state = ResourceState::COMMON;
		}
		if (heap_type == HeapType::UPLOAD)
		{
			state = ResourceState::GEN_READ;
		}if (heap_type == HeapType::READBACK)
		{
			state = ResourceState::COPY_DEST;
		}

		alloc_info = Device::get().get_alloc_info(desc);

		PlacementAddress address = {};


		if (heap_type == HAL::HeapType::RESERVED)
		{
			/*	if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
				{
					desc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
					desc.Alignment = 4 * 1024;
				}*/

				//Log::get() << "create " << desc.Dimension << Log::endl;

				//D3D12_RESOURCE_DESC ddd = desc;
				//Log::get() << "size " << ddd << Log::endl;
	//	assert(0);
			/*		TEST(Device::get().get_native_device()->CreateReservedResource(
						&desc,
						static_cast<D3D12_RESOURCE_STATES>(state),
						(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
						IID_PPV_ARGS(&resource)));*/


		}
		else
		{
			tracked_info->alloc_handle = ResourceHeapPageManager::get().alloc(alloc_info.size, alloc_info.alignment, D3D12_HEAP_FLAGS(alloc_info.flags), heap_type);
			address = { tracked_info->alloc_handle.get_heap()->get_hal().get(),tracked_info->alloc_handle.get_offset() };


			//	assert(0);
				/*	TEST(Device::get().get_native_device()->CreatePlacedResource(
						tracked_info->alloc_handle.get_heap()->get_native().Get(),
						tracked_info->alloc_handle.get_offset(),
						&desc,
						static_cast<D3D12_RESOURCE_STATES>(state),
						(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
						IID_PPV_ARGS(&resource)));*/


		}
		HAL::Resource::ptr resource = std::make_shared<HAL::Resource>(*Device::get().get_hal_device(), desc, address, state);

		resource->user_data = this;
		tracked_info->set_resource(resource);

		id = counter_id.fetch_add(1);

		get_dx()->SetName(std::to_wstring(id).c_str());
		//	Log::get() << "resource creation: " << id << " at:\n" << get_stack_trace().to_string() << Log::endl;

		if (heap_type != HeapType::READBACK && desc.is_buffer())
			gpu_adress = get_dx()->GetGPUVirtualAddress();

		init_subres(Device::get().get_hal_device()->Subresources(get_desc()), state);

		init_tilings();


		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}
	Resource::Resource(const ResourceDesc& desc, HeapType heap_type, ResourceState state, vec4 clear_value)
	{
		init(desc, heap_type, state, clear_value);
	}

	Resource::Resource(const ResourceDesc& desc, ResourceHandle handle, bool own)
	{
		auto t = CounterManager::get().start_count<Resource>();

		//tmp_handle = handle;
		D3D12_CLEAR_VALUE value;

		if (desc.is_texture())
		{

			auto texture_desc = desc.as_texture();
			value.Format = to_srv(to_native(texture_desc.Format));
			value.Color[0] = 0;
			value.Color[1] = 0;
			value.Color[2] = 0;
			value.Color[3] = 0;

			if (check(desc.Flags & HAL::ResFlags::DepthStencil))
			{
				value.Format = to_dsv(to_native(texture_desc.Format));
				value.DepthStencil.Depth = 1.0f;
				value.DepthStencil.Stencil = 0;
			}



		}


		ResourceState state;


		if (check(desc.Flags & HAL::ResFlags::DepthStencil))
			state = ResourceState::DEPTH_WRITE;
		else if (check(desc.Flags & HAL::ResFlags::RenderTarget))
			state = ResourceState::RENDER_TARGET;
		else if (check(desc.Flags & HAL::ResFlags::ShaderResource))
			state = ResourceState::PIXEL_SHADER_RESOURCE;

		else
			state = ResourceState::COPY_DEST;


		heap_type = handle.get_heap()->get_type();

		if (heap_type == HeapType::UPLOAD)
		{
			state = ResourceState::GEN_READ;
		}if (heap_type == HeapType::READBACK)
		{
			state = ResourceState::COPY_DEST;
		}
		PlacementAddress address = { handle.get_heap()->get_hal().get(),handle.get_offset() };

		HAL::Resource::ptr resource = std::make_shared<HAL::Resource>(*Device::get().get_hal_device(), desc, address, state);
		resource->user_data = this;
		tracked_info->set_resource(resource);
		id = counter_id.fetch_add(1);

		get_dx()->SetName(std::to_wstring(id).c_str());

		if (heap_type != HeapType::READBACK && desc.is_buffer())
			gpu_adress = get_dx()->GetGPUVirtualAddress();

		init_subres(Device::get().get_hal_device()->Subresources(get_desc()), state);

		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}

		if (own)
		{
			tracked_info->alloc_handle = handle;
		}
	}


	Resource::Resource(const HAL::Resource::ptr& resource, ResourceState state, bool own)
	{
		resource->user_data = this;
		tracked_info->set_resource(resource);


		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		get_dx()->GetHeapProperties(&HeapProperties, &HeapFlags);

		heap_type = (HeapType)HeapProperties.Type;
		force_delete = !own;
		id = counter_id.fetch_add(1);

		if (get_desc().is_buffer())
			gpu_adress = get_dx()->GetGPUVirtualAddress();

		init_subres(Device::get().get_hal_device()->Subresources(get_desc()), state);

		if (HeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD || HeapProperties.Type == D3D12_HEAP_TYPE_READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}
	void Resource::set_name(std::string name)
	{
		this->name = name;
		//get_dx()->SetName(convert(name).c_str());

		debug = name == "gi_filtered_reflection";

		tracked_info->debug = debug;
	}

	Resource::~Resource()
	{
		if (buffer_data)
		{
			get_dx()->Unmap(0, nullptr);
		}
#ifdef DEV
		std::lock_guard<std::mutex> g(m);

		assert(lists.empty());
#endif 

	}

}
