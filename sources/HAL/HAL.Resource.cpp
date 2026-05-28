module HAL:Resource;

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
	Resource::ptr create_resource(Device& device, const HAL::ResourceDesc& desc, ResourceHandle handle)
	{
		PROFILE(L"create_resource handle");

		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(device, desc, handle);
		return std::make_shared<TextureResource>(device, desc, handle);
	}

	Resource::ptr create_resource(Device& device, const HAL::ResourceDesc& desc, HeapType heap_type)
	{
		PROFILE(L"create_resource heap_type");
		if (desc.is_buffer())
			return std::make_shared<HAL::Buffer>(device, desc, heap_type);
		return std::make_shared<TextureResource>(device, desc, heap_type);
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
				ASSERT(false);
			}
			}, data.desc);

		load_waiter = get_device().get_ds_queue().execute(request);

		get_device().get_ds_queue().flush();
		load_waiter.wait();
	}

	Resource::Resource() : state_manager(this), tiled_manager(this) {}

	bool Resource::is_ready() const
	{
		return load_waiter.is_completed();
	}

	ResourceType Resource::get_type() const
	{
		if (desc.is_buffer())
			return ResourceType::Buffer;

		return ResourceType::Texture;
	}

	const ResourceDesc& Resource::get_desc() const
	{
		return desc;
	}

	ResourceStateManager& Resource::get_state_manager()
	{
		return state_manager;
	}

	TiledResourceManager& Resource::get_tiled_manager()
	{
		return tiled_manager;
	}

	std::shared_ptr<Resource> Resource::get_tracked()
	{
		return get_ptr<Resource>();
	}

	void Resource::disable_state_tracking()
	{
		desc.Flags |= ResFlags::DisableStateTracking;
	}

	HeapType Resource::get_heap_type() const
	{
		return heap_type;
	}

}


