module HAL:Resource;
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

	assert(false);

	return HAL::ResourceDesc::Buffer(0);
}


namespace HAL
{

	namespace API
	{


	void Resource::init(Device& device, const ResourceDesc& _desc, const PlacementAddress& address, ResourceState initialState)
	{
		auto THIS = static_cast<HAL::Resource*>(this);


		THIS->desc = _desc;

//		Log::get() << "creating resource " << _desc << Log::endl;
		CD3DX12_RESOURCE_DESC resourceDesc = to_native(THIS->desc);

		if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{

			if ((resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				resourceDesc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
		}

		D3D12_RESOURCE_ALLOCATION_INFO info = device.native_device->GetResourceAllocationInfo(0, 1, &resourceDesc);
		if (info.SizeInBytes == UINT64_MAX)
		{
			resourceDesc.Alignment = 0;
			info = device.native_device->GetResourceAllocationInfo(0, 1, &resourceDesc);
		}
		resourceDesc.Alignment = info.Alignment;

		D3D12_CLEAR_VALUE value;

		D3D12_CLEAR_VALUE *pass_value=nullptr;

		if (THIS->desc.is_texture())
		{

			auto texture_desc = THIS->desc.as_texture();
			assert((texture_desc.is1D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D)
				|| (texture_desc.is2D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
				|| (texture_desc.is3D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			);

			if (check(THIS->desc.Flags & HAL::ResFlags::RenderTarget))
			{
				value.Format = to_native(texture_desc.Format.to_srv());
				value.Color[0] = 0;
				value.Color[1] = 0;
				value.Color[2] = 0;
				value.Color[3] = 0;
				pass_value = &value;
			}
		
			if (check(THIS->desc.Flags & HAL::ResFlags::DepthStencil))
			{
				value.Format = to_native(texture_desc.Format.to_dsv());
				value.DepthStencil.Depth = 1.0f;
				value.DepthStencil.Stencil = 0;
				pass_value = &value;
			}

			if (!texture_desc.Format.is_shader_visible())
			{
				THIS->desc.Flags &= (~HAL::ResFlags::ShaderResource);
			}


		}

		
		if (address.heap)
		{
			TEST(device, device.native_device->CreatePlacedResource(
				address.heap->native_heap.Get(),
				address.offset,
				&resourceDesc,
				static_cast<D3D12_RESOURCE_STATES>(initialState),
				pass_value,
				IID_PPV_ARGS(&native_resource)));
		}
		else
		{
			if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
				//resourceDesc.Alignment = 4 * 1024;
			}

			TEST(device, device.native_device->CreateReservedResource(
				&resourceDesc,
				static_cast<D3D12_RESOURCE_STATES>(initialState),
				pass_value,
				IID_PPV_ARGS(&native_resource)));
		}

		THIS->desc = extract(native_resource);
		if (THIS->desc.is_buffer())
			this->address = GPUAddressPtr(native_resource->GetGPUVirtualAddress());
		else
			this->address = 0;

	}
	void Resource::init(D3D::Resource  resource)
	{
		auto THIS = static_cast<HAL::Resource*>(this);

		native_resource = resource;
		THIS->desc = extract(resource);
	}
}





	void Resource::_init(const ResourceDesc& desc, HeapType _heap_type, ResourceState state, vec4 clear_value)
	{
		auto t = CounterManager::get().start_count<Resource>();
		heap_type = _heap_type;
		//D3D12_CLEAR_VALUE value;

		//if (desc.is_texture())
		//{

		//	auto texture_desc = desc.as_texture();
		//	value.Format = to_srv(to_native(texture_desc.Format));
		//	value.Color[0] = clear_value.x;
		//	value.Color[1] = clear_value.y;
		//	value.Color[2] = clear_value.z;
		//	value.Color[3] = clear_value.w;

		//	if (check(desc.Flags & HAL::ResFlags::DepthStencil))
		//	{
		//		value.Format = to_dsv(to_native(texture_desc.Format));
		//		value.DepthStencil.Depth = 1.0f;
		//		value.DepthStencil.Stencil = 0;
		//	}


		//	if (!texture_desc.Format.is_shader_visible())
		//	{
		//		assert(false);
		//		//desc.Flags = desc.Flags & (~HAL::ResFlags::ShaderResource);
		//	}
		//}



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

		alloc_info = HAL::Device::get().get_alloc_info(desc);

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
			HeapIndex index = { HAL::MemoryType::COMMITED , heap_type };

			alloc_handle = Device::get().get_static_gpu_data().alloc_memory(alloc_info.size, alloc_info.alignment, index);
			address = { alloc_handle.get_heap().get(),alloc_handle.get_offset() };


			//	assert(0);
				/*	TEST(Device::get().get_native_device()->CreatePlacedResource(
						alloc_handle.get_heap()->get_native().Get(),
						alloc_handle.get_offset(),
						&desc,
						static_cast<D3D12_RESOURCE_STATES>(state),
						(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
						IID_PPV_ARGS(&resource)));*/


		}
		init(HAL::Device::get(), desc, address, state);

		state_manager.init_subres(HAL::Device::get().Subresources(get_desc()), state);
			if (heap_type == HeapType::RESERVED) 
		tiled_manager.init_tilings();

		gpu_address = ResourceAddress{ this,0 };
		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}
	Resource::Resource(const ResourceDesc& desc, HeapType heap_type, ResourceState state, vec4 clear_value) :state_manager(this), tiled_manager(this)
	{
		_init(desc, heap_type, state, clear_value);

	}

	Resource::Resource(const ResourceDesc& desc, ResourceHandle handle, bool own) :state_manager(this), tiled_manager(this)
	{
		auto t = CounterManager::get().start_count<Resource>();

		//tmp_handle = handle;
	/*	D3D12_CLEAR_VALUE value;

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



		}*/


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
		PlacementAddress address = { handle.get_heap().get(),handle.get_offset() };

		init(HAL::Device::get(), desc, address, state);

		state_manager.init_subres(HAL::Device::get().Subresources(get_desc()), state);

		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}

		if (own)
		{
			alloc_handle = handle;
		}

		gpu_address = ResourceAddress{ this,0 };

	}
	Resource::Resource(const ResourceDesc& desc, PlacementAddress address) :state_manager(this), tiled_manager(this)
	{
		auto t = CounterManager::get().start_count<Resource>();

		//tmp_handle = handle;
	/*	D3D12_CLEAR_VALUE value;

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



		}*/


		ResourceState state;


		if (check(desc.Flags & HAL::ResFlags::DepthStencil))
			state = ResourceState::DEPTH_WRITE;
		else if (check(desc.Flags & HAL::ResFlags::RenderTarget))
			state = ResourceState::RENDER_TARGET;
		else if (check(desc.Flags & HAL::ResFlags::ShaderResource))
			state = ResourceState::PIXEL_SHADER_RESOURCE;

		else
			state = ResourceState::COPY_DEST;


		heap_type = address.heap->get_type();

		if (heap_type == HeapType::UPLOAD)
		{
			state = ResourceState::GEN_READ;
		}if (heap_type == HeapType::READBACK)
		{
			state = ResourceState::COPY_DEST;
		}
		init(HAL::Device::get(), desc, address, state);

		state_manager.init_subres(HAL::Device::get().Subresources(get_desc()), state);

		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}



		gpu_address = ResourceAddress{ this,0 };

	}


	Resource::Resource(const D3D::Resource& resource, ResourceState state) :state_manager(this), tiled_manager(this)
	{
		init(resource);

		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		get_dx()->GetHeapProperties(&HeapProperties, &HeapFlags);

		heap_type = from_native(HeapProperties.Type);

		state_manager.init_subres(HAL::Device::get().Subresources(get_desc()), state);

		if (HeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD || HeapProperties.Type == D3D12_HEAP_TYPE_READBACK)
		{
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}

		gpu_address = ResourceAddress{ this,0 };

	}
	void Resource::set_name(std::string name)
	{
		this->name = name;
		get_dx()->SetName(convert(name).c_str());

	}

	Resource::~Resource()
	{
		if (buffer_data)
		{
			get_dx()->Unmap(0, nullptr);
		}

		alloc_handle.Free();
	}
	std::span<std::byte> Resource::cpu_data()const
	{

		return { buffer_data,buffer_data + get_size() };
	}

	std::byte* ResourceAddress::get_cpu_data() const
	{
		return resource->cpu_data().data() + resource_offset;
	}
		



}

HAL::GPUAddressPtr to_native(const HAL::ResourceAddress& address)
{
	return address.resource ? (address.resource->get_address() + address.resource_offset) : 0;
}