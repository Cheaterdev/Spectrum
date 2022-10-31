module Graphics;
import Log;

import Debug;

import IdGenerator;
import StateContext;
import Data;
import Debug;


namespace HAL
{
	void  Device::stop_all()
	{
		for (auto&& q : queues)
			q = nullptr;


		Singleton<Graphics::StaticCompiledGPUData>::reset();
		HAL::DescriptorHeapManager::reset();
		HAL::PipelineStateCache::reset();

		HAL::HeapFactory::reset();
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
	}



}
