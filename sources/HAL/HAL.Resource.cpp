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
	Resource::ptr create_resource(const HAL::ResourceDesc& desc, ResourceHandle handle)
	{
		PROFILE(L"create_resource handle");

		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(desc, handle);
		return std::make_shared<TextureResource>(desc, handle);
	}

	Resource::ptr create_resource(const HAL::ResourceDesc& desc, HeapType heap_type)
	{
		PROFILE(L"create_resource heap_type");
		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(desc, heap_type);
		return std::make_shared<TextureResource>(desc, heap_type);
	}
	std::mutex t;

	void Resource::write(GPUBinaryData<true>& data)
	{
		std::lock_guard<std::mutex> g(t);

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
		
		Device::get().get_ds_queue().flush();
		load_waiter.wait();
	}

}


