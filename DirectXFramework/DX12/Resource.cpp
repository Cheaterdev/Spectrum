#include "pch.h"

namespace DX12
{
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
        auto info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
		delete_me = heap.create_resource(desc, state, clear_value);
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

		gpu_state = state;
		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()));


    }
    Resource::Resource(const CD3DX12_RESOURCE_DESC& desc, ResourceAllocator&heap, ResourceState state, vec4 clear_value)
    {
        init(desc, heap, state, clear_value);
    }


    Resource::Resource(const ComPtr<ID3D12Resource>& resouce, bool own)
    {
        m_Resource = resouce;
        desc = CD3DX12_RESOURCE_DESC(m_Resource->GetDesc());
        force_delete = !own;
		 id = counter_id.fetch_add(1);

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
            gpu_adress = m_Resource->GetGPUVirtualAddress();

		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()));
       // states.resize(20);
	}
	Resource::Resource(const ComPtr<ID3D12Resource>& resource, CommonAllocator::Handle handle, ResourceState state):Resource(resource, true)
	{
		this->alloc_handle = handle;
		this->gpu_state = state;
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
            auto info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
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

	CommonAllocator::Handle PlacedAllocator::allocate_resource(const CD3DX12_RESOURCE_DESC& desc, HeapType heap_type)
	{
		D3D12_RESOURCE_ALLOCATION_INFO  info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);

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


		TEST(Device::get().get_native_device()->CreateHeap(&desc, IID_PPV_ARGS(&heap)));

		heap_size = size;
	}


	Resource::ptr PlacedAllocator::create_resource(const CD3DX12_RESOURCE_DESC& desc, Allocator::Handle handle, ResourceState state, vec4 clear_value)
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


		ComPtr<ID3D12Resource> m_Resource;
		TEST(Device::get().get_native_device()->CreatePlacedResource(
			heap->heap.Get(),
			handle.aligned_offset,
			&desc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
			IID_PPV_ARGS(&m_Resource)));


		return std::make_shared<Resource>(m_Resource, handle, state);
	}



	Resource::ptr PlacedAllocator::create_resource(const CD3DX12_RESOURCE_DESC& desc, HeapType heap_type, ResourceState state, vec4 clear_value)
	{
		CommonAllocator::Handle handle = allocate_resource(desc, heap_type);

		return create_resource(desc, handle, state, clear_value);
	}

	Resource::ptr DefaultAllocator::create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)
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


			if (state == ResourceState::UNKNOWN)
			{
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
					state = ResourceState::DEPTH_WRITE;
				else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
					state = ResourceState::RENDER_TARGET;
				else
					state = ResourceState::COMMON;
			}



			ComPtr<ID3D12Resource> m_Resource;
			TEST(Device::get().get_native_device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(HeapType::DEFAULT)),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&m_Resource)));

			return std::make_shared<Resource>(m_Resource, true);
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

			return std::make_shared<Resource>(m_Resource, true);
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

			return std::make_shared<Resource>(m_Resource, true);
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
			srv = frame.srv_uav_cbv_cpu.place();
				place_srv(srv);
		}

}
