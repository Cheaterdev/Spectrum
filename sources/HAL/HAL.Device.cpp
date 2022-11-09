module HAL:Device;
import Core;
import HAL;
import :FrameManager;
import :GPUTimer;
import :HeapAllocators;

namespace HAL
{
	void  Device::stop_all()
	{
		for (auto&& q : queues)
			q = nullptr;


		HAL::PipelineStateCache::reset();
	}

	Device::~Device()
	{
		stop_all();
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

				const auto &props = device->get_properties();


				if (props.mesh_shader && props.full_bindless)
				{
					Log::get() << "Selecting adapter: " << adapter_desc.Description << Log::endl;
					result = device;
				}
			});
		return result;
	}

	Device::Device(HAL::DeviceDesc desc)
	{
		init(desc);

		queues[CommandListType::DIRECT].reset(new HAL::Queue(CommandListType::DIRECT, this));
		queues[CommandListType::COMPUTE].reset(new HAL::Queue(CommandListType::COMPUTE, this));
		queues[CommandListType::COPY].reset(new HAL::Queue(CommandListType::COPY, this));

		rtx = get_properties().rtx;

		frame_manager = std::make_unique<FrameResourceManager>(*this);
		time_manager = std::make_unique<GPUTimeManager>(*this);
		heap_factory = std::make_unique<HeapFactory>(*this);

		static_gpu_data = std::make_unique<StaticCompiledGPUData>(*this);
		descriptor_heap_manager = std::make_unique<DescriptorHeapManager>(*this);
		resource_heap_manager = std::make_unique<ResourceHeapPageManager>(*this);

	}


	FrameResourceManager& Device::get_frame_manager()
	{
		return *frame_manager;
	}

	GPUTimeManager& Device::get_time_manager()
	{
		return *time_manager;
	}
	HeapFactory& Device::get_heap_factory()
	{
		return *heap_factory;
	}


	StaticCompiledGPUData& Device::get_static_gpu_data() {
		return *static_gpu_data;
	}
	DescriptorHeapManager& Device::get_descriptor_heap_manager() {
		return *descriptor_heap_manager;
	}
	ResourceHeapPageManager& Device::get_resource_heap_manager() {
		return *resource_heap_manager;
	}
}
