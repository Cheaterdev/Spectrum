#include "pch.h"
namespace DX12 {

	DescriptorHeapManager::DescriptorHeapManager()
	{
		heap_cb_sr_ua_static.reset(new DescriptorHeap(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
		heap_rt.reset(new DescriptorHeap(65536, DescriptorHeapType::RTV, DescriptorHeapFlags::NONE));
		heap_ds.reset(new DescriptorHeap(65536, DescriptorHeapType::DSV, DescriptorHeapFlags::NONE));


		gpu_srv.reset(new DescriptorHeapPaged(65536*8, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE));
		gpu_smp.reset(new DescriptorHeapPaged(2048, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE));
	
		cpu_srv.reset(new DescriptorHeapPaged(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
		cpu_rtv.reset(new DescriptorHeapPaged(65536, DescriptorHeapType::RTV, DescriptorHeapFlags::NONE));
		cpu_dsv.reset(new DescriptorHeapPaged(65536, DescriptorHeapType::DSV, DescriptorHeapFlags::NONE));
		cpu_smp.reset(new DescriptorHeapPaged(65536, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE));
	}


	void Handle::place(const Handle& r, D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		if (cpu != r.cpu)
			Device::get().get_native_device()->CopyDescriptorsSimple(1, cpu, r.cpu, type);

		*resource_info = *r.resource_info;
	}

	void HandleTable::place(const HandleTable& r, D3D12_DESCRIPTOR_HEAP_TYPE t)
	{
		Device::get().get_native_device()->CopyDescriptorsSimple(r.get_count(), get_base().cpu, r.get_base().cpu, t);
	}
	/*
	void HandleTableLight::place(const HandleTableLight& r, D3D12_DESCRIPTOR_HEAP_TYPE t)
	{
		Device::get().get_native_device()->CopyDescriptorsSimple(r.get_count(), cpu, r.cpu, t);
	}
	void HandleTableLight::place(const HandleTable& r, D3D12_DESCRIPTOR_HEAP_TYPE t)
	{
		Device::get().get_native_device()->CopyDescriptorsSimple(r.get_count(), cpu, r.get_base().cpu, t);
	}*/

	DynamicDescriptorManager::DynamicDescriptorManager(Creator& creator) :creator(creator)
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


	void DynamicDescriptorManager::set(UINT i, UINT offset, std::vector<Handle>& h)
	{

		if (h.size() == 0) return;
		if (!root_tables[i].fixed)
			root_tables[i].handles.resize(h.size());

		for (size_t j = 0; j < h.size(); j++)
			root_tables[i].handles[offset + j] = h[j];
		root_tables[i].changed = true;
	}
	void DynamicDescriptorManager::set(UINT i, UINT offset, Handle h)
	{
		if ((!root_tables[i].fixed) && root_tables[i].handles.size() <= offset)
			root_tables[i].handles.resize(offset + 1);

		root_tables[i].handles[offset] = h;
		root_tables[i].changed = true;
	}
	void DynamicDescriptorManager::set(UINT i, UINT offset, HandleTable h)
	{
		if (h.get_count() == 0) return;
		if ((!root_tables[i].fixed) && root_tables[i].handles.size() <= offset + h.get_count())
			root_tables[i].handles.resize(offset + h.get_count() + 1);

		for (UINT j = 0; j < std::min(static_cast<UINT>(root_tables[i].handles.size()), offset + h.get_count()) - offset; j++)
			root_tables[i].handles[offset + j] = h[j];
		root_tables[i].changed = true;
	}

	UINT DynamicDescriptorManager::calculate_needed_size()
	{
		UINT size = 0;
		for (auto& e : root_tables)
		{
			if (!e.second.changed) continue;
			size += static_cast<UINT>(e.second.handles.size());
		}


		return size;
	}

	void DynamicDescriptorManager::unbind_all()
	{
		for (auto& e : root_tables)
			e.second.changed = true;


	}
	void DynamicDescriptorManager::bind(CommandList* list)
	{
/*
		auto heap = creator.get_heap();
		if (heap)
			list->set_heap(DescriptorHeapType::CBV_SRV_UAV, heap);*/

	}
	/*	Handle DynamicDescriptorManager::place(const Handle &h)
		{
			return creator.place(h);
		}*/

	void DynamicDescriptorManager::bind(CommandList* list, std::function<void(UINT, Handle)> f)
	{
		int needed_size = calculate_needed_size();

		/*
				if (creator.prepare(needed_size))
				{
					unbind_all();
				}

				list->set_heap(DescriptorHeapType::CBV_SRV_UAV, creator.get_heap());
				*/
		for (auto& e : root_tables)
		{
			if (!e.second.changed) continue;

			auto table = creator.place(static_cast<UINT>(e.second.handles.size()));

			for (unsigned int j = 0; j < e.second.handles.size(); j++)
				if (e.second.handles[j].is_valid())
					table[j].place(e.second.handles[j]);

			e.second.changed = false;
			f(e.first, table[0]);

		}



	}

	void DynamicDescriptorManager::bind_table(UINT i, std::vector<Handle>& h)
	{
		//	root_tables[i].changed = true;
		//	swap(root_tables[i].handles, h);

		set(i, 0, h);
	}

	void DynamicDescriptorManager::parse(RootSignature::ptr root)
	{
		auto& desc = root->get_desc();
		return;

		root_tables.clear();
		for (auto& e : desc.tables)
		{
			if (e.second.count == 0) continue;
			root_tables[e.first].fixed = e.second.fixed;
			root_tables[e.first].count = e.second.count;
			root_tables[e.first].handles.resize(e.second.count);
			if (e.second.type == DescriptorRange::SRV)
				for (auto&& l : root_tables[e.first].handles) l = null_srv[0];

			if (e.second.type == DescriptorRange::UAV)
				for (auto&& l : root_tables[e.first].handles) l = null_uav[0];

			/*
			memset(root_tables[e.first].handles.data(),&null_srv[0],sizeof(Handle)*e.second.count);

			if (e.second.type == DescriptorRange::UAV)
				memset(root_tables[e.first].handles.data(), &null_uav[0], sizeof(Handle)*e.second.count);*/
			root_tables[e.first].changed = true;
		}
	}

	void Handle::clear(CommandList& list, float4 color) const
	{

		list.clear_rtv(*this, color);
	}


	Handle DescriptorPage::place()
	{
	//	std::lock_guard<std::mutex> g(m);
		Handle h =  heap->handle(heap_offset + (offset++));
		h.resource_info->resource_ptr = nullptr;
		return h;
	}



	HandleTableLight DescriptorPage::place(UINT count)
	{
		//std::lock_guard<std::mutex> g(m);
		assert(offset+count<=this->count);
		auto table = heap->get_light_table_view(heap_offset + offset, count);

		for (int i = 0; i < table.get_count(); i++)
		{
			table[i].resource_info->resource_ptr = nullptr;
		}
		offset += count;
		return table;
	}



	void DescriptorPage::free()
	{
	//	std::lock_guard<std::mutex> g(m);
		offset = 0;
		heap->free_page(this);
	}

}