module HAL:DescriptorHeap;


import HAL;
import :Types;

import Core;

namespace HAL {

	DescriptorHeapType get_heap_type(HandleType type)
	{
		switch (type)
		{
		case HandleType::CBV:
		case HandleType::SRV:
		case HandleType::UAV:
			return DescriptorHeapType::CBV_SRV_UAV;

		case HandleType::RTV:
			return DescriptorHeapType::RTV;

		case HandleType::DSV:
			return DescriptorHeapType::DSV;

		case HandleType::SMP:
			return DescriptorHeapType::SAMPLER;
		}

		return DescriptorHeapType();
	}
	ResourceInfo& Handle::get_resource_info() const
	{
		return storage->get_heap()->get_resource_info(get_offset());
	}
	Handle::operator bool() const
	{
		return is_valid();
	}
	DescriptorHeap::DescriptorHeap(Device & device, uint num, DescriptorHeapType type, HAL::DescriptorHeapFlags flags) :API::DescriptorHeap(device, HAL::DescriptorHeapDesc{ num, type , flags })
	{

		descriptor_size = device.get_descriptor_size(type);

		resources.resize(num);
	}


	void Handle::place(const Handle& r) const
	{

		auto& my_heap = *storage->get_heap();
		auto& other_heap = *r.storage->get_heap();

		my_heap[get_offset()] = other_heap[r.get_offset()];
		get_resource_info() = r.get_resource_info();
	}


	void ResourceInfo::for_each_subres(std::function<void(const HAL::Resource::ptr&, UINT)> f) const
	{
		std::visit(overloaded{
			[&](const HAL::Views::ShaderResource& v) {
				std::visit(overloaded{
					[&](const HAL::Views::ShaderResource::Buffer&) { f(v.Resource, ALL_SUBRESOURCES); },
					[&](const HAL::Views::ShaderResource::Texture2D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0 && desc.is2D())
							f(v.Resource, ALL_SUBRESOURCES);
						else
							for (auto mip = t.MostDetailedMip; mip < t.MostDetailedMip + t.MipLevels; mip++)
								f(v.Resource, desc.CalcSubresource(mip, 0, t.PlaneSlice));
					},
					[&](const HAL::Views::ShaderResource::Texture2DArray& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0 && t.FirstArraySlice == 0 && t.ArraySize == desc.ArraySize && desc.is2D())
							f(v.Resource, ALL_SUBRESOURCES);
						else
							for (auto mip = t.MostDetailedMip; mip < t.MostDetailedMip + t.MipLevels; mip++)
								for (auto arr = t.FirstArraySlice; arr < t.FirstArraySlice + t.ArraySize; arr++)
									f(v.Resource, desc.CalcSubresource(mip, arr, t.PlaneSlice));
					},
					[&](const HAL::Views::ShaderResource::Texture3D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0)
							f(v.Resource, ALL_SUBRESOURCES);
						else
							for (auto mip = t.MostDetailedMip; mip < t.MostDetailedMip + t.MipLevels; mip++)
								f(v.Resource, desc.CalcSubresource(mip, 0, 0));
					},
					[&](const HAL::Views::ShaderResource::Cube& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0)
							f(v.Resource, ALL_SUBRESOURCES);
						else
							for (auto mip = t.MostDetailedMip; mip < t.MostDetailedMip + t.MipLevels; mip++)
								for (auto arr = 0; arr < 6; arr++)
									f(v.Resource, desc.CalcSubresource(mip, arr, 0));
					},
					[&](const HAL::Views::ShaderResource::Raytracing&) { f(v.Resource, ALL_SUBRESOURCES); },
					[&](auto) { ASSERT(false); }
				}, v.View);
			},
			[&](const HAL::Views::UnorderedAccess& v) {
				std::visit(overloaded{
					[&](const HAL::Views::UnorderedAccess::Buffer& b) {
						f(v.Resource, ALL_SUBRESOURCES);
						if (b.CounterResource) f(b.CounterResource, ALL_SUBRESOURCES);
					},
					[&](const HAL::Views::UnorderedAccess::Texture2D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (desc.MipLevels == 1 && desc.is2D())
							f(v.Resource, ALL_SUBRESOURCES);
						else
							f(v.Resource, desc.CalcSubresource(t.MipSlice, 0, t.PlaneSlice));
					},
					[&](const HAL::Views::UnorderedAccess::Texture2DArray& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (desc.MipLevels == 1 && desc.is2D() && desc.ArraySize == 1)
							f(v.Resource, ALL_SUBRESOURCES);
						else
							for (auto arr = t.FirstArraySlice; arr < t.FirstArraySlice + t.ArraySize; arr++)
								f(v.Resource, desc.CalcSubresource(t.MipSlice, arr, t.PlaneSlice));
					},
					[&](const HAL::Views::UnorderedAccess::Texture3D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (t.FirstWSlice == 0 && t.WSize == desc.Dimensions.z && desc.MipLevels == 1)
							f(v.Resource, ALL_SUBRESOURCES);
						else
							f(v.Resource, desc.CalcSubresource(t.MipSlice, 0, 0));
					},
					[&](auto) { ASSERT(false); }
				}, v.View);
			},
			[&](const HAL::Views::RenderTarget& v) {
				std::visit(overloaded{
					[&](const HAL::Views::RenderTarget::Texture2D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						if (desc.MipLevels == 1 && desc.is2D() == 1)
							f(v.Resource, ALL_SUBRESOURCES);
						else
							f(v.Resource, desc.CalcSubresource(t.MipSlice, 0, t.PlaneSlice));
					},
					[&](const HAL::Views::RenderTarget::Texture2DArray& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						for (auto arr = t.FirstArraySlice; arr < t.FirstArraySlice + t.ArraySize; arr++)
							f(v.Resource, desc.CalcSubresource(t.MipSlice, arr, t.PlaneSlice));
					},
					[&](auto) { ASSERT(false); }
				}, v.View);
			},
			[&](const HAL::Views::DepthStencil& v) {
				std::visit(overloaded{
					[&](const HAL::Views::DepthStencil::Texture2D& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						f(v.Resource, desc.CalcSubresource(t.MipSlice, 0, 0));
					},
					[&](const HAL::Views::DepthStencil::Texture2DArray& t) {
						auto& desc = v.Resource->get_desc().as_texture();
						for (auto arr = t.FirstArraySlice; arr < t.FirstArraySlice + t.ArraySize; arr++)
							f(v.Resource, desc.CalcSubresource(t.MipSlice, arr, 0));
					},
					[&](auto) { ASSERT(false); }
				}, v.View);
			},
			[&](const HAL::Views::ConstantBuffer& v) { f(v.Resource, ALL_SUBRESOURCES); },
			[&](auto) { ASSERT(false); }
		}, view);
	}

	Resource* ResourceInfo::get_resource() const
	{
		Resource* result = nullptr;
		std::visit(overloaded{
			[&](const HAL::Views::ShaderResource& v)   { result = v.Resource.get(); },
			[&](const HAL::Views::UnorderedAccess& v)  { result = v.Resource.get(); },
			[&](const HAL::Views::RenderTarget& v)     { result = v.Resource.get(); },
			[&](const HAL::Views::DepthStencil& v)     { result = v.Resource.get(); },
			[&](const HAL::Views::ConstantBuffer& v)   { result = v.Resource.get(); },
			[&](auto) { ASSERT(false); }
		}, view);
		return result;
	}

	bool ResourceInfo::is_valid() const
	{
		return view.index() != 0;
	}

	ResourceInfo& DescriptorHeap::get_resource_info(uint offset)
	{
		return resources[offset];
	}

	DescriptorHeapStorage::DescriptorHeapStorage(const DescriptorHeapAllocation& handle, bool deferred_gpu_write) : handle(handle), deferred_gpu_write(deferred_gpu_write) {}

	DescriptorHeapStorage::~DescriptorHeapStorage()
	{
		if (handle.CanFree())
		{
			auto& heap = *get_heap();
			for (uint i = 0; i < get_count(); i++)
			{
				heap.get_resource_info(get_offset() + i) = HAL::Views::Null();
			}
			handle.Free();
		}
	}

	HAL::DescriptorHeap::ptr DescriptorHeapStorage::get_heap() const { return handle.get_heap(); }
	bool DescriptorHeapStorage::is_valid() const { return !!handle; }
	uint DescriptorHeapStorage::get_offset() const { return static_cast<uint>(handle.get_offset()); }
	uint DescriptorHeapStorage::get_count() const { return static_cast<uint>(handle.get_size()); }
	bool DescriptorHeapStorage::can_free() const { return handle.CanFree(); }
	std::shared_ptr<DescriptorHeapStorage> DescriptorHeapStorage::get_tracked() { return get_ptr<DescriptorHeapStorage>(); }

	Handle::Handle(std::shared_ptr<DescriptorHeapStorage> storage, UINT offset) : storage(storage), offset(offset) {}

	bool Handle::is_valid() const
	{
		return storage && (offset != std::numeric_limits<uint>::max()) && storage->get_heap() != nullptr;
	}

	bool Handle::operator!=(const Handle& r)
	{
		if (offset != r.offset) return true;
		return false;
	}

	uint Handle::get_count() const
	{
		if (!storage) return 0;
		ASSERT(offset == 0);
		return storage->get_count();
	}

	void Handle::operator=(const Handle& r)
	{
		// Plain, unchecked copy -- this is used for generic whole-object
		// handle copies (e.g. StructuredBufferView's move-assignment
		// memberwise-copying every sub-handle, most structurally
		// inapplicable to the underlying resource and never read by
		// anything). The "is this handle actually safe to bind for shader
		// use" check lives at the narrower, correct point: the
		// HasTexture2D-etc.-constrained operator= overloads in HAL.HLSL.ixx
		// that pull ONE named view field out of a source struct to bind
		// into a Table field -- see is_written()'s comment.
		storage = r.storage;
		offset = r.offset;
		written = r.written;
	}

	Handle Handle::operator[](uint i) const
	{
		ASSERT(offset == 0);
		ASSERT(i < storage->get_count());
		return Handle(storage, i);
	}

	uint Handle::get_offset() const
	{
		return storage->get_offset() + offset;
	}

	std::shared_ptr<DescriptorHeapStorage> Handle::get_storage() const { return storage; }

	D3D12_CPU_DESCRIPTOR_HANDLE Handle::get_cpu() const
	{
		auto& heap = *storage->get_heap();
		return heap[get_offset()].get_cpu();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE Handle::get_gpu() const
	{
		auto& heap = *storage->get_heap();
		return heap[get_offset()].get_gpu();
	}

	DescriptorHeapFactory::ptr_type DescriptorHeapFactory::make_heap(DescriptorHeapIndex index, size_t size)
	{
		if (check(index.flags & DescriptorHeapFlags::ShaderVisible))
		{
			if (index.type == DescriptorHeapType::CBV_SRV_UAV)
				return gpu_cbv_srv_uav;
			else
				return gpu_sampler;
		}
		ASSERT(index.type != DescriptorHeapType::CBV_SRV_UAV);
		return std::make_shared<HAL::DescriptorHeap>(device, static_cast<uint>(size), index.type, index.flags);
	}

	DescriptorHeapFactory::DescriptorHeapFactory(Device& device) : device(device)
	{
		gpu_sampler = std::make_shared<HAL::DescriptorHeap>(device, 2048, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::ShaderVisible);
		gpu_cbv_srv_uav = std::make_shared<HAL::DescriptorHeap>(device, 65536 * 8, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::ShaderVisible);
	}

	DescriptorHeapFactory::ptr_type DescriptorHeapFactory::get_sampler_heap() { return gpu_sampler; }
	DescriptorHeapFactory::ptr_type DescriptorHeapFactory::get_cbv_srv_uav_heap() { return gpu_cbv_srv_uav; }

}
