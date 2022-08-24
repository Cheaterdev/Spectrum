module;
#include "pch_dx.h"

#include "Memory.h"
module Graphics:Memory;
import :Device;
import :Resource;

import StateContext;
import Allocators;

import Threading;
import Singleton;
import Utils;
import Math;
import Trackable;
import HAL.Types;
import HAL.Heap;

using namespace HAL;

namespace Graphics
{
	void ResourceHandle::Free()
	{
		if (!handle) return;
		owner->free(handle);
	}

	ResourceHeap::ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags)
	{
		desc.Size = size;
		desc.Type = type;
		if(flags== D3D12_HEAP_FLAG_NONE)desc.Flags = HeapFlags::NONE;
		if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS) desc.Flags = HeapFlags::BUFFERS_ONLY;
		if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES) desc.Flags = HeapFlags::TEXTURES_ONLY;
		if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES) desc.Flags = HeapFlags::RTDS_ONLY;

		tracked_info->heap = std::make_shared<HAL::Heap>(*Device::get().get_hal_device(), desc);
	}

	std::span<std::byte> ResourceHeap::get_data()
	{
		return tracked_info->heap->cpu_data();
	}

	void ResourceHeap::init_cpu(ptr res)
	{
			if (!tracked_info->heap->cpu_data().empty())
			{

				CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(this->desc.Size);
				ResourceHandle handle(0, res);
				cpu_buffer = std::make_shared<Resource>(desc, handle);
			}
	}


}