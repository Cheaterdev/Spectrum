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






		template<class LockPolicy = Thread::Free>
		class GPUCompiledManager 
		{
			enum_array<HAL::DescriptorHeapType, typename HAL::DynamicDescriptor<LockPolicy>::ptr> cpu_heaps;
			enum_array<HAL::DescriptorHeapType, typename HAL::DynamicDescriptor<LockPolicy>::ptr> gpu_heaps;

		public:

			HAL::DynamicDescriptor<LockPolicy>& get_cpu_heap(HAL::DescriptorHeapType type)
			{
				assert(cpu_heaps[type]);
				return *cpu_heaps[type];
			}

			HAL::DynamicDescriptor<LockPolicy>& get_gpu_heap(HAL::DescriptorHeapType type)
			{
				assert(cpu_heaps[type]);
				return *gpu_heaps[type];
			}

			GPUCompiledManager(Device& device)
			{
				gpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE);
				gpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE);

				cpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::RTV] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::RTV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::DSV] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::DSV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<HAL::DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE);
			}

			void reset()
			{

				for (auto& h : gpu_heaps)
					if (h)
						h->reset();

				for (auto& h : cpu_heaps)
					if (h)
						h->reset();
				
			}

		};

	
		template<class AllocationPolicy>
		struct GPUEntityStorage:
			public CPUGPUAllocator<AllocationPolicy>,
			public GPUCompiledManager<typename AllocationPolicy::LockPolicy>,
			public QueryHeapPageManager<AllocationPolicy>
		{

			GPUEntityStorage(Device& device):
			CPUGPUAllocator<AllocationPolicy>(device),
			QueryHeapPageManager<AllocationPolicy>(device),
			GPUCompiledManager<typename AllocationPolicy::LockPolicy>(device)
			{
				
			}
			CPUGPUAllocator<AllocationPolicy>::HandleType alloc_memory(size_t size, size_t alignment, CPUGPUAllocator<AllocationPolicy>::HeapMemoryOptions options)
			{
				return CPUGPUAllocator<AllocationPolicy>::alloc(size, alignment, options);
			}

			QueryHeapPageManager<AllocationPolicy>::HandleType  alloc_query(size_t size, size_t alignment, QueryHeapPageManager<AllocationPolicy>::HeapMemoryOptions options)
			{
				return QueryHeapPageManager<AllocationPolicy>::alloc(size,alignment,options);
			}

			void reset() requires (std::is_same_v<typename AllocationPolicy::AllocatorType,LinearAllocator>)
			{
				CPUGPUAllocator<AllocationPolicy>::reset();
				QueryHeapPageManager<AllocationPolicy>::reset();
				GPUCompiledManager<typename AllocationPolicy::LockPolicy>::reset();
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