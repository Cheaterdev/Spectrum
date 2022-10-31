export module HAL:FrameManager;

import Utils;
import StateContext;
import Profiling;
import Threading;
import Exceptions;
import :Types;
import Math;
import Data;
import Singleton;
import Debug;

import stl.core;
import stl.memory;

import :HeapAllocators;
import :DescriptorHeap;
import :Private.CommandListCompiler;
export
namespace HAL {

		template<class LockPolicy = Thread::Free>
		class GPUCompiledManager : public Uploader<LockPolicy>
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

			GPUCompiledManager()
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

				Uploader<LockPolicy>::reset();
			}

		};


		class StaticCompiledGPUData :public Singleton<StaticCompiledGPUData>, public GPUCompiledManager<Thread::Lockable>
		{
		public:
			using Uploader<Thread::Lockable>::place_raw;
		};

		class FrameResources :public SharedObject<FrameResources>, public GPUCompiledManager<Thread::Lockable>
		{
			friend class FrameResourceManager;

			std::uint64_t frame_number = 0;
			std::mutex m;
		public:
			using ptr = std::shared_ptr<FrameResources>;

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

		class FrameResourceManager :public Singleton<FrameResourceManager>
		{
			std::atomic_size_t frame_number = 0;

		public:
			FrameResources::ptr begin_frame();
		};

}