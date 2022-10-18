module;


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
import HAL;

using namespace HAL;

namespace HAL
{
	void ResourceHandle::Free()
	{
		if (!handle) return;
		owner->free(handle);
	}

	//ResourceHeap::ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags)
	//{
	//	desc.Size = size;
	//	desc.Type = type;
	//	if (flags == D3D12_HEAP_FLAG_NONE)desc.Flags = HeapFlags::NONE;
	//	if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS) desc.Flags = HeapFlags::BUFFERS_ONLY;
	//	if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES) desc.Flags = HeapFlags::TEXTURES_ONLY;
	//	if (flags == D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES) desc.Flags = HeapFlags::RTDS_ONLY;

	//	HAL::Heap::init(Device::get(), desc);
	//}

	//void ResourceHeap::init_cpu(ptr res)
	//{
	//	if (!get_data().empty())
	//	{
	//		ResourceHandle handle(0, res);
	//		cpu_buffer = std::make_shared<Resource>(ResourceDesc::Buffer(this->desc.Size), handle);
	//	}
	//}


}