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

	DX12::ResourceHandle ResourceHeapAllocator::alloc(size_t size, size_t alignment, D3D12_HEAP_FLAGS flags, HeapType type)
	{
		std::lock_guard<std::mutex> g(m);

		HeapIndex index;

		index.flags = flags;
		index.type = type;

		auto& creator = creators[index];

		if (!creator)
		{
			creator = std::make_shared<HeapAllocator>(index);
		}
		//	auto& creator = *it;

		return creator->alloc(size, alignment);
	}

	DX12::TileHeapPosition ResourceHeapAllocator::create_tile(D3D12_HEAP_FLAGS flags, HeapType type, UINT count /*= 1*/)
	{
		static const size_t TileSize = 64 * 1024_t;

		auto handle = alloc(count * TileSize, TileSize, flags, type);

		TileHeapPosition result;

		result.offset = UINT(handle.get_offset() / (64 * 1024));
		result.heap = handle.get_heap();

		result.handle = handle;
		result.count = count;
		return result;
	}

}