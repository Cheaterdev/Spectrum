module HAL:Resource;


import <d3d12/d3d12_includes.h>;
import wrl;

import d3d12;
import Core;

import :FrameManager;
#undef THIS


HAL::ResourceDesc extract(D3D::Resource resource)
{
	auto native_desc = resource->GetDesc();

	if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		return HAL::ResourceDesc::Buffer(native_desc.Width, from_native(native_desc.Flags));
	}
	else
	{
		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			return  HAL::ResourceDesc::Tex1D(from_native(native_desc.Format), { uint(native_desc.Width) }, native_desc.DepthOrArraySize, native_desc.MipLevels, from_native(native_desc.Flags));
		}

		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			return  HAL::ResourceDesc::Tex2D(from_native(native_desc.Format), { uint(native_desc.Width), native_desc.Height }, native_desc.DepthOrArraySize, native_desc.MipLevels, from_native(native_desc.Flags));
		}

		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			return  HAL::ResourceDesc::Tex3D(from_native(native_desc.Format), { uint(native_desc.Width), native_desc.Height, native_desc.DepthOrArraySize }, native_desc.MipLevels, from_native(native_desc.Flags));
		}
	}

	ASSERT(false);

	return HAL::ResourceDesc::Buffer(0);
}


namespace HAL
{

	namespace API
	{
		GPUAddressPtr Resource::get_address() { return address; }

		void Resource::init(Device& device, const ResourceDesc& _desc, const PlacementAddress& address, TextureLayout initialLayout)
		{
			auto THIS = static_cast<HAL::Resource*>(this);
			auto& desc = THIS->desc;
			auto& heap_type = THIS->heap_type;

			if(address.heap)
			heap_type = address.heap->get_type();
			else
				heap_type = HAL::HeapType::RESERVED;
			desc = _desc;

			//		Log::get() << "creating resource " << _desc << Log::endl;
			auto resourceDesc = to_native(desc);
	


			ResourceAllocationInfo info = device.get_alloc_info(_desc);

			resourceDesc.Alignment = info.alignment;

			D3D12_CLEAR_VALUE value;

			D3D12_CLEAR_VALUE* pass_value = nullptr;


			std::vector<DXGI_FORMAT> castable_formats;




			if (desc.is_texture())
			{

				auto texture_desc = desc.as_texture();
				//			castable_formats.emplace_back(to_native(texture_desc.Format));
				//	castable_formats.emplace_back(to_native(texture_desc.Format.to_srv()));
				//	castable_formats.emplace_back(to_native(texture_desc.Format.to_dsv()));


				ASSERT((texture_desc.is1D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D)
					|| (texture_desc.is2D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
					|| (texture_desc.is3D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D)
				);

				if (check(desc.Flags & HAL::ResFlags::RenderTarget))
				{
					value.Format = to_native(texture_desc.Format.to_srv());
					value.Color[0] = 0;
					value.Color[1] = 0;
					value.Color[2] = 0;
					value.Color[3] = 0;
					pass_value = &value;
				}

				if (check(desc.Flags & HAL::ResFlags::DepthStencil))
				{
					value.Format = to_native(texture_desc.Format.to_dsv());
					value.DepthStencil.Depth = 0.0f;
					value.DepthStencil.Stencil = 0;
					pass_value = &value;
				}

				if (!texture_desc.Format.is_shader_visible())
				{
					THIS->desc.Flags &= (~HAL::ResFlags::ShaderResource);
				}



				if (heap_type == HeapType::UPLOAD)
				{
					initialLayout = TextureLayout::SHADER_RESOURCE | TextureLayout::COPY_SOURCE;
				}
				else if (heap_type == HeapType::READBACK)
				{
					initialLayout = TextureLayout::COPY_DEST;
				}
				else 	if (initialLayout == TextureLayout::UNDEFINED)
				{
					if (check(desc.Flags & HAL::ResFlags::ShaderResource))
						initialLayout = TextureLayout::SHADER_RESOURCE;
					else if (check(desc.Flags & HAL::ResFlags::UnorderedAccess))
						initialLayout = TextureLayout::UNORDERED_ACCESS;
					else if (check(desc.Flags & HAL::ResFlags::DepthStencil))
						initialLayout = TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::DEPTH_STENCIL_READ;
					else if (check(desc.Flags & HAL::ResFlags::RenderTarget))
						initialLayout = TextureLayout::RENDER_TARGET;
					else
						initialLayout = TextureLayout::COPY_DEST; // probably update from CPU or copy
				}


			if(check(_desc.Flags&ResFlags::DisableStateTracking))
			{
				if(check(_desc.Flags&ResFlags::ShaderResource))
					initialLayout = TextureLayout::SHADER_RESOURCE;// | TextureLayout::COPY_SOURCE;
				else
				   initialLayout = TextureLayout::COPY_SOURCE;
			}


			}
			else
			{
				initialLayout = TextureLayout::UNDEFINED;
			}


			
			D3D12_BARRIER_LAYOUT layout = to_native(initialLayout);

			if (address.heap)
			{
				TEST(device, device.native_device->CreatePlacedResource2(
					address.heap->native_heap.Get(),
					address.offset,
					&resourceDesc,
					layout,
					pass_value,
					uint(castable_formats.size()), castable_formats.data(),
					IID_PPV_ARGS(&native_resource)));
			}
			else
			{
						auto resourceDesc1 = to_native_1(desc);

				if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
				{
					resourceDesc1.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
					//resourceDesc.Alignment = 4 * 1024;
				}

				TEST(device, device.native_device->CreateReservedResource2(
					&resourceDesc1,
					layout,
					pass_value,
					nullptr,
					uint(castable_formats.size()), castable_formats.data(),
					IID_PPV_ARGS(&native_resource)));
			}
			   	auto prev_flags = THIS->desc.Flags;
			init(native_resource, initialLayout, device);
			THIS->desc.Flags |= 	prev_flags;
		}

		void Resource::init(D3D::Resource  resource, TextureLayout layout, Device& device)
		{
			auto THIS = static_cast<HAL::Resource*>(this);
			THIS->m_device = static_cast<HAL::Device*>(&device);


			THIS->desc = extract(native_resource);

			if(layout == TextureLayout::PRESENT)
				THIS->desc.Flags |= ResFlags::Swapchain;

			if (THIS->desc.is_buffer())
				this->address = GPUAddressPtr(native_resource->GetGPUVirtualAddress());
			else
				this->address = 0;

			THIS->state_manager.init_subres(device.Subresources(THIS->get_desc()), layout);


			if (THIS->heap_type == HeapType::RESERVED)
				THIS->tiled_manager.init_tilings();


		}
	}





	void Resource::_init(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout, vec4 clear_value)
	{
		auto t = CounterManager::get().start_count<Resource>();
		m_device = &device;

		alloc_info = device.get_alloc_info(desc);

		PlacementAddress address = {};
		if (heap_type != HAL::HeapType::RESERVED)
		{
			HeapIndex index = { HAL::MemoryType::COMMITED , heap_type };

			alloc_handle = device.get_static_gpu_data().alloc_memory(alloc_info.size, alloc_info.alignment, index);
			address = { alloc_handle.get_heap().get(),alloc_handle.get_offset() };
		}

		init(device, desc, address, initialLayout);
	}
	Resource::Resource(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout, vec4 clear_value) :state_manager(this), tiled_manager(this)
	{
		_init(device, desc, heap_type, initialLayout, clear_value);

	}

	Resource::Resource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own) :state_manager(this), tiled_manager(this)
	{
		auto t = CounterManager::get().start_count<Resource>();
		m_device = &device;

		PlacementAddress address = { handle.get_heap().get(),handle.get_offset() };

		init(device, desc, address,  TextureLayout::UNDEFINED);

		if (own)
		{
			alloc_handle = handle;
		}
	}

	Resource::Resource(Device& device, const ResourceDesc& desc, PlacementAddress address) :state_manager(this), tiled_manager(this)
	{
		auto t = CounterManager::get().start_count<Resource>();
		m_device = &device;

		init(device, desc, address, TextureLayout::UNDEFINED);
	}


	Resource::Resource(Device& device, const D3D::Resource& resource, TextureLayout initialLayout) :state_manager(this), tiled_manager(this)
	{
		native_resource = resource;
		m_device = &device;

		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		get_dx()->GetHeapProperties(&HeapProperties, &HeapFlags);

		heap_type = from_native(HeapProperties.Type);


		init(native_resource, initialLayout, device);

	}

	void Resource::set_name(std::string name)
	{
		if(!this->name.empty()&&name.empty())	return;
		ASSERT(name.size()>0);
		this->name = name;
		get_dx()->SetName(convert(name).c_str());

	}

	Resource::~Resource()
	{
		alloc_handle.Free();
	}


}
