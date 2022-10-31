module Graphics;
import HAL;
import :Types;

import Utils;

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
	ResourceInfo* Handle::get_resource_info() const
	{

		assert(heap);
		return heap->get_resource_info(this);
	}
	Handle::operator bool() const
	{
		return !!heap;
	}
	std::shared_ptr<DescriptorHeapManager> DescriptorHeapManager::create_singleton()
	{
		return std::make_shared<DescriptorHeapManager>(HAL::Device::get());
	}

	DescriptorHeapManager::DescriptorHeapManager(Device&device)
	{
		heap_rt.reset(new DescriptorHeap(device,65536, DescriptorHeapType::RTV, HAL::DescriptorHeapFlags::NONE));
		heap_ds.reset(new DescriptorHeap(device, 65536, DescriptorHeapType::DSV, HAL::DescriptorHeapFlags::NONE));


		heaps[DescriptorHeapType::CBV_SRV_UAV].reset(new DescriptorHeapPaged(device, 65536 * 8, DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::SHADER_VISIBLE));
		heaps[DescriptorHeapType::SAMPLER].reset(new DescriptorHeapPaged(device, 2048, DescriptorHeapType::SAMPLER, HAL::DescriptorHeapFlags::SHADER_VISIBLE));

		//	cpu_heaps[DescriptorHeapType::CBV_SRV_UAV].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
		heaps[DescriptorHeapType::RTV].reset(new DescriptorHeapPaged(device, 65536, DescriptorHeapType::RTV, HAL::DescriptorHeapFlags::NONE));
		heaps[DescriptorHeapType::DSV].reset(new DescriptorHeapPaged(device, 65536, DescriptorHeapType::DSV, HAL::DescriptorHeapFlags::NONE));
		//	cpu_heaps[DescriptorHeapType::SAMPLER].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE));
	}


	DescriptorHeap::DescriptorHeap(Device & device, UINT num, DescriptorHeapType type, HAL::DescriptorHeapFlags flags) :API::DescriptorHeap(device, HAL::DescriptorHeapDesc{ num, type , flags }), flags(flags)
	{
		max_count = num;

		descriptor_size = device.get_descriptor_size(type);
		//	assert(m_GPUHeap);

		resources.resize(max_count);
	}


	void Handle::place(const Handle& r) const
	{
		(*heap)[offset] = (*r.heap)[r.offset];
		*get_resource_info() = *r.get_resource_info();
	}

	Handle DescriptorPage::place()
	{
		Handle h = heap->handle(heap_offset + (offset++));
		h.get_resource_info()->resource_ptr = nullptr;
		return h;
	}



	HandleTableLight DescriptorPage::place(UINT count)
	{
		assert(offset + count <= this->count);
		auto table = heap->get_light_table_view(heap_offset + offset, count);

		for (UINT i = 0; i < (UINT)table.get_count(); i++)
		{
			auto h = table[i];
			h.get_resource_info()->resource_ptr = nullptr;
		}
		offset += count;
		return table;
	}



	HandleTable DescriptorPage::place2(UINT count)
	{
		assert(offset + count <= this->count);
		auto table = heap->make_table(count, heap_offset + offset);

		for (UINT i = 0; i < (UINT)table.get_count(); i++)
		{
			auto h = table[i];
			h.get_resource_info()->resource_ptr = nullptr;
		}
		offset += count;
		return table;
	}
	void DescriptorPage::free()
	{
		offset = 0;
		heap->free_page(this);
	}

	DescriptorPage* DescriptorHeapPaged::create_page(UINT count)
	{

		UINT pages_count = (count + 32 - 1) / 32;

		std::lock_guard<std::mutex> g(m);
		auto handle = allocator.Allocate(pages_count, 1);

		return new DescriptorPage(std::dynamic_pointer_cast<DescriptorHeapPaged>(shared_from_this()), handle, (UINT)(handle.get_offset() * 32), pages_count * 32);
	}

	void DescriptorHeapPaged::free_page(DescriptorPage* page)
	{

		std::lock_guard<std::mutex> g(m);
		allocator.Free(page->alloc_handle);
		delete page;
	}

	void for_each(const ResourceInfo* info, const HAL::Views::RenderTarget& view, std::function<void(HAL::Resource*, UINT)> f)
	{

		std::visit(overloaded{
			[&](const HAL::Views::RenderTarget::Texture2D& Texture2D) {
				auto& desc = info->resource_ptr->get_desc().as_texture();

				if (desc.MipLevels == 1 && desc.is2D() == 1)
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, Texture2D.PlaneSlice);
				f(info->resource_ptr,  res);
			}
			},
			[&](const HAL::Views::RenderTarget::Texture2DArray& Texture2DArray) {
					auto& desc = info->resource_ptr->get_desc().as_texture();

			for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, Texture2DArray.PlaneSlice);
				f(info->resource_ptr, res);
			}
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);
	}

	void for_each(const ResourceInfo* info, const HAL::Views::UnorderedAccess& view, std::function<void(HAL::Resource*, UINT)> f)
	{


		std::visit(overloaded{
			[&](const HAL::Views::UnorderedAccess::Buffer& Buffer) {
				f(info->resource_ptr, ALL_SUBRESOURCES);
			if (info->counter_resource) f(info->counter_resource, ALL_SUBRESOURCES);
			},
			[&](const HAL::Views::UnorderedAccess::Texture1D& Texture1D) {
	assert(false);
			},
			[&](const HAL::Views::UnorderedAccess::Texture1DArray& Texture1DArray) {
	assert(false);
			},
			[&](const HAL::Views::UnorderedAccess::Texture2D& Texture2D) {
					auto& desc = info->resource_ptr->get_desc().as_texture();

if (desc.MipLevels == 1 && desc.is2D())
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, Texture2D.PlaneSlice);
				f(info->resource_ptr,  res);
			}
			},
			[&](const HAL::Views::UnorderedAccess::Texture2DArray& Texture2DArray) {
					auto& desc = info->resource_ptr->get_desc().as_texture();

			if (desc.MipLevels == 1 && desc.is2D() && desc.ArraySize == 1)
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{

				for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, Texture2DArray.PlaneSlice);
					f(info->resource_ptr, res);
				}

			}
			},
			[&](const HAL::Views::UnorderedAccess::Texture3D& Texture3D) {
					auto& desc = info->resource_ptr->get_desc().as_texture();

				if (Texture3D.FirstWSlice == 0 && Texture3D.WSize == desc.Dimensions.z && desc.MipLevels == 1)
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture3D.MipSlice, 0, 0);
				f(info->resource_ptr,  res);
			}
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);



	}


	void for_each(const ResourceInfo* info, const HAL::Views::DepthStencil& view, std::function<void(HAL::Resource*, UINT)> f)
	{
		std::visit(overloaded{
		[&](const HAL::Views::DepthStencil::Texture1D& Texture1D) {
	assert(false);
		},
		[&](const HAL::Views::DepthStencil::Texture1DArray& Texture1DArray) {
	assert(false);
		},
		[&](const HAL::Views::DepthStencil::Texture2D& Texture2D) {
auto& desc = info->resource_ptr->get_desc().as_texture();


			UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, 0);
			f(info->resource_ptr, res);
		},
		[&](const HAL::Views::DepthStencil::Texture2DArray& Texture2DArray) {

				auto& desc = info->resource_ptr->get_desc().as_texture();

			for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, 0);
				f(info->resource_ptr, res);
			}
		},
		[&](const HAL::Views::DepthStencil::Texture2DMS& Texture2DMS) {
	assert(false);
		},
		[&](const HAL::Views::DepthStencil::Texture2DMSArray& Texture2DMSArray) {
	assert(false);
		},
		[&](auto other) {
			assert(false);
		}
			}, view.View);

	}


	void for_each(const ResourceInfo* info, const HAL::Views::ShaderResource& view, std::function<void(HAL::Resource*, UINT)> f)
	{
		std::visit(overloaded{
		[&](const HAL::Views::ShaderResource::Buffer& Buffer) {
			f(info->resource_ptr, ALL_SUBRESOURCES);
		},
		[&](const HAL::Views::ShaderResource::Texture1D& Texture1D) {
	assert(false);
		},
		[&](const HAL::Views::ShaderResource::Texture1DArray& Texture1DArray) {
	assert(false);
		},
		[&](const HAL::Views::ShaderResource::Texture2D& Texture2D) {
				auto& desc = info->resource_ptr->get_desc().as_texture();

			if (Texture2D.MipLevels == desc.MipLevels && Texture2D.MostDetailedMip == 0 && desc.is2D())
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture2D.MostDetailedMip; mip < Texture2D.MostDetailedMip + Texture2D.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, Texture2D.PlaneSlice);
					f(info->resource_ptr, res);
				}
			}
		},
		[&](const HAL::Views::ShaderResource::Texture2DArray& Texture2DArray) {
				auto& desc = info->resource_ptr->get_desc().as_texture();

			if (Texture2DArray.MipLevels == desc.MipLevels && Texture2DArray.MostDetailedMip == 0 && Texture2DArray.FirstArraySlice == 0 && Texture2DArray.ArraySize == desc.ArraySize && desc.is2D())
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture2DArray.MostDetailedMip; mip < Texture2DArray.MostDetailedMip + Texture2DArray.MipLevels; mip++)
					for (auto array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, Texture2DArray.PlaneSlice);
						f(info->resource_ptr, res);
					}
			}
		},
		[&](const HAL::Views::ShaderResource::Texture3D& Texture3D) {
				auto& desc = info->resource_ptr->get_desc().as_texture();

			if (Texture3D.MipLevels == desc.MipLevels && Texture3D.MostDetailedMip == 0)
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture3D.MostDetailedMip; mip < Texture3D.MostDetailedMip + Texture3D.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, 0);
					f(info->resource_ptr, res);
				}
			}
		},
		[&](const HAL::Views::ShaderResource::Texture2DMS& Texture2DMS) {
assert(false);
		},
		[&](const HAL::Views::ShaderResource::Texture2DMSArray& Texture2DMSArray) {
assert(false);
		},
		[&](const HAL::Views::ShaderResource::Cube& Cube) {
				auto& desc = info->resource_ptr->get_desc().as_texture();

			if (Cube.MipLevels == desc.MipLevels && Cube.MostDetailedMip == 0)
			{
				f(info->resource_ptr, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Cube.MostDetailedMip; mip < Cube.MostDetailedMip + Cube.MipLevels; mip++)
					for (auto array = 0; array < 6; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, 0);
						f(info->resource_ptr, res);
					}
			}
		},
		[&](const HAL::Views::ShaderResource::CubeArray& CubeArray) {
assert(false);
		},
		[&](const HAL::Views::ShaderResource::Raytracing& Raytracing) {
f(info->resource_ptr, ALL_SUBRESOURCES);
		},
		[&](auto other) {
			assert(false);
		}
			}, view.View);

	}


	void ResourceInfo::for_each_subres(std::function<void(HAL::Resource*, UINT)> f) const
	{

		std::visit(overloaded{
		[&](const HAL::Views::ShaderResource& ShaderResource) {
			for_each(this, ShaderResource, f);
		},
			[&](const HAL::Views::UnorderedAccess& UnorderedAccess) {
			for_each(this,UnorderedAccess, f);
		},
			[&](const HAL::Views::RenderTarget& RenderTarget) {
			for_each(this, RenderTarget, f);
		},
			[&](const HAL::Views::DepthStencil& DepthStencil) {
			for_each(this,DepthStencil, f);
		},
		[&](auto other) {
			assert(false);
		}
			}, view);
	}


}
