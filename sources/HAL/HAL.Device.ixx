export module HAL:Device;

import Core;
import :API.Device;
export class EnginePSOHolder;
export namespace HAL
{
	class FrameResourceManager;
	class GPUTimeManager;
class HeapFactory;
class StaticCompiledGPUData;
class DescriptorHeapManager;
class ResourceHeapPageManager;
class PipelineStateCache;
	class Queue;

	class QueryHeapFactory;
	class QueryHeapPageManager;
		class Device : public Singleton<Device>, public API::Device
		{
			friend class API::Device;
			friend class Singleton<Device>;
		private:
			
			HAL::DeviceProperties properties;

			HAL::Adapter::ptr adapter;
			enum_array<HAL::CommandListType, std::shared_ptr<HAL::Queue>> queues;
			IdGenerator<Thread::Lockable> id_generator;
			friend class CommandList;
			bool rtx = false;
			std::unique_ptr<FrameResourceManager> frame_manager;
			std::unique_ptr<GPUTimeManager> time_manager;
			std::unique_ptr<HeapFactory> heap_factory;
			std::unique_ptr<StaticCompiledGPUData> static_gpu_data;
			std::unique_ptr<DescriptorHeapManager> descriptor_heap_manager;
			std::unique_ptr<ResourceHeapPageManager> resource_heap_manager;
		std::unique_ptr<PipelineStateCache> pipeline_state_cache;
		std::unique_ptr<EnginePSOHolder> engine_pso_holder;


		std::unique_ptr<QueryHeapFactory> query_heap_factory;
		std::unique_ptr<QueryHeapPageManager> query_heap_manager;

		public:
			Device(HAL::DeviceDesc desc);
			virtual ~Device();

			static std::shared_ptr<Device> create_singleton();


			void init_managers();
			void stop_all();
		
			const HAL::DeviceProperties& get_properties() const { return properties;}
			ContextGenerator context_generator;
			std::shared_ptr<CommandList> get_upload_list();
	
			std::shared_ptr<HAL::Queue>& get_queue(HAL::CommandListType type);

			bool is_rtx_supported() { return rtx; }

			mutable bool alive = true;

			FrameResourceManager& get_frame_manager();
			GPUTimeManager& get_time_manager();
			HeapFactory& get_heap_factory();
			StaticCompiledGPUData& get_static_gpu_data();
			DescriptorHeapManager& get_descriptor_heap_manager();
			ResourceHeapPageManager& get_resource_heap_manager();
			PipelineStateCache& get_pipeline_state_cache();
			EnginePSOHolder& get_engine_pso_holder();

			QueryHeapFactory& get_query_heap_factory();
			QueryHeapPageManager& get_query_heap_manager();
		};

	


}