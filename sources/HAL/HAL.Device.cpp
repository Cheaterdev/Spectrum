module HAL:Device;
import Core;
import HAL;
import :FrameManager;

import :DescriptorHeap;
import :HeapAllocators;
import :PSO;

namespace HAL
{
	void  Device::stop_all()
	{
		for (auto&& q : queues)
			q = nullptr;

	}

	Device::~Device()
	{
		stop_all();

		pipeline_state_cache = nullptr;
	}

	std::shared_ptr<CommandList> Device::get_upload_list()
	{
		auto list = (queues[CommandListType::COPY]->get_free_list());
		list->begin("UploadList");
		return list;
	}

	HAL::Queue::ptr& Device::get_queue(CommandListType type)
	{
		return queues[type];
	}

	std::shared_ptr<Device> Device::create_singleton()
	{
		HAL::init();

		std::shared_ptr<Device> result;
		HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter)
			{

				DXGI_ADAPTER_DESC adapter_desc = adapter->get_desc();
				Log::get() << "adapter: " << adapter_desc.Description << Log::endl;

				HAL::DeviceDesc desc;
				desc.adapter = adapter;
				auto device = std::make_shared<Device>(desc);

				const auto& props = device->get_properties();


				if (props.mesh_shader && props.full_bindless)
				{
					Log::get() << "Selecting adapter: " << adapter_desc.Description << Log::endl;
					result = device;
				}
			});

		result->init_managers();
		return result;
	}

	Device::Device(HAL::DeviceDesc desc)
	{
		init(desc);


	}

	void Device::init_managers()
	{
		for (auto type : magic_enum::enum_values<CommandListType>())
			queues[type] = std::make_shared<HAL::Queue>(type, this);

		ds_queue = std::make_unique<DirectStorageQueue>(*this);
		rtx = !Debug::RunForPix && get_properties().rtx;

		heap_factory = std::make_unique<HeapFactory>(*this);
		descriptor_heap_factory = std::make_unique<DescriptorHeapFactory>(*this);
		query_heap_factory = std::make_unique<QueryHeapFactory>(*this);



		frame_manager = std::make_unique<FrameResourceManager>(*this);
		static_gpu_data = std::make_unique<StaticCompiledGPUData>(*this);

		pipeline_state_cache = std::make_unique<PipelineStateCache>(*this);

		engine_pso_holder = std::make_unique<EnginePSOHolder>();

		engine_pso_holder->init(*this);

		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			command_allocators[type].create_func = [type]() {
				return std::make_shared<CommandAllocator>(type);

			};
		}


	}
	FrameResourceManager& Device::get_frame_manager()
	{
		return *frame_manager;
	}

	HeapFactory& Device::get_heap_factory()
	{
		return *heap_factory;
	}


	StaticCompiledGPUData& Device::get_static_gpu_data() {
		return *static_gpu_data;
	}

	PipelineStateCache& Device::get_pipeline_state_cache() {
		return *pipeline_state_cache;
	}
	EnginePSOHolder& Device::get_engine_pso_holder() {
		return *engine_pso_holder;
	}

	QueryHeapFactory& Device::get_query_heap_factory()
	{
		return *query_heap_factory;

	}
	DescriptorHeapFactory& Device::get_descriptor_heap_factory()
	{
		return *descriptor_heap_factory;
	}

	DirectStorageQueue& Device::get_ds_queue()
	{
		return *ds_queue;
	}
	void Device::free_ca(std::shared_ptr<CommandAllocator> e)
	{
		e->reset();
		command_allocators[e->get_type()].put(e);
	}

	std::shared_ptr<CommandAllocator> Device::get_ca(CommandListType type) {
		return command_allocators[type].get();
	}
}
