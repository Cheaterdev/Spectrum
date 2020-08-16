#include "pch.h"

namespace DX12
{

	void ResourceStateManager::process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, const Resource* resource, int id, uint64_t full_id)
	{
		if (!resource) return;

		auto cpu_state = get_state(id);

		/*if (cpu_state.all.first_state != ResourceState::DIFFERENT && gpu_state.all.state != ResourceState::DIFFERENT)
		{
			if (cpu_state.all.first_state != gpu_state.all.state)
			{
				target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
					static_cast<D3D12_RESOURCE_STATES>(gpu_state.all.state),
					static_cast<D3D12_RESOURCE_STATES>(cpu_state.all.first_state),
					D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
			}
			gpu_state.all.state = gpu_state.all.state;
		}
		else */{

	//		unsigned int all_state = ResourceState::UNKNOWN;
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
			//	auto& gpu = gpu_state.all.state == ResourceState::DIFFERENT ? gpu_state.subres[i] : gpu_state.all;
			//	auto& cpu = cpu_state.all.first_state == ResourceState::DIFFERENT ? cpu_state.subres[i] : cpu_state.all;
				auto& gpu = gpu_state.subres[i];
				auto& cpu =  cpu_state.subres[i];

				if(cpu.command_list_id != full_id) continue;

				if (gpu.state != cpu.first_state )
				{
					assert(gpu.state != ResourceState::DIFFERENT);
					assert(gpu.state != ResourceState::UNKNOWN);

					assert(cpu.first_state != ResourceState::DIFFERENT);
					assert(cpu.first_state != ResourceState::UNKNOWN);

					target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
						static_cast<D3D12_RESOURCE_STATES>(gpu.state),
						static_cast<D3D12_RESOURCE_STATES>(cpu.first_state),
						i));

				/*	for (int j = 0; j < target.size() - 1; j++)
					{
						if (target.back().Type == target[j].Type)
							if (target.back().Transition.pResource == target[j].Transition.pResource)
								if (target.back().Transition.Subresource == target[j].Transition.Subresource)
									assert(false);
					}*/
			
				} 

				gpu_state.subres[i].state = cpu.state;

	//			if (all_state == ResourceState::UNKNOWN) all_state = cpu.state;
	//			if (all_state != cpu.state) all_state = ResourceState::DIFFERENT;

			}



			//gpu_state.all.state = all_state;

		}


	}

	void ResourceStateManager::transition(std::vector<D3D12_RESOURCE_BARRIER>& target, const Resource* resource, unsigned int state, unsigned int s, int id, uint64_t full_id) const
	{
		auto& cpu_state = get_state(id);


		auto transition_one = [&](int subres) {

			auto& subres_cpu = cpu_state.subres[subres];

			unsigned int prev_state = subres_cpu.state;
		

			if (subres_cpu.command_list_id != full_id)
			{
				subres_cpu.command_list_id = full_id;
				subres_cpu.first_state = state;
			}
			else
			{
				assert(state != ResourceState::DIFFERENT);
				assert(state != ResourceState::UNKNOWN);

				assert(subres_cpu.state != ResourceState::DIFFERENT);
				assert(subres_cpu.state != ResourceState::UNKNOWN);


				if (subres_cpu.state != state)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
						static_cast<D3D12_RESOURCE_STATES>(subres_cpu.state),
						static_cast<D3D12_RESOURCE_STATES>(state),
						subres));

				/*	for (int i = 0; i < target.size() - 1; i++)
					{
						if (target.back().Type == target[i].Type)
							if (target.back().Transition.pResource == target[i].Transition.pResource)
								if (target.back().Transition.Subresource == target[i].Transition.Subresource)
									assert(false);
					}
					*/
				}
				else if (state == ResourceState::UNORDERED_ACCESS)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get()));
				}

			}


			subres_cpu.state = state;
		};
		
		if (s == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			for (int i = 0; i < gpu_state.subres.size(); i++)
				transition_one(i);
		}
		else
		{
			transition_one(s);
		}
		
	}


    static std::atomic_size_t counter[3] = {0, 0, 0};
	std::atomic_size_t counter_id;
    DescriptorHeap::DescriptorHeap(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags)
    {
        max_count = num;
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = num;
        rtvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
        rtvHeapDesc.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
        rtvHeapDesc.NodeMask = 1;
        TEST(Device::get().get_native_device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
        descriptor_size = Device::get().get_native_device()->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type));
		assert(m_rtvHeap);
        /*   for (UINT n = 0; n < num; n++)
           {
               Handle h;
               h.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), n, descriptor_size);
               h.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), n, descriptor_size);
               handles.push_back(h);
           }*/

		resources.resize(max_count);
    }


    CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::get_handle(UINT i)
    {
        return  CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, descriptor_size);
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::get_gpu_handle(UINT i)
    {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), i, descriptor_size);
    }

	
    void Resource::init(const CD3DX12_RESOURCE_DESC& desc, ResourceAllocator& heap, ResourceState state, vec4 clear_value)
	{
	//	auto& timer = Profiler::get().start(L"Resource");
        auto t = CounterManager::get().start_count<Resource>();
        heap_type = heap.get_type();
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
		
		if (state == ResourceState::UNKNOWN)
		{
			if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				state = ResourceState::DEPTH_WRITE;
			else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				state = ResourceState::RENDER_TARGET;
			else 
					state = ResourceState::COMMON;
		}

		auto delete_me = heap.create_resource(desc, state, clear_value);
		m_Resource = delete_me->m_Resource;
     //   ClassLogger<Resource>::get() << "creating resource " << info.SizeInBytes << " heap: " << static_cast<UINT>(heap_type) << " total: " << (counter[static_cast<int>(heap_type) - 1] += info.SizeInBytes) << Log::endl;
       /* TEST(Device::get().get_native_device()->CreateCommittedResource(
                 &CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(type)),
                 D3D12_HEAP_FLAG_NONE,
                 &desc,
                 static_cast<D3D12_RESOURCE_STATES>(state),
                 (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
                 IID_PPV_ARGS(&m_Resource)));*/

	 id = counter_id.fetch_add(1);

        m_Resource->SetName(std::to_wstring(id).c_str());
	//	Log::get() << "resource creation: " << id << " at:\n" << get_stack_trace().to_string() << Log::endl;
        this->desc = CD3DX12_RESOURCE_DESC(m_Resource->GetDesc());

        if (heap_type != HeapType::READBACK && desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
            gpu_adress = m_Resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()), state);

		init_tilings();
    }
    Resource::Resource(const CD3DX12_RESOURCE_DESC& desc, ResourceAllocator&heap, ResourceState state, vec4 clear_value) :TiledResourceManager(m_Resource)
    {
        init(desc, heap, state, clear_value);
    }


    Resource::Resource(const ComPtr<ID3D12Resource>& resouce, ResourceState state, bool own) :TiledResourceManager(m_Resource)
    {
		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		resouce->GetHeapProperties(&HeapProperties, &HeapFlags);


		heap_type = (HeapType)HeapProperties.Type;
        m_Resource = resouce;
        desc = CD3DX12_RESOURCE_DESC(m_Resource->GetDesc());
        force_delete = !own;
		 id = counter_id.fetch_add(1);

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
            gpu_adress = m_Resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()), state);
       // states.resize(20);
	}
	Resource::Resource(const ComPtr<ID3D12Resource>& resource, CommonAllocator::Handle handle, ResourceState state):Resource(resource, state, true)
	{
		this->alloc_handle = handle;
		D3D12_HEAP_PROPERTIES HeapProperties;
		D3D12_HEAP_FLAGS  HeapFlags;
		resource->GetHeapProperties(&HeapProperties,&HeapFlags);

		if (HeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD)
		{
			resource->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}
	}

    Resource::~Resource()
    {
		if (buffer_data)
		{
			m_Resource->Unmap(0, nullptr);
		}
        if (!force_delete)
        {
            Device::get().unused(m_Resource);
         
	//std::stringstream stream;
	//	stream << std::hex << gpu_adress;
	//		std::string result(stream.str());
         //   ClassLogger<Resource>::get() << "deleting resource " << id<<" "<< result <<" "<< m_Resource.Get() << " " << info.SizeInBytes << " heap: " << static_cast<UINT>(heap_type) << " total: " << (counter[static_cast<int>(heap_type) - 1] -= info.SizeInBytes) << Log::endl;
        }
    }
	/*
    void Resource::change_state(CommandList::ptr& command_list, ResourceState from, ResourceState to, UINT subres)
    {

        if (subres == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            command_list->transition(this, to); else

            command_list->transition(this, from, to, subres);
    }

    void Resource::assume_state(int id, unsigned int state)
    {
    //    if (states.size() <= id)states.resize(id);

      //  states[id] = state;

		states = state;
    }
	void Resource::assume_state(unsigned int state)
	{
	//	states.clear();
	//	states.resize(20, state);

		states = state;
	}
    unsigned int Resource::get_state(int id)
    {
     //   if (states.size() <= id)states.resize(id);

     //   return states[id];
		return states;

    }
	*/


	HeapType ReservedAllocator::get_type()
	{
		return HeapType::DEFAULT;
	}
	HeapType DefaultAllocator::get_type()
	{
		return HeapType::DEFAULT;
	}
	HeapType ReadbackAllocator::get_type()
	{
		return HeapType::READBACK;
	}HeapType UploadAllocator::get_type()
	{
		return HeapType::UPLOAD;
	}	

	PlacedAllocator::PlacedAllocator() :
		heap_srv(HeapType::DEFAULT, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES),
		heap_rtv(HeapType::DEFAULT, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES),
		heap_uav(HeapType::DEFAULT, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES)
		
	{
	

		/*	heap_srv.init(1024 * 1024 * 128, HeapType::DEFAULT, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);
			heap_rtv.init(1024 * 1024 * 128, HeapType::DEFAULT, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);
			heap_upload_texture.init(1024 * 1024 * 128, HeapType::UPLOAD, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);
			heap_upload_buffer.init(1024 * 1024 * 128, HeapType::UPLOAD, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);*/
	}

	CommonAllocator::Handle PlacedAllocator::allocate_resource(CD3DX12_RESOURCE_DESC& desc, HeapType heap_type)
	{

		if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			if ((desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
		}

		D3D12_RESOURCE_ALLOCATION_INFO info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
		desc.Alignment = info.Alignment;

		if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			desc.Alignment = 0;
			info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
		}

		Allocator* heap = nullptr;


		if (heap_type == HeapType::DEFAULT)
		{
			if (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			{
				heap = &heap_rtv;
			}
			else
				heap = &heap_srv;
		}
		else if (heap_type == HeapType::UPLOAD)
		{
			heap = &current_frame->heap_upload_buffer;
		}



		return heap->Allocate(info.SizeInBytes, info.Alignment);
	}

	void ResourceHeap::init(size_t size)
	{
		heap = nullptr;

		D3D12_HEAP_DESC desc;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Flags = flags;
		desc.SizeInBytes = size;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Properties.Type = (D3D12_HEAP_TYPE)type;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//	assert(desc.SizeInBytes>0 && desc.SizeInBytes < 1024 * 1024 * 256);

		TEST(Device::get().get_native_device()->CreateHeap(&desc, IID_PPV_ARGS(&heap)));

		heap_size = size;
	}


	Resource::ptr PlacedAllocator::create_resource( CD3DX12_RESOURCE_DESC& desc, CommonAllocator::Handle handle, ResourceState state, vec4 clear_value)
	{

		if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			if ((desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}

			D3D12_RESOURCE_ALLOCATION_INFO info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
			if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			{
				desc.Alignment = 0;
				info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
			}
		}
		
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

		ResourceHeap* heap = dynamic_cast<ResourceHeap*>(handle.get_owner());

		if (state == ResourceState::UNKNOWN)
		{
			if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				state = ResourceState::DEPTH_WRITE;
			else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				state = ResourceState::RENDER_TARGET;
			else
				state = ResourceState::COMMON;
		}


		if (!is_shader_visible(desc.Format))
			desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		ComPtr<ID3D12Resource> m_Resource;
		TEST(Device::get().get_native_device()->CreatePlacedResource(
			heap->heap.Get(),
			handle.get_offset(),
			&desc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
			IID_PPV_ARGS(&m_Resource)));


		return std::make_shared<Resource>(m_Resource, handle, state);
	}



	Resource::ptr PlacedAllocator::create_resource( CD3DX12_RESOURCE_DESC& desc, HeapType heap_type, ResourceState state, vec4 clear_value)
	{
		CommonAllocator::Handle handle = allocate_resource(desc, heap_type);

		return create_resource(desc, handle, state, clear_value);
	}


	Resource::ptr ReservedAllocator::create_resource(const CD3DX12_RESOURCE_DESC& _desc, ResourceState state, vec4 clear_value)
	{
		CD3DX12_RESOURCE_DESC desc = _desc;

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


		if (state == ResourceState::UNKNOWN)
		{
			if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				state = ResourceState::DEPTH_WRITE;
			else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				state = ResourceState::RENDER_TARGET;
			else
				state = ResourceState::COMMON;
		}
		if (!is_shader_visible(desc.Format))
			desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		ComPtr<ID3D12Resource> m_Resource;
		TEST(Device::get().get_native_device()->CreateReservedResource(
			&desc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
			IID_PPV_ARGS(&m_Resource)));

		return std::make_shared<Resource>(m_Resource, state, true);
	}


	Resource::ptr DefaultAllocator::create_resource(const CD3DX12_RESOURCE_DESC& _desc, ResourceState state, vec4 clear_value)
		{
		CD3DX12_RESOURCE_DESC desc = _desc;

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


			if (state == ResourceState::UNKNOWN)
			{
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
					state = ResourceState::DEPTH_WRITE;
				else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
					state = ResourceState::RENDER_TARGET;
				else
					state = ResourceState::COMMON;
			}


			if (!is_shader_visible(desc.Format))
				desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			ComPtr<ID3D12Resource> m_Resource;
			TEST(Device::get().get_native_device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(HeapType::DEFAULT)),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&m_Resource)));


		
			return std::make_shared<Resource>(m_Resource, state, true);
		}

		Resource::ptr UploadAllocator::create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)
		{
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

			ComPtr<ID3D12Resource> m_Resource;
			TEST(Device::get().get_native_device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(HeapType::UPLOAD)),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&m_Resource)));

			return std::make_shared<Resource>(m_Resource,state, true);
		}
	
	
		Resource::ptr ReadbackAllocator::create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)
		{
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
			ComPtr<ID3D12Resource> m_Resource;
			TEST(Device::get().get_native_device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(HeapType::READBACK)),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&m_Resource)));

			return std::make_shared<Resource>(m_Resource,state, true);
		}


		void ResourceView::init_views(FrameResources& frame) {

			auto& desc = resource->get_desc();

			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
			{
				rtv = frame.rtv_cpu.place();
				place_rtv(rtv);
			}
			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
				dsv = frame.dsv_cpu.place();
				place_dsv(dsv);
			}
			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
				uav = frame.srv_uav_cbv_cpu.place();
				place_uav(uav);
			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER&&desc.Flags == D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE&&desc.Width<65536) {
				cb = frame.srv_uav_cbv_cpu.place();
				place_cb(cb);
			}

			if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
				srv = frame.srv_uav_cbv_cpu.place();
				place_srv(srv);
			}
		}

}
