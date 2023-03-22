module HAL:FrameManager;

import Core;
import :Types;

import HAL;

namespace HAL {

	FrameResources::ptr FrameResourceManager::begin_frame()
	{

		auto result = std::make_shared<FrameResources>(HAL::Device::get());


		result->frame_number = frame_number++;

		return result;
	}

	std::shared_ptr<CommandList> FrameResources::start_list(std::string name, CommandListType type)
	{
		auto list = (HAL::Device::get().get_queue(type)->get_free_list());	
		list->frame_resources = get_ptr();
		list->begin(name);

		return list;
	}



	UploadInfo GPUEntityStorageInterface::aquire_data(UINT64 uploadBufferSize, HeapType heap_type, unsigned int alignment)
	{
		const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));

		HeapIndex index = { HAL::MemoryType::COMMITED , heap_type };
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


		void FrameResources::free_ca(std::shared_ptr<CommandAllocator> e)
			{
				command_allocators[e->get_type()].put(e);
			}

}