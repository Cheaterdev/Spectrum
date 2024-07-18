module HAL:Resource;
import <HAL.h>;
import :HeapAllocators;
import :API.Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;
import :CommandList;
import :Queue;


import :Resource.Buffer;
import :Resource.Texture;

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
	std::mutex t;

	void Resource::write(GPUBinaryData<true>& data)
	{
		std::lock_guard<std::mutex> g(t);

		Log::get()<<data.operation.path<<Log::endl;
		StorageRequest request;

		request.resource = get_ptr();
		request.file = data.operation.path;
		request.file_offset = data.operation.file_offset;
		request.size = data.get_size();
		request.uncompressed_size = data.uncompressed_size;
		request.compressed = request.uncompressed_size!= data.get_size();

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

		load_waiter = Device::get().get_ds_queue().execute(request);
		
		Log::get()<<1<<Log::endl;
		Device::get().get_ds_queue().flush();
		
		Log::get()<<2<<Log::endl;
		load_waiter.wait();
		
		Log::get()<<3<<Log::endl;
	}

}


