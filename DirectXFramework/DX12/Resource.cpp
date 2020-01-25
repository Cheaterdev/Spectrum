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
         Device::get().get_native_device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        descriptor_size = Device::get().get_native_device()->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type));
		assert(m_rtvHeap);
        /*   for (UINT n = 0; n < num; n++)
           {
               Handle h;
               h.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), n, descriptor_size);
               h.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), n, descriptor_size);
               handles.push_back(h);
           }*/
    }


    CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::get_handle(UINT i)
    {
        return  CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, descriptor_size);
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::get_gpu_handle(UINT i)
    {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), i, descriptor_size);
    }

	
    void Resource::init(const CD3DX12_RESOURCE_DESC& desc, HeapType type, ResourceState state, vec4 clear_value)
    {
        auto t = CounterManager::get().start_count<Resource>();
        heap_type = type;
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
		
        auto info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);

		if (info.SizeInBytes > 257698037)
			ClassLogger<Resource>::get() << "Ololo" << Log::endl;
        ClassLogger<Resource>::get() << "creating resource " << info.SizeInBytes << " heap: " << static_cast<UINT>(heap_type) << " total: " << (counter[static_cast<int>(heap_type) - 1] += info.SizeInBytes) << Log::endl;
        TEST(Device::get().get_native_device()->CreateCommittedResource(
                 &CD3DX12_HEAP_PROPERTIES(static_cast<D3D12_HEAP_TYPE>(type)),
                 D3D12_HEAP_FLAG_NONE,
                 &desc,
                 static_cast<D3D12_RESOURCE_STATES>(state),
                 (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
                 IID_PPV_ARGS(&m_Resource)));
	//	auto stack = get_stack_trace();

	
	 id = counter_id.fetch_add(1);

        m_Resource->SetName(std::to_wstring(id).c_str());
	//	Log::get() << "resource creation: " << id << " at:\n" << get_stack_trace().to_string() << Log::endl;
        this->desc = CD3DX12_RESOURCE_DESC(m_Resource->GetDesc());

        if (type != HeapType::READBACK && desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
            gpu_adress = m_Resource->GetGPUVirtualAddress();

		gpu_state = state;
		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()));

    }
    Resource::Resource(const CD3DX12_RESOURCE_DESC& desc, HeapType type, ResourceState state, vec4 clear_value)
    {
        init(desc, type, state, clear_value);
       // states.resize(20, state);
		gpu_state = state;
		init_subres(this->desc.Subresources(Device::get().get_native_device().Get()));
	
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

    Resource::~Resource()
    {
        if (!force_delete)
        {
            Device::get().unused(m_Resource);
            auto info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);
			std::stringstream stream;
			stream << std::hex << gpu_adress;
			std::string result(stream.str());
            ClassLogger<Resource>::get() << "deleting resource " << id<<" "<< result <<" "<< m_Resource.Get() << " " << info.SizeInBytes << " heap: " << static_cast<UINT>(heap_type) << " total: " << (counter[static_cast<int>(heap_type) - 1] -= info.SizeInBytes) << Log::endl;
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
    DescriptorHeapManager::DescriptorHeapManager()
    {
        heap_cb_sr_ua.reset(new DescriptorHeap(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE));
        heap_cb_sr_ua_static.reset(new DescriptorHeap(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
        heap_samplers.reset(new DescriptorHeap(64, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE));
        heap_rt.reset(new DescriptorHeap(65536, DescriptorHeapType::RTV, DescriptorHeapFlags::NONE));
        heap_ds.reset(new DescriptorHeap(65536, DescriptorHeapType::DSV, DescriptorHeapFlags::NONE));
        samplers_default_table = heap_samplers->create_table(5);
        D3D12_SAMPLER_DESC wrapSamplerDesc = {};
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        wrapSamplerDesc.MinLOD = 0;
        wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        wrapSamplerDesc.MipLODBias = 0.0f;
        wrapSamplerDesc.MaxAnisotropy = 16;
        wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
        Device::get().create_sampler(wrapSamplerDesc, samplers_default_table[0].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        Device::get().create_sampler(wrapSamplerDesc, samplers_default_table[1].cpu);
        wrapSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        Device::get().create_sampler(wrapSamplerDesc, samplers_default_table[2].cpu);
    }


    void Handle::place(const Handle& r, D3D12_DESCRIPTOR_HEAP_TYPE type ) const
    {
		if(cpu!=r.cpu)
        Device::get().get_native_device()->CopyDescriptorsSimple(1, cpu, r.cpu, type);
    }

	void HandleTable::place(const HandleTable& r, D3D12_DESCRIPTOR_HEAP_TYPE t)
	{
		Device::get().get_native_device()->CopyDescriptorsSimple(r.get_count(), get_base().cpu, r.get_base().cpu, t);
	}


    static std::queue<DescriptorHeap::ptr> free_tables;
    static std::mutex tables_mutex;


	DescriptorHeap::ptr DynamicDescriptorManager::get_free_table()
    {
        std::lock_guard<std::mutex> g(tables_mutex);

        if (!free_tables.empty())
        {
			DescriptorHeap::ptr res = free_tables.front();
            free_tables.pop();
            return res;
        }

		return std::make_shared<DescriptorHeap>(1024, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE); //::get().get_csu()->create_table(256);
    }
    void DynamicDescriptorManager::free_table(DescriptorHeap::ptr t)
    {
        std::lock_guard<std::mutex> g(tables_mutex);
        free_tables.push(t);
    }

	DynamicDescriptorManager::DynamicDescriptorManager()
	{
		//	original = DescriptorHeapManager::get().get_csu()->create_table(256);

		null_srv = DescriptorHeapManager::get().get_csu_static()->create_table(1);
		null_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		srvDesc.Format = to_srv(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;



		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.PlaneSlice = 0;
		desc.Texture2D.MipSlice = 0;


		Device::get().get_native_device()->CreateShaderResourceView(nullptr, &srvDesc, null_srv[0].cpu);
		Device::get().get_native_device()->CreateUnorderedAccessView(nullptr, nullptr, &desc, null_uav[0].cpu);
//	heap_cb_sr_ua.reset(new DescriptorHeap(1024, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE));
	}

	DX12::DescriptorHeap::ptr DynamicDescriptorManager::create_heap()
	{
		pages.emplace_back(get_free_table());
		offset = 0;
		return pages.back();
	}

	void DynamicDescriptorManager::reset()
	{
		offset = 0;

		for (auto& p : pages)
			free_table(p);

		pages.clear();
	}
	void DynamicDescriptorManager::set(UINT i, UINT offset, std::vector<Handle>& h)
	{

		if (h.size() == 0) return;
		if (!root_tables[i].fixed)
			root_tables[i].handles.resize(h.size());

		for(size_t j=0;j<h.size();j++)
		root_tables[i].handles[offset+j] = h[j];
		root_tables[i].changed = true;
	}
	void DynamicDescriptorManager::set(UINT i, UINT offset, Handle h)
	{
		if ((!root_tables[i].fixed)&&root_tables[i].handles.size()<=offset)
			root_tables[i].handles.resize(offset+1);

		root_tables[i].handles[offset] = h;
		root_tables[i].changed = true;
	}
	void DynamicDescriptorManager::set(UINT i, UINT offset, HandleTable h)
	{
		if (h.get_count() == 0) return;
		if ((!root_tables[i].fixed)&&root_tables[i].handles.size() <= offset+h.get_count())
			root_tables[i].handles.resize(offset + h.get_count() + 1);

		for(UINT j=0;j<std::min(static_cast<UINT>(root_tables[i].handles.size()),offset+h.get_count())- offset;j++)
		root_tables[i].handles[offset+j] = h[j];
		root_tables[i].changed = true;
	}
	bool DynamicDescriptorManager::has_free_size(UINT count)
	{
		return (!pages.empty() && (offset + count < pages.back()->size()));
	}

	UINT DynamicDescriptorManager::calculate_needed_size()
	{
		UINT size = 0;
		for (auto &e : root_tables)
		{
			if (!e.second.changed) continue;
			size += static_cast<UINT>(e.second.handles.size());
		}

	
		return size;
	}

	void DynamicDescriptorManager::unbind_all()
	{
		for (auto &e : root_tables)
			e.second.changed = true;	
		
	
	}
	void DynamicDescriptorManager::bind(CommandList * list)
	{
		if (pages.size())
		{
			auto &heap = pages.back();
			list->set_heap(DescriptorHeapType::CBV_SRV_UAV, heap);
		}
	}
	Handle DynamicDescriptorManager::place(const Handle &h)
	{

		if (!has_free_size(1))
		{

			create_heap();
			unbind_all();
		}
		auto &heap = pages.back();
		auto table = heap->get_table_view(offset, 1);

		offset += 1;
		table[0].place(h);

		return table[0];
	}

	void DynamicDescriptorManager::bind(CommandList * list, std::function<void(UINT, Handle)> f)
	{
		int needed_size = calculate_needed_size();

		if (!has_free_size(needed_size))
		{

			create_heap();
			unbind_all();
			///////////////////////

		}


		auto &heap = pages.back();
		if(needed_size)
		list->set_heap(DescriptorHeapType::CBV_SRV_UAV, heap);

		for (auto &e : root_tables)
		{
			if (!e.second.changed) continue;
		
			auto table = heap->get_table_view(offset, static_cast<UINT>(e.second.handles.size()));

			for (unsigned int j = 0; j < e.second.handles.size(); j++)
				if (e.second.handles[j].is_valid())
					table[j].place(e.second.handles[j]);

			e.second.changed = false;
			f(e.first, table[0]);
			offset += static_cast<UINT>(e.second.handles.size());

		//	if (!e.second.fixed)
			//	e.second.handles.resize(e.second.count);
		}

		

	}

	void DynamicDescriptorManager::bind_table(UINT i, std::vector<Handle>& h)
	{
	//	root_tables[i].changed = true;
	//	swap(root_tables[i].handles, h);

		set(i,0, h);
	}

	void DynamicDescriptorManager::parse(RootSignature::ptr root)
	{
		auto& desc = root->get_desc();


		root_tables.clear();
		for (auto& e : desc.tables)
		{
			root_tables[e.first].fixed = e.second.fixed;
			root_tables[e.first].count = e.second.count;
			root_tables[e.first].handles.resize(e.second.count);
			if (e.second.type == DescriptorRange::SRV)
				for (auto&&l : root_tables[e.first].handles) l = null_srv[0];

			if (e.second.type == DescriptorRange::UAV)
				for (auto&&l : root_tables[e.first].handles) l = null_uav[0];

			/*
			memset(root_tables[e.first].handles.data(),&null_srv[0],sizeof(Handle)*e.second.count);

			if (e.second.type == DescriptorRange::UAV)
				memset(root_tables[e.first].handles.data(), &null_uav[0], sizeof(Handle)*e.second.count);*/
			root_tables[e.first].changed = true;
		}
	}

	void Handle::clear(CommandList& list, float4 color) const
    {
		list.flush_transitions();
        list.get_native_list()->ClearRenderTargetView(cpu, &color.x, 0, nullptr);
    }
}
