export module HAL:HeapAllocators;
import Core;

import :Types;
import :Utils;
import :Heap;
import :API.Device;
using namespace HAL;


export namespace HAL
{
	struct HeapIndex
	{
		MemoryType memory;
		HeapType type;

		GEN_DEF_COMP(HeapIndex);
	};
	
	struct ResourceContext
	{
		using HeapPageType = HAL::Heap;
		using HeapMemoryOptions = HeapIndex;

		static const size_t PageAlignment = 1_mb;

		static const size_t GarbageCount = 10;
	};


	using ResourceHandle = Allocators::HeapHandle<HAL::Heap>;


	struct TileHeapPosition
	{

		ResourceHandle handle;
		Heap::ptr heap;
		UINT offset;
		UINT count = 1;
	};

	struct ResourceTile
	{
		ivec3 pos = ivec3(0, 0, 0);
		UINT subresource;

		ivec3 size = ivec3(1, 1, 1);
		TileHeapPosition heap_position;
	};


	class HeapFactory:public Allocators::HeapFactory<ResourceContext, GlobalAllocationPolicy>
	{
		Device& device;
		virtual ptr_type make_heap(HeapIndex index, size_t size) override
		{
			HeapDesc desc = { size , index.type, index.memory , HeapFlags::NONE};
			return std::make_shared<HAL::Heap>(device,desc);
		}

	public:
		HeapFactory(Device& device) :device(device)
				{
					
				}

	};
	


	struct UploadInfo :public HAL::ResourceAddress
	{
		uint size;
	};


	template<class AllocationPolicy>
	class GPUMemoryAllocator:public Allocators::HeapPageManager<ResourceContext, AllocationPolicy>
	{
		
	public:
		using Allocators::HeapPageManager<ResourceContext, AllocationPolicy>::alloc;

		static constexpr uint DEFAULT_ALIGN = 256;

		GPUMemoryAllocator(Device&device):Allocators::HeapPageManager<ResourceContext, AllocationPolicy>(device.get_heap_factory())
		{
			
		}
	

	};


}

