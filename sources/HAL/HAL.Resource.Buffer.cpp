module HAL:Resource.Buffer;
import <HAL.h>;

import :HeapAllocators;
import :API.Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;
import :CommandList;
import :Queue;

import Core;

namespace HAL
{

	void Buffer::write(std::vector<std::byte>& data)
	{
		auto list = (HAL::Device::get().get_upload_list());
		list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(data.data()), data.size());
		list->end();
		list->execute();
	}

	std::vector<std::byte>  Buffer::read()
	{
		std::vector<std::byte> data;

		auto list = (HAL::Device::get().get_upload_list());
		std::future<bool> task;
		int t = 0;
		task = list->get_copy().read_buffer(this, 0, get_size(), [&](std::span<std::byte> memory)
			{
				assert(memory.size());
				data.assign(memory.begin(), memory.end());
				t = 1;
			});


		list->end();
		list->execute();
		task.wait();

		assert(t == 1);
		assert(data.size() > 0);
		return data;
	}

	void Buffer::init()
	{
		if (heap_type == HeapType::UPLOAD || heap_type == HeapType::READBACK)
		{
			assert(buffer_data == nullptr);
			get_dx()->Map(0, nullptr, reinterpret_cast<void**>(&buffer_data));
		}

		gpu_address = ResourceAddress{ this,0 };

	}

}


