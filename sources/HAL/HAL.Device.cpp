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
		auto list = (queues[CommandListType::DIRECT]->get_free_list());
		list->begin(L"UploadList");
		return list;
	}

	void Device::register_promote(std::shared_ptr<Resource> resource)
	{
		std::lock_guard<std::mutex> g(upload_mutex);
		pending_promotes.push_back(std::move(resource));
	}

	void Device::flush_uploads()
	{
		std::vector<std::shared_ptr<Resource>> batch;
		{
			std::lock_guard<std::mutex> g(upload_mutex);
			if (pending_promotes.empty()) return;
			batch.swap(pending_promotes);
		}

		auto list = get_upload_list();

		std::vector<std::shared_ptr<Resource>> promoted;
		promoted.reserve(batch.size());

		for (auto& r : batch)
		{
			if (r->get_desc().is_buffer()) continue;   // buffers carry no layout

			auto layout = resting_layout(r.get());
			// Only promote resources that actually have a read state.
			if (!check(layout & (TextureLayout::SHADER_RESOURCE | TextureLayout::UNORDERED_ACCESS)))
				continue;

			// Record the read use. Nothing has to be pinned afterwards: the
			// resting layout is derived from the resource's own flags, and this
			// list returns it there when it is done.
			list->add_resource_usage(r.get(), check(layout & TextureLayout::SHADER_RESOURCE)
				? ResourceStates::SHADER_RESOURCE
				: ResourceStates::UNORDERED_ACCESS);
			promoted.push_back(r);
		}

		// Submit on the DIRECT queue and wait. This runs on the MAIN thread at
		// frame-begin (unlike the old worker-thread finish_upload that deadlocked),
		// and only when there are pending promotes — a one-time cost after loads.
		// Waiting keeps the list alive until the GPU is done and orders the
		// transition ahead of this frame's rendering.
		list->execute_and_wait();
	}

	HAL::Queue::ptr& Device::get_queue(CommandListType type)
	{
		return queues[type];
	}

	Device::Device(HAL::DeviceDesc desc)
	{
		init(desc);


	}

	void Device::init_managers()
	{
		for (auto type : magic_enum::enum_values<CommandListType>())
			queues[type] = std::make_shared<HAL::Queue>(type, *this);


		//auto another_direct = std::make_shared<HAL::Queue>(CommandListType::DIRECT, this);
		ds_queue = std::make_unique<DirectStorageQueue>(*this);
		rtx = !Debug::RunForPix && get_properties().rtx;

		heap_factory = std::make_unique<HeapFactory>(*this);
		descriptor_heap_factory = std::make_unique<DescriptorHeapFactory>(*this);
		query_heap_factory = std::make_unique<QueryHeapFactory>(*this);



		frame_manager = std::make_unique<FrameResourceManager>(*this);
		static_gpu_data = std::make_unique<StaticCompiledGPUData>(*this);

		pipeline_state_cache = std::make_unique<PipelineStateCache>(*this);

		engine_root_layout_holder = std::make_unique<EngineRootLayoutHolder>();
		engine_root_layout_holder->init(*this);

		engine_pso_holder = std::make_unique<EnginePSOHolder>();
		engine_pso_holder->init(*this);

		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			command_allocators[type].create_func = [this, type]() {
				return std::make_shared<CommandAllocator>(*this, type);
			};
		}


	}
	FrameResourceManager& Device::get_frame_manager() const
	{
		return *frame_manager;
	}

	HeapFactory& Device::get_heap_factory() const
	{
		return *heap_factory;
	}


	StaticCompiledGPUData& Device::get_static_gpu_data() const
	{
		return *static_gpu_data;
	}

	PipelineStateCache& Device::get_pipeline_state_cache() const
	{
		return *pipeline_state_cache;
	}
	EngineRootLayoutHolder& Device::get_engine_root_layout_holder() const
	{
		return *engine_root_layout_holder;
	}

	EnginePSOHolder& Device::get_engine_pso_holder() const
	{
		return *engine_pso_holder;
	}

	QueryHeapFactory& Device::get_query_heap_factory() const
	{
		return *query_heap_factory;

	}
	DescriptorHeapFactory& Device::get_descriptor_heap_factory() const
	{
		return *descriptor_heap_factory;
	}

	DirectStorageQueue& Device::get_ds_queue() const
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

	const HAL::DeviceProperties& Device::get_properties() const { return properties; }
	bool Device::is_rtx_supported() { return rtx; }
}
