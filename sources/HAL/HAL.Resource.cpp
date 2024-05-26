module HAL:Resource;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
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

	Resource::ptr create_resource(const HAL::ResourceDesc& desc, ResourceHandle heap_type)
	{
		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(desc, heap_type);

		return std::make_shared<TextureResource>(desc, heap_type);

	}

	Resource::ptr create_resource(const HAL::ResourceDesc& desc, HeapType heap_type)
	{
		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(desc, heap_type);

		return std::make_shared<TextureResource>(desc, heap_type);

	}
	void Buffer::write(std::vector<std::byte>& data)
	{
		auto list = (HAL::Device::get().get_upload_list());
		list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(data.data()), data.size());
		list->end();
		list->execute();
	}


	void Resource::write(GPUBinaryData<true>& data)
	{

		StorageRequest request;

		request.resource = get_ptr();
		request.file = data.operation.path;
		request.file_offset = data.operation.file_offset;
		request.size = data.get_size();
		request.uncompressed_size = data.uncompressed_size;
		request.compressed = true;

		std::visit(overloaded{
			[&](const GPUBinaryData<true>::Buffer& buffer) {
					request.operation = StorageRequest::Buffer{buffer.offset};
			},
			[&](const GPUBinaryData<true>::Texture& texture) {
			request.operation = StorageRequest::Texture{texture.subresource,texture.count};
			},
			[&](auto other) {
				assert(false);
			}
			}, data.desc);


		//uint64 uncompressed_size;
		load_waiter = Device::get().get_ds_queue().execute(request);
		//	HAL::Device::get().get_ds_queue().signal_and_wait();
	}


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


