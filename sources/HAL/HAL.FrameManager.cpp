module HAL:FrameManager;

import Core;
import :Types;

import HAL;

namespace HAL {

	// GPUEntityStorageInterface

	UploadInfo GPUEntityStorageInterface::place_data(UINT64 uploadBufferSize, unsigned int alignment)
	{
		return aquire_data(uploadBufferSize, HeapType::UPLOAD, alignment);
	}

	UploadInfo GPUEntityStorageInterface::read_data(UINT64 uploadBufferSize, unsigned int alignment, unsigned int offset)
	{
		return aquire_data(uploadBufferSize, HeapType::READBACK, alignment, offset);
	}

	void GPUEntityStorageInterface::write(UploadInfo& info, size_t offset, void* data, size_t size)
	{
		if (size > 0) std::memcpy(info.get_cpu_data() + offset, data, size);
	}

	UploadInfo GPUEntityStorageInterface::aquire_data(UINT64 uploadBufferSize, HeapType heap_type, unsigned int alignment, unsigned int offset)
	{
		const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));

		HeapIndex index = { HAL::MemoryType::COMMITED , heap_type, offset };
		auto handle = alloc_memory(AlignedSize, alignment, index);

		return  { handle.get_heap()->as_buffer()->get_resource_address().offset(handle.get_offset()), uint(uploadBufferSize) };
	}

	TileHeapPosition GPUEntityStorageInterface::create_tile(HeapType type, UINT count)
	{
		static const size_t TileSize = 64 * 1024;
		HeapIndex index = { HAL::MemoryType::COMMITED , type };

		auto handle = alloc_memory(count * TileSize, TileSize, index);

		TileHeapPosition result;

		result.offset = static_cast<UINT>(handle.get_offset() / (64 * 1024));
		result.heap = handle.get_heap();

		result.handle = handle;
		result.count = count;
		return result;
	}


	// GPUEntityStorageProxy

	void GPUEntityStorageProxy::set_proxy(std::shared_ptr<GPUEntityStorageInterface> proxy)
	{
		this->proxy = proxy;
	}

	ResourceHandle GPUEntityStorageProxy::alloc_memory(size_t size, size_t alignment, HeapIndex options)
	{
		ASSERT(proxy);
		return proxy->alloc_memory(size, alignment, options);
	}

	QueryHandle GPUEntityStorageProxy::alloc_query(uint size, QueryType options)
	{
		ASSERT(proxy);
		return proxy->alloc_query(size, options);
	}

	Handle GPUEntityStorageProxy::alloc_base_descriptor(uint size, DescriptorHeapIndex options)
	{
		ASSERT(proxy);
		return proxy->alloc_base_descriptor(size, options);
	}

	void GPUEntityStorageProxy::resolve_timers(std::function<void(const QueryType&, uint64, uint64, QueryHeap::ptr) > f)
	{
		ASSERT(proxy);
		proxy->resolve_timers(f);
	}


	// StaticCompiledGPUData

	StaticCompiledGPUData::StaticCompiledGPUData(Device& device)
		: device(device), GPUEntityStorage<GlobalAllocationPolicy>(device)
	{
	}


	// FrameResources

	FrameResources::FrameResources(Device& device) : GPUEntityStorage<LocalAllocationPolicy>(device)
	{
		gpu_resources.create_func = [&device]() {
			return std::make_shared<GPUEntityStorage<LocalAllocationPolicy>>(device);
		};

		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			command_allocators[type].create_func = [type]() {
				return Device::get().get_ca(type);
			};
		}
	}

	FrameResources::~FrameResources()
	{
		reset();

		for (auto& e : gpu_resources.table)
		{
			e->reset();
		}
		for (auto type : magic_enum::enum_values<CommandListType>())
			for (auto& e : command_allocators[type].table)
			{
				Device::get().free_ca(e);
			}
	}

	std::uint64_t FrameResources::get_frame()
	{
		return frame_number;
	}

	void FrameResources::free_storage(std::shared_ptr<GPUEntityStorageInterface> e)
	{
		gpu_resources.put(std::static_pointer_cast<GPUEntityStorage<LocalAllocationPolicy>>(e));
	}

	std::shared_ptr<GPUEntityStorageInterface> FrameResources::get_storage()
	{
		return gpu_resources.get();
	}

	void FrameResources::free_ca(std::shared_ptr<CommandAllocator> e)
	{
		command_allocators[e->get_type()].put(e);
	}

	std::shared_ptr<CommandAllocator> FrameResources::get_ca(CommandListType type)
	{
		return command_allocators[type].get();
	}

	std::shared_ptr<CommandList> FrameResources::start_list(std::wstring_view name, CommandListType type)
	{
		auto list = (HAL::Device::get().get_queue(type)->get_free_list());
		list->frame_resources = get_ptr();
		list->begin(name);

		return list;
	}


	// FrameResourceManager

	FrameResourceManager::FrameResourceManager(Device& device) : device(device)
	{
	}

	FrameResources::ptr FrameResourceManager::begin_frame()
	{
		auto result = std::make_shared<FrameResources>(HAL::Device::get());

		result->frame_number = frame_number++;

		return result;
	}

}
