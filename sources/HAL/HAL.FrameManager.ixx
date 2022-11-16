export module HAL:FrameManager;

import Core;
import :Types;
import :HeapAllocators;
import :DescriptorHeap;
import :Private.CommandListCompiler;
import :Device;
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

			GPUCompiledManager(Device& device):Uploader<LockPolicy>(device)
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


		class StaticCompiledGPUData : public GPUCompiledManager<Thread::Lockable>
		{
			Device& device;
		public:
			using Uploader<Thread::Lockable>::place_raw;

			StaticCompiledGPUData(Device& device) :device(device), GPUCompiledManager<Thread::Lockable>(device) {}
		};

		class FrameResources :public SharedObject<FrameResources>, public GPUCompiledManager<Thread::Lockable>
		{
			friend class FrameResourceManager;

			std::uint64_t frame_number = 0;
			std::mutex m;
		public:
			using ptr = std::shared_ptr<FrameResources>;
			FrameResources(Device& device) : GPUCompiledManager<Thread::Lockable>(device)
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