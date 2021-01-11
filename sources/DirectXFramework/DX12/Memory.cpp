#include "pch.h"
#include "Memory.h"
#include "Device12.h"

namespace DX12
{
	void ResourceHandle::Free()
	{
		if (!handle) return;
		owner->free(handle);
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

}