module HAL:Resource;

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

	void Resource::write(std::vector<std::byte>& data)
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
		request.size = data.size;


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


	std::vector<std::byte>  Resource::read(uint i)
	{
		std::vector<std::byte> data;

		auto list = (HAL::Device::get().get_upload_list());
		std::future<bool> task;
		if(desc.is_buffer())
		{ task = list->get_copy().read_buffer(this, 0, get_size(), [&data](std::span<std::byte> memory)
			{
							data.assign(memory.begin(), memory.end());

			});
			
		}else
		{
			//ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>
			task = list->get_copy().read_texture(this, i, [&data](std::span<std::byte> memory,texture_layout layout)
			{
				data.assign(memory.begin(),memory.end());

			});
		}
		
		list->end();
		list->execute();
		task.wait();
		return data;
	}

	

}


