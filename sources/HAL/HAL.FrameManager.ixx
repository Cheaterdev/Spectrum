export module HAL:FrameManager;

import Core;
import :Types;
import :HeapAllocators;
import :DescriptorHeap;
import :Private.CommandListCompiler;
import :Device;
import :QueryHeap;
export
namespace HAL {


	class GPUEntityStorageInterface
	{
	public:
		virtual ResourceHandle alloc_memory(size_t size, size_t alignment, HeapIndex options) = 0;
		virtual QueryHandle alloc_query(uint size, QueryType options) = 0;
		virtual Handle  alloc_descriptor(uint size, DescriptorHeapIndex options) = 0;
		virtual ~GPUEntityStorageInterface() = default;
	};

		template<class AllocationPolicy>
		struct GPUEntityStorage:
		public GPUEntityStorageInterface,
			public CPUGPUAllocator<AllocationPolicy>,
			public DescriptorHeapPageManager<AllocationPolicy>,
			public QueryHeapPageManager<AllocationPolicy>
		{
			static constexpr bool resetable = std::is_same_v<typename AllocationPolicy::AllocatorType,LinearAllocator>;

			GPUEntityStorage(Device& device):
				CPUGPUAllocator<AllocationPolicy>(device),
				QueryHeapPageManager<AllocationPolicy>(device),
				DescriptorHeapPageManager<AllocationPolicy>(device)
			{
				
			}
			CPUGPUAllocator<AllocationPolicy>::HandleType alloc_memory(size_t size, size_t alignment, CPUGPUAllocator<AllocationPolicy>::HeapMemoryOptions options) override
			{
				return CPUGPUAllocator<AllocationPolicy>::alloc(size, alignment, options);
			}

			QueryHeapPageManager<AllocationPolicy>::HandleType  alloc_query(uint size,  QueryHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
			{
				return QueryHeapPageManager<AllocationPolicy>::alloc(size,1,options);
			}


			Handle  alloc_descriptor(uint size, DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
			{
				auto h = DescriptorHeapPageManager<AllocationPolicy>::alloc(size, 1, options);
				
				return Handle{std::make_shared<DescriptorHeapStorage>(h),0};
			}

			void reset() requires (resetable)
			{

				DescriptorHeapPageManager<AllocationPolicy>::for_each([](const DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions& type, uint64 from , uint64 to, HAL::DescriptorHeap::ptr heap)
				{
		
					for(uint i=static_cast<uint>(from);i<static_cast<uint>(to);i++)
					{
						heap->get_resource_info(i) = HAL::Views::Null();
					}

				});

				CPUGPUAllocator<AllocationPolicy>::reset();
				QueryHeapPageManager<AllocationPolicy>::reset();
				DescriptorHeapPageManager<AllocationPolicy>::reset();
			}



			~GPUEntityStorage()
			{
				if constexpr (resetable) reset();
			}
		};

		class StaticCompiledGPUData : public GPUEntityStorage<GlobalAllocationPolicy>
		{
			Device& device;
		public:
			using GPUEntityStorage<GlobalAllocationPolicy>::place_raw;

			StaticCompiledGPUData(Device& device) :device(device), GPUEntityStorage<GlobalAllocationPolicy>(device) {}
		};

		class FrameResources :public SharedObject<FrameResources>, public GPUEntityStorage<FrameAllocationPolicy>
		{
			friend class FrameResourceManager;

			std::uint64_t frame_number = 0;
			std::mutex m;
		public:
			using ptr = std::shared_ptr<FrameResources>;
			FrameResources(Device& device) : GPUEntityStorage<FrameAllocationPolicy>(device)
			{
				
			}
			~FrameResources()
			{
				reset();
			}

			std::uint64_t get_frame()
			{
				return frame_number;
			}

			std::shared_ptr<CommandList> start_list(std::string name = "", CommandListType type = CommandListType::DIRECT);
		};

		class FrameResourceManager
		{
			std::atomic_size_t frame_number = 0;
			Device& device;
		public:
			FrameResourceManager(Device& device) :device(device){}
			FrameResources::ptr begin_frame();
		};

}