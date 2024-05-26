module HAL:Resource.Texture;
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

	std::vector<std::byte>  TextureResource::read(uint i)
	{
		std::vector<std::byte> data;

		auto list = (HAL::Device::get().get_upload_list());
		std::future<bool> task;

		//ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>
		task = list->get_copy().read_texture(this, i, [&data](std::span<std::byte> memory, texture_layout layout)
			{
				data.assign(memory.begin(), memory.end());

			});


		list->end();
		list->execute();
		task.wait();
		return data;
	}


}


