export module HAL:Device;

import Core;
import :API.Device;

import :Types;

export class EnginePSOHolder;
export namespace HAL
{
	class FrameResourceManager;

	class HeapFactory;
	class StaticCompiledGPUData;
	class PipelineStateCache;
	class Queue;
	class DirectStorageQueue;
	class DescriptorHeapFactory;
	class QueryHeapFactory;
	class GPUTimeProfiler;
	class GPUEntityStorageInterface;
	class CommandAllocator;
	class Device : public Singleton<Device>, public API::Device, public TypedObject<Device>
	{
		friend class API::Device;
		friend class Singleton<Device>;
	private:

		HAL::DeviceProperties properties;

		HAL::Adapter::ptr adapter;
		enum_array<HAL::CommandListType, std::shared_ptr<HAL::Queue>> queues;

		std::shared_ptr<HAL::DirectStorageQueue> ds_queue;

		IdGenerator<Thread::Lockable> id_generator;
		friend class CommandList;
		bool rtx = false;
		std::unique_ptr<FrameResourceManager> frame_manager;

		std::unique_ptr<HeapFactory> heap_factory;
		std::unique_ptr<StaticCompiledGPUData> static_gpu_data;

		std::unique_ptr<PipelineStateCache> pipeline_state_cache;
		std::unique_ptr<EnginePSOHolder> engine_pso_holder;


		std::unique_ptr<QueryHeapFactory> query_heap_factory;
		std::unique_ptr<DescriptorHeapFactory> descriptor_heap_factory;

		enum_array<CommandListType, Pool<std::shared_ptr<CommandAllocator>>> command_allocators;

	public:
		Device(HAL::DeviceDesc desc);
		virtual ~Device();

		static std::shared_ptr<Device> create_singleton();


		void init_managers();
		void stop_all();

		const HAL::DeviceProperties& get_properties() const;
		ContextGenerator context_generator;
		std::shared_ptr<CommandList> get_upload_list();

		std::shared_ptr<HAL::Queue>& get_queue(HAL::CommandListType type);

		bool is_rtx_supported();

		mutable bool alive = true;

		FrameResourceManager& get_frame_manager() const;
		HeapFactory& get_heap_factory() const;
		StaticCompiledGPUData& get_static_gpu_data() const;

		PipelineStateCache& get_pipeline_state_cache() const;
		EnginePSOHolder& get_engine_pso_holder() const;

		QueryHeapFactory& get_query_heap_factory() const;
		DescriptorHeapFactory& get_descriptor_heap_factory() const;


		DirectStorageQueue& get_ds_queue() const;

		texture_layout get_texture_layout(const ResourceDesc& desc, UINT sub_resource, ivec3 box);


		texture_layout get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource);

		std::vector<std::byte> compress(std::span<std::byte>);



		std::shared_ptr<CommandAllocator> get_ca(CommandListType);

		void free_ca(std::shared_ptr<CommandAllocator>);
	};




}