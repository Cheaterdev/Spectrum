#include "pch_dx.h"

#include "Memory.h"
import Device;
import Resource;

namespace DX12
{
	void ResourceHandle::Free()
	{
		if (!handle) return;
		owner->free(handle);
	}

	void ResourceHeap::init(size_t size)
	{
		tracked_info->heap = nullptr;

		
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Flags = flags;
		desc.SizeInBytes = size;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Properties.Type = to_native(type);
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		TEST(Device::get().get_native_device()->CreateHeap(&desc, IID_PPV_ARGS(&tracked_info->heap)));

		heap_size = size;

	}
	
	void ResourceHeap::init_cpu(ptr res)
	{

		if(desc.Flags&D3D12_HEAP_FLAG_DENY_BUFFERS)
		{
			return;
		}
		
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(heap_size);

		ResourceHandle handle(0, res);

	
		cpu_buffer = std::make_shared<Resource>(desc, handle);
	}


	std::span<std::byte> ResourceHeap::get_data()
	{
		return std::span(cpu_buffer->buffer_data, desc.SizeInBytes);
	}
}