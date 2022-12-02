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
	ResourceInfo* Handle::get_resource_info() const
	{
		return storage->get_heap()->get_resource_info(get_offset());
	}
	Handle::operator bool() const
	{
		return is_valid();
	}
	DescriptorHeap::DescriptorHeap(Device & device, UINT num, DescriptorHeapType type, HAL::DescriptorHeapFlags flags) :API::DescriptorHeap(device, HAL::DescriptorHeapDesc{ num, type , flags })
	{

		descriptor_size = device.get_descriptor_size(type);
		//	assert(m_GPUHeap);

		resources.resize(num);
	}


	void Handle::place(const Handle& r) const
	{

		auto& my_heap = *storage->get_heap();
		auto& other_heap = *r.storage->get_heap();

		my_heap[get_offset()] = other_heap[r.get_offset()];
		*get_resource_info() = *r.get_resource_info();
	}

	void for_each(const ResourceInfo* info, const HAL::Views::RenderTarget& view, std::function<void(HAL::Resource*, UINT)> f)
	{

		std::visit(overloaded{
			[&](const HAL::Views::RenderTarget::Texture2D& Texture2D) {
				auto& desc = view.Resource->get_desc().as_texture();

				if (desc.MipLevels == 1 && desc.is2D() == 1)
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, Texture2D.PlaneSlice);
				f(view.Resource,  res);
			}
			},
			[&](const HAL::Views::RenderTarget::Texture2DArray& Texture2DArray) {
					auto& desc = view.Resource->get_desc().as_texture();

			for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, Texture2DArray.PlaneSlice);
				f(view.Resource, res);
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
				f(view.Resource, ALL_SUBRESOURCES);
			if (Buffer.CounterResource) f(Buffer.CounterResource, ALL_SUBRESOURCES);
			},
			[&](const HAL::Views::UnorderedAccess::Texture1D& Texture1D) {
	assert(false);
			},
			[&](const HAL::Views::UnorderedAccess::Texture1DArray& Texture1DArray) {
	assert(false);
			},
			[&](const HAL::Views::UnorderedAccess::Texture2D& Texture2D) {
					auto& desc = view.Resource->get_desc().as_texture();

if (desc.MipLevels == 1 && desc.is2D())
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, Texture2D.PlaneSlice);
				f(view.Resource,  res);
			}
			},
			[&](const HAL::Views::UnorderedAccess::Texture2DArray& Texture2DArray) {
					auto& desc = view.Resource->get_desc().as_texture();

			if (desc.MipLevels == 1 && desc.is2D() && desc.ArraySize == 1)
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{

				for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, Texture2DArray.PlaneSlice);
					f(view.Resource, res);
				}

			}
			},
			[&](const HAL::Views::UnorderedAccess::Texture3D& Texture3D) {
					auto& desc = view.Resource->get_desc().as_texture();

				if (Texture3D.FirstWSlice == 0 && Texture3D.WSize == desc.Dimensions.z && desc.MipLevels == 1)
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(Texture3D.MipSlice, 0, 0);
				f(view.Resource,  res);
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
auto& desc = view.Resource->get_desc().as_texture();


			UINT res = desc.CalcSubresource(Texture2D.MipSlice, 0, 0);
			f(view.Resource, res);
		},
		[&](const HAL::Views::DepthStencil::Texture2DArray& Texture2DArray) {

				auto& desc = view.Resource->get_desc().as_texture();

			for (UINT array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(Texture2DArray.MipSlice, array, 0);
				f(view.Resource, res);
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
			f(view.Resource, ALL_SUBRESOURCES);
		},
		[&](const HAL::Views::ShaderResource::Texture1D& Texture1D) {
	assert(false);
		},
		[&](const HAL::Views::ShaderResource::Texture1DArray& Texture1DArray) {
	assert(false);
		},
		[&](const HAL::Views::ShaderResource::Texture2D& Texture2D) {
				auto& desc = view.Resource->get_desc().as_texture();

			if (Texture2D.MipLevels == desc.MipLevels && Texture2D.MostDetailedMip == 0 && desc.is2D())
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture2D.MostDetailedMip; mip < Texture2D.MostDetailedMip + Texture2D.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, Texture2D.PlaneSlice);
					f(view.Resource, res);
				}
			}
		},
		[&](const HAL::Views::ShaderResource::Texture2DArray& Texture2DArray) {
				auto& desc = view.Resource->get_desc().as_texture();

			if (Texture2DArray.MipLevels == desc.MipLevels && Texture2DArray.MostDetailedMip == 0 && Texture2DArray.FirstArraySlice == 0 && Texture2DArray.ArraySize == desc.ArraySize && desc.is2D())
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture2DArray.MostDetailedMip; mip < Texture2DArray.MostDetailedMip + Texture2DArray.MipLevels; mip++)
					for (auto array = Texture2DArray.FirstArraySlice; array < Texture2DArray.FirstArraySlice + Texture2DArray.ArraySize; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, Texture2DArray.PlaneSlice);
						f(view.Resource, res);
					}
			}
		},
		[&](const HAL::Views::ShaderResource::Texture3D& Texture3D) {
				auto& desc = view.Resource->get_desc().as_texture();

			if (Texture3D.MipLevels == desc.MipLevels && Texture3D.MostDetailedMip == 0)
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Texture3D.MostDetailedMip; mip < Texture3D.MostDetailedMip + Texture3D.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, 0);
					f(view.Resource, res);
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
				auto& desc = view.Resource->get_desc().as_texture();

			if (Cube.MipLevels == desc.MipLevels && Cube.MostDetailedMip == 0)
			{
				f(view.Resource, ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = Cube.MostDetailedMip; mip < Cube.MostDetailedMip + Cube.MipLevels; mip++)
					for (auto array = 0; array < 6; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, 0);
						f(view.Resource, res);
					}
			}
		},
		[&](const HAL::Views::ShaderResource::CubeArray& CubeArray) {
assert(false);
		},
		[&](const HAL::Views::ShaderResource::Raytracing& Raytracing) {
f(view.Resource, ALL_SUBRESOURCES);
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
