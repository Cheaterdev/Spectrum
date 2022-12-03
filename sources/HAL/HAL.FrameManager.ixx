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




			template<class T>
		concept GPUEntityStorageType = requires () { T::IsGPUEntityStorage; };
	
		template<class AllocationPolicy>
		struct GPUEntityStorage:
			public CPUGPUAllocator<AllocationPolicy>,
			public DescriptorHeapPageManager<AllocationPolicy>,
			public QueryHeapPageManager<AllocationPolicy>
		{
			using IsGPUEntityStorage = bool;

			GPUEntityStorage(Device& device):
				CPUGPUAllocator<AllocationPolicy>(device),
				QueryHeapPageManager<AllocationPolicy>(device),
				DescriptorHeapPageManager<AllocationPolicy>(device)
			{
				
			}
			CPUGPUAllocator<AllocationPolicy>::HandleType alloc_memory(size_t size, size_t alignment, CPUGPUAllocator<AllocationPolicy>::HeapMemoryOptions options)
			{
				return CPUGPUAllocator<AllocationPolicy>::alloc(size, alignment, options);
			}

			QueryHeapPageManager<AllocationPolicy>::HandleType  alloc_query(uint size,  QueryHeapPageManager<AllocationPolicy>::HeapMemoryOptions options)
			{
				return QueryHeapPageManager<AllocationPolicy>::alloc(size,1,options);
			}


			Handle  alloc_descriptor(uint size, DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions options)
			{
				auto h = DescriptorHeapPageManager<AllocationPolicy>::alloc(size, 1, options);
				
				return Handle{std::make_shared<DescriptorHeapStorage>(h),0};
			}

			void reset() requires (std::is_same_v<typename AllocationPolicy::AllocatorType,LinearAllocator>)
			{
				CPUGPUAllocator<AllocationPolicy>::reset();
				QueryHeapPageManager<AllocationPolicy>::reset();
				DescriptorHeapPageManager<AllocationPolicy>::reset();
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