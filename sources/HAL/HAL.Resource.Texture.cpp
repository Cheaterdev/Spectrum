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
			const auto s = range.subresources[i];
			// The readback rows are the backend's copy layout (D3D12: this same footprint; Vulkan: tight rows), so
			// the rows are moved one by one into the file layout.
			tasks.push_back(list->get_copy().read_texture(this, first_subresource + i, [&data, s](std::span<std::byte> memory, texture_layout layout)
				{
					ASSERT(layout.rows_count == s.rows);
					for (uint z = 0; z < s.depth; z++)
						for (uint y = 0; y < s.rows; y++)
						{
							const uint64 src = uint64(z) * layout.slice_stride + uint64(y) * layout.row_stride;
							ASSERT(src + s.row_size <= memory.size());
							std::memcpy(data.data() + s.offset + z * s.slice_pitch + uint64(y) * s.row_pitch, memory.data() + src, s.row_size);
						}
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

	TextureResource::TextureResource(Device& device, const ResourceDesc& desc, PlacementAddress handle, vec4 clear_value)
		: Resource(device, desc, handle, clear_value)
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


