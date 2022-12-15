module HAL:Resource;

import :HeapAllocators;
import :API.Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;
import :CommandList;

import Core;

namespace HAL
{

	void Resource::write(std::vector<std::byte>& data)
	{
		auto list = (HAL::Device::get().get_upload_list());
		list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(data.data()), data.size());
		list->end();
		list->execute();
	}
	std::vector<std::byte>  Resource::read()
	{
		std::vector<std::byte> data;

		auto list = (HAL::Device::get().get_upload_list());
		auto task = list->get_copy().read_buffer(this, 0, get_size(), [&data](const char* mem, UINT64 size)
			{
				data.assign((std::byte*)mem, (std::byte*)mem + size);

			});
		list->end();
		list->execute();
		task.wait();
		return data;
	}

	

}


