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