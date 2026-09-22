module HAL:Resource.Texture;


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

		auto list = (get_device().get_upload_list());
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

	std::vector<std::byte> TextureResource::read_range(uint first_subresource, uint count)
	{
		auto range = get_device().get_texture_range_layout(get_desc(), first_subresource, count);
		std::vector<std::byte> data(range.total_size);

		auto list = (get_device().get_upload_list());
		std::vector<std::future<bool>> tasks;
		tasks.reserve(count);

		for (uint i = 0; i < count; i++)
		{
			uint64 offset = range.subresource_offsets[i];
			tasks.push_back(list->get_copy().read_texture(this, first_subresource + i, [&data, offset](std::span<std::byte> memory, texture_layout layout)
				{
					std::memcpy(data.data() + offset, memory.data(), memory.size());
				}));
		}

		list->end();
		list->execute();

		for (auto&& t : tasks)
			t.wait();

		return data;
	}

	void TextureResource::init() {}

	TextureResource::TextureResource(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout, vec4 clear_value)
		: Resource(device, desc, heap_type, initialLayout, clear_value)
	{
		init();
	}

	TextureResource::TextureResource(Device& device, const ResourceDesc& desc, PlacementAddress handle)
		: Resource(device, desc, handle)
	{
		init();
	}

	TextureResource::TextureResource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own)
		: Resource(device, desc, handle, own)
	{
		init();
	}

	TextureResource::TextureResource(Device& device, const API::NativeImportHandle& handle, TextureLayout initialLayout)
		: Resource(device, handle, initialLayout)
	{
		init();
	}

}


