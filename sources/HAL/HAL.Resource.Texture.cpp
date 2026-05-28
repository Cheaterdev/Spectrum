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

	void TextureResource::init() {}

	TextureResource::TextureResource(const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout, vec4 clear_value)
		: Resource(desc, heap_type, initialLayout, clear_value)
	{
		init();
	}

	TextureResource::TextureResource(const ResourceDesc& desc, PlacementAddress handle)
		: Resource(desc, handle)
	{
		init();
	}

	TextureResource::TextureResource(const ResourceDesc& desc, ResourceHandle handle, bool own)
		: Resource(desc, handle, own)
	{
		init();
	}

	TextureResource::TextureResource(const D3D::Resource& resouce, TextureLayout initialLayout)
		: Resource(resouce, initialLayout)
	{
		init();
	}

}


