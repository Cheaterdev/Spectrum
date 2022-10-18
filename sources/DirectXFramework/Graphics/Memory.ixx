module;


export module Graphics:Memory;

import Data;
import StateContext;
import Allocators;

import Threading;
import Singleton;
import Utils;
import Math; 
import :Device;
import Trackable;
import HAL;
import Utils;
using namespace HAL;

import d3d12;

import :Definitions;
export
{


	namespace Graphics
	{

		using ResourceHeap = HAL::Heap;

		using ResourceHeapPage = HAL::ResourceHeapPage;
using ResourceHandle = HAL::ResourceHandle;

		template<class LockPolicy>
		using ResourceHeapAllocator = ResourceHeapAllocator<LockPolicy>;


		//class ResourceHeap :public SharedObject<ResourceHeap>, public ObjectState<TrackedObjectState>, public TrackedObject, public HAL::API::Heap
		//{
		//	friend class HAL::API::Heap;
		//	HAL::HeapDesc desc;
		//public:
		//	std::shared_ptr<Resource> cpu_buffer;

		//	using ptr = std::shared_ptr<ResourceHeap>;

		//	ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags);


		//	HeapType get_type() { return desc.Type; }
		//	virtual ~ResourceHeap() = default;

		//	std::span<std::byte> get_data();
		//	void init_cpu(ptr);
		//};

		//	template<class AllocatorType>
		
		/*
		template<class AllocatorType>
		struct ResourceHeapAllocator : public ResourceHeap, public AllocatorType
		{
			using ResourceHeap::ResourceHeap;

			void allocate_for_allocator(bool reset = false)
			{
				auto max_usage = AllocatorType::get_max_usage();
				if (max_usage > heap_size)
				{
					ResourceHeap::init(max_usage);
				}
			}
		};*/
	}


}