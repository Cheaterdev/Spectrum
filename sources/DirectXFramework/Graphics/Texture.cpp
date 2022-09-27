module;

#include <memory>

#include "helper.h"
module Graphics:Texture;
import :Queue;

import HAL;
using namespace HAL;

namespace Graphics
{

	void Texture::init()
	{
		if (tracked_info->m_Resource)
		{

			auto desc = get_desc().as_texture();
			if (desc.ArraySize == 6)
				cubemap_view = std::make_shared<CubemapView>(this);

			if (desc.ArraySize % 6 == 0)
				array_cubemap_view = std::make_shared<CubemapArrayView>(this);


			if (desc.ArraySize > 1)
				array_2d_view = std::make_shared<Array2DView>(this);

			if (desc.is3D())
				texture_3d_view = std::make_shared<Texture3DView>(this);
			else
				texture_2d_view = std::make_shared<Texture2DView>(this);
		}
	}

	ivec3 Texture::get_size(int mip)
	{
		auto& desc = get_desc().as_texture();
		return uint3::max(uint3(1, 1, 1), desc.Dimensions / (1 << mip));
	}

	CubemapView::ptr& Texture::cubemap()
	{
		if (!cubemap_view)
			cubemap_view = std::make_shared<CubemapView>(this);

		return cubemap_view;
	}

	Texture2DView::ptr& Texture::texture_2d()
	{
		return texture_2d_view;
	}

	Texture3DView::ptr& Texture::texture_3d()
	{
		return texture_3d_view;
	}

	bool Texture::is_rt()
	{
		return check(get_desc().Flags & HAL::ResFlags::RenderTarget);
	}




	const Texture::ptr Texture::null(new Texture(nullptr));

	texure_header::texure_header(std::filesystem::path name, bool force_srgb /*= false*/, bool mips /*= true*/)
	{
		this->name = name;
		this->force_srgb = force_srgb;
		this->mips = mips;
	}


	Texture::Texture()
	{
	}
	Texture::Texture(void*)
	{
		init();
	}

	Texture::Texture(HAL::Resource::ptr native, HandleTable rtv, ResourceState state) : Resource(native, state)
	{
		//  resource.reset(new Resource(native));
		set_name("Texture");
		//this->rtv = rtv;
		//resource->debug = true;
		texture_2d_view = std::make_shared<Texture2DView>(this, rtv);
		//   init();
	}
	Texture::Texture(HAL::Resource::ptr native, ResourceState state) : Resource(native, state)
	{
		//    resource.reset(new Resource(native));
		set_name("Texture");
		//	resource->debug = true;
		init();
	}
	Texture::Texture(HAL::ResourceDesc desc, ResourceState state /*= ResourceState::PIXEL_SHADER_RESOURCE*/, HeapType heap_type, std::shared_ptr<texture_data> data /*= nullptr*/) :Resource(desc, heap_type, data ? ResourceState::COPY_DEST : state)
	{
		//set_name(std::string("Texture_") + std::to_string(desc.Width) + "_" + std::to_string(desc.Height) + "_" + std::to_string(desc.DepthOrArraySize));
		//desc = get_desc();
		if (data)
			upload_data(data);
		init();
	}

	texture_data::ptr Texture::get_data() const
	{
		auto desc = get_desc().as_texture();
		//auto list = Device::get().get_upload_list();

		auto list = Device::get().get_queue(CommandListType::COPY)->get_free_list();
		list->begin("Texture Readback");

		desc.Format = desc.Format.to_typeless();

		auto dims = uint3::max(desc.Dimensions, uint3{ 1,1,1 });
		//  auto res = list->read_texture(resource, ivec3(0, 0, 0), ivec3(desc.Width, desc.Height, 1), i);
		texture_data::ptr p_data(new texture_data(desc.ArraySize, desc.MipLevels, dims.x, dims.y, dims.z, desc.Format));
		auto& data = *p_data;
		std::vector<std::future<bool>> tasks;

		for (unsigned int a = 0; a < desc.ArraySize; a++)
			for (unsigned int m = 0; m < desc.MipLevels; m++)
			{
				int i = D3D12CalcSubresource(m, a, 0, desc.MipLevels, desc.ArraySize);
				tasks.emplace_back(list->get_copy().read_texture(this, ivec3(0, 0, 0), { data.array[a]->mips[m]->width, data.array[a]->mips[m]->height, data.array[a]->mips[m]->depth }, i, [desc, &data, a, m](const char* mem, UINT64 pitch, UINT64 slice, UINT64 size)
					{
						auto c = desc;

						for (unsigned int z = 0; z < data.array[a]->mips[m]->depth; z++)
						{
							for (unsigned int w = 0; w < data.array[a]->mips[m]->num_rows; w++)
								memcpy(data.array[a]->mips[m]->data.data() + w * data.array[a]->mips[m]->width_stride + z * data.array[a]->mips[m]->slice_stride
									, mem + w * pitch + z * slice,
									data.array[a]->mips[m]->width_stride);
						}
					}));
			}

		//	list->transition(resource, ResourceState::COPY_SOURCE, type, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		list->end();
		list->execute();

		for (auto&& t : tasks)
			t.wait();

		return p_data;
	}


	void Texture::upload_data(texture_data::ptr dat)
	{

		auto& data = *dat;
		data.format = data.format.to_typeless();
		HAL::ResourceDesc desc;

		if (data.depth > 1)
			desc = HAL::ResourceDesc::Tex3D(data.format, { data.width, data.height, data.depth }, data.mip_maps);
		else
			desc = HAL::ResourceDesc::Tex2D(data.format, { data.width, data.height }, data.array_size, data.mip_maps);


		Resource::init(desc, HeapType::DEFAULT, (data.array_size * data.mip_maps) ? ResourceState::COMMON : ResourceState::PIXEL_SHADER_RESOURCE);
		auto list = Device::get().get_upload_list();

		if (data.array_size * data.mip_maps)
		{

			for (unsigned int a = 0; a < data.array_size; a++)
				for (unsigned int m = 0; m < data.mip_maps; m++)
				{
					int i = m * data.array_size + a;
					list->get_copy().update_texture(this, { 0, 0, 0 }, { data.array[a]->mips[m]->width, data.array[a]->mips[m]->height, data.array[a]->mips[m]->depth }, i, (const char*)data.array[a]->mips[m]->data.data(), data.array[a]->mips[m]->width_stride, data.array[a]->mips[m]->slice_stride);
				}
		}

		list->end();
		list->execute_and_wait();
		init();
	}
	Texture::ptr Texture::compress()
	{
		auto data = texture_data::compress(get_data());

		if (!data) return nullptr;

		auto desc = get_desc().as_texture();
		desc.Format = data->format;
		desc.MipLevels = data->mip_maps;
		desc.Dimensions = uint3(data->width, data->height, 0);

		return std::make_shared<Texture>(HAL::ResourceDesc{ desc, HAL::ResFlags::ShaderResource }, ResourceState::COMMON, HeapType::DEFAULT, data);
	}
	Array2DView::ptr& Texture::array2d()
	{
		return array_2d_view;
	}
	Graphics::Texture::ptr Texture::load_native(const texure_header& header, resource_file_depender& depender)
	{
		auto file = FileSystem::get().get_file(header.name);

		if (file)
		{
			depender.add_depend(file);
			int flags = 0;

			if (header.force_srgb) flags |= texture_data::LoadFlags::MAKE_LINEAR;

			if (header.mips) flags |= texture_data::LoadFlags::GENERATE_MIPS;

			auto tex_data = texture_data::load_texture(file, flags);

			if (!tex_data)
				return nullptr;

			HAL::ResourceDesc desc;

			if (tex_data->depth > 1)
				desc = HAL::ResourceDesc::Tex3D(tex_data->format, { tex_data->width, tex_data->height, tex_data->depth }, tex_data->mip_maps);
			else
				desc = HAL::ResourceDesc::Tex2D(tex_data->format, { tex_data->width, tex_data->height }, tex_data->array_size, tex_data->mip_maps);

			return std::make_shared<Texture>(desc, ResourceState::PIXEL_SHADER_RESOURCE, HeapType::DEFAULT, tex_data);
		}

		return nullptr;
	}
	CubemapArrayView::CubemapArrayView(Resource* _resource) : View(_resource)
	{
		auto res_desc = resource->get_desc();

		uint count = res_desc.as_texture().ArraySize / 6;


		for (uint i = 0; i < count; i++)
			views.emplace_back(new CubemapView(resource, i));

		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource))
		{
			srvs = StaticDescriptors::get().place(1 + count);
			place_srv(srvs[0]);

			for (uint i = 0; i < count; i++)
				place_srv(srvs[1 + i], i);

		}
	}

	void CubemapArrayView::place_srv(Handle  h)
	{
		uint count = resource->get_desc().as_texture().ArraySize / 6;
		auto b = HAL::Views::ShaderResource::CubeArray{ 0, 0, 0, count, 0 };
		h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
	}

	void CubemapArrayView::place_srv(Handle h, UINT offset)
	{
		auto b = HAL::Views::ShaderResource::CubeArray{ 0, 0, offset * 6, 1, 0 };
		h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
	}

	CubemapView::CubemapView(Resource* _resource, int offset) : View(_resource)
	{
		auto res_desc = resource->get_desc();
		single_count = 6 * res_desc.as_texture().MipLevels;

		if (check(res_desc.Flags & HAL::ResFlags::RenderTarget))
		{

			rtvs = DescriptorHeapManager::get().get_rt()->create_table(single_count);

			for (uint m = 0; m < res_desc.as_texture().MipLevels; m++)
			{
				for (uint i = 0; i < 6u; i++)
				{

					auto b = HAL::Views::RenderTarget::Texture2DArray{ m, 0, offset * 6 + i, 1 };
					rtvs[i + 6 * m] = HAL::Views::RenderTarget{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
				}
			}
		}

		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource))
		{
			srvs = StaticDescriptors::get().place(1 + res_desc.as_texture().MipLevels);
			place_srv(srvs[0]);

			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
				place_srv(srvs[1 + i], i);

			static_srv = StaticDescriptors::get().place(1);
			place_srv(static_srv[0]);
		}

		p.resize(res_desc.as_texture().MipLevels);
		scissor.resize(res_desc.as_texture().MipLevels);

		for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
		{
			int mm = 1 << i;
			p[i].Width = std::max(1.0f, static_cast<float>(resource->get_desc().as_texture().Dimensions.x / mm));
			p[i].Height = std::max(1.0f, static_cast<float>(resource->get_desc().as_texture().Dimensions.y / mm));
			p[i].TopLeftX = 0;
			p[i].TopLeftY = 0;
			p[i].MinDepth = 0;
			p[i].MaxDepth = 1;
			scissor[i] = { 0, 0, p[i].Width , p[i].Height };
		}

		for (uint i = 0; i < 6u; i++)
		{
			faces[i] = std::make_shared<Texture2DView>(_resource, i + offset * 6);
		}
	}

	Handle CubemapView::get_rtv(UINT index, UINT mip)
	{
		return rtvs[index + 6 * mip];
	}

	Handle CubemapView::get_srv(UINT mip)
	{
		return srvs[1 + mip];
	}

	Handle CubemapView::get_srv()
	{
		return srvs[0];
	}

	void CubemapView::place_srv(Handle  h)
	{
		auto b = HAL::Views::ShaderResource::Cube{ 0, resource->get_desc().as_texture().MipLevels, 0 };
		h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
	}

	void CubemapView::place_srv(Handle  h, UINT mip)
	{
		auto b = HAL::Views::ShaderResource::Cube{ 0, resource->get_desc().as_texture().MipLevels, 0 };
		h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
	}

	Viewport CubemapView::get_viewport(UINT mip)
	{
		return p[mip];
	}

	sizer_long CubemapView::get_scissor(UINT mip)
	{
		return scissor[mip];
	}

	Handle CubemapView::get_static_srv()
	{
		return static_srv[0];
	}

	Texture2DView::Texture2DView(Resource* _resource, HandleTable t) : View(_resource)
	{
		rtvs = t;
		auto res_desc = resource->get_desc();
		p.resize(res_desc.as_texture().MipLevels);
		scissor.resize(res_desc.as_texture().MipLevels);

		for (UINT i = 0; i < res_desc.as_texture().MipLevels; i++)
		{
			UINT mm = 1 << i;
			p[i].Width = std::max(1.0f, static_cast<float>(resource->get_desc().as_texture().Dimensions.x / mm));
			p[i].Height = std::max(1.0f, static_cast<float>(resource->get_desc().as_texture().Dimensions.y / mm));
			p[i].TopLeftX = 0;
			p[i].TopLeftY = 0;
			p[i].MinDepth = 0;
			p[i].MaxDepth = 1;
			scissor[i] = { 0, 0, p[i].Width , p[i].Height };
		}
	}
	Texture2DView::Texture2DView(Resource* _resource, int array_index) : View(_resource), array_index(array_index)
	{
		auto res_desc = resource->get_desc();
		//    single_count = 6 * res_desc.MipLevels;
		//      rtvs = DescriptorHeapManager::get().get_rt()->create_table(single_count);
		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource)) {
			srvs = StaticDescriptors::get().place(1 + res_desc.as_texture().MipLevels);
			place_srv(srvs[0]);
			static_srv = StaticDescriptors::get().place(1);
			place_srv(static_srv[0]);

			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
				place_srv(srvs[i + 1], i);

		}
		if (check(res_desc.Flags & HAL::ResFlags::RenderTarget))
		{
			rtvs = DescriptorHeapManager::get().get_rt()->create_table(res_desc.as_texture().MipLevels);

			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
				place_rtv(rtvs[i], i);
		}

		if (check(res_desc.Flags & HAL::ResFlags::UnorderedAccess))
		{
			uavs = StaticDescriptors::get().place(resource->get_desc().as_texture().MipLevels);

			for (uint i = 0; i < resource->get_desc().as_texture().MipLevels; i++)
				place_uav(uavs[i], i, 0);

			static_uav = StaticDescriptors::get().place(1);
			place_uav(static_uav[0]);
		}

		p.resize(res_desc.as_texture().MipLevels);
		scissor.resize(res_desc.as_texture().MipLevels);

		for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
		{
			int mm = 1 << i;
			p[i].Width = (float)std::max(1u, resource->get_desc().as_texture().Dimensions.x / mm);
			p[i].Height = (float)std::max(1u, resource->get_desc().as_texture().Dimensions.y / mm);
			p[i].TopLeftX = 0;
			p[i].TopLeftY = 0;
			p[i].MinDepth = 0;
			p[i].MaxDepth = 1;
			scissor[i] = { 0, 0, p[i].Width , p[i].Height };
		}

		/*  for (int m = 0; m < res_desc.MipLevels; m++)
		  {
			  for (int i = 0; i < 6; i++)
			  {
				  D3D12_RENDER_TARGET_VIEW_DESC desc;
				  desc.Format = to_srv(res_desc.Format);
				  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				  desc.Texture2DArray.PlaneSlice = 0;
				  desc.Texture2DArray.MipSlice = m;
				  desc.Texture2DArray.ArraySize = 1;
				  desc.Texture2DArray.FirstArraySlice = i;
				  Device::get().get_native_device()->CreateRenderTargetView(resource->get_native().Get(), &desc, rtvs[i + 6 * m].cpu);
			  }
		  }*/



		hlsl = StaticDescriptors::get().place(1 + 2 * res_desc.as_texture().MipLevels);
		int offset = 0;
		texture2D = HLSL::Texture2D<>(hlsl[offset++]);

		place_srv(texture2D);

		if (check(res_desc.Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwTexture2D.resize(res_desc.as_texture().MipLevels);
			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
			{
				rwTexture2D[i] = HLSL::RWTexture2D<>(hlsl[offset++]);
				place_uav(rwTexture2D[i], i, 0);
			}


		}

		texture2DMips.resize(res_desc.as_texture().MipLevels);
		for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
		{
			texture2DMips[i] = HLSL::Texture2D<>(hlsl[offset++]);
			place_srv(texture2DMips[i], i);
		}
	}
	Handle Texture2DView::get_rtv(UINT mip)
	{
		return rtvs[mip];
	}
	void Texture2DView::place_srv(Handle  h)
	{
		srv()(h);
	}
	void Texture2DView::place_srv(Handle  h, UINT mip)
	{
		srv(mip)(h);
	}
	void Texture2DView::place_rtv(Handle  h, int i)
	{
		rtv(i)(h);
	}
	void Texture2DView::place_dsv(Handle  h, UINT mip)
	{
		if (array_index == -1)
		{
			auto b = HAL::Views::DepthStencil::Texture2D{ mip };
			h = HAL::Views::DepthStencil{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_dsv(), HAL::Views::DepthStencil::Flags::None, b };
		}
		else
		{
			auto b = HAL::Views::DepthStencil::Texture2DArray{ mip,uint(array_index),1 };
			h = HAL::Views::DepthStencil{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_dsv(),HAL::Views::DepthStencil::Flags::None, b };
		}
	}

	void Texture2DView::place_uav(Handle  h, UINT mip, UINT slice)
	{
		uav(mip, slice)(h);
	}
	std::function<void(Handle)> Texture2DView::uav(UINT mip, UINT slice)
	{
		return [this, mip, slice](Handle  h)
		{
			if (!resource) return;

			if (array_index == -1)
			{
				auto b = HAL::Views::UnorderedAccess::Texture2D{ mip,slice };
				h = HAL::Views::UnorderedAccess{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}
			else
			{
				auto b = HAL::Views::UnorderedAccess::Texture2DArray{ mip,uint(array_index), 1, slice };
				h = HAL::Views::UnorderedAccess{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}

		};
	}
	std::function<void(Handle)> Texture2DView::srv(PixelSpace space)
	{
		return [this, space](Handle h)
		{
			if (!resource) return;

			//	if (space == PixelSpace::MAKE_LINERAR)
			//		srvDesc.Format = to_linear(srvDesc.Format);
			if (array_index == -1)
			{

				auto b = HAL::Views::ShaderResource::Texture2D{ 0, resource->get_desc().as_texture().MipLevels, 0 };
				h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}
			else
			{

				auto b = HAL::Views::ShaderResource::Texture2DArray{ 0, resource->get_desc().as_texture().MipLevels, uint(array_index),1,0 };
				h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}
		};
	}
	std::function<void(Handle)> Texture2DView::srv(UINT mip, UINT levels)
	{
		return [this, mip, levels](Handle  h)
		{

			if (array_index == -1)
			{

				auto b = HAL::Views::ShaderResource::Texture2D{ mip, levels, 0 };
				h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}
			else
			{

				auto b = HAL::Views::ShaderResource::Texture2DArray{ mip, levels, uint(array_index),1,0 };
				h = HAL::Views::ShaderResource{ resource->get_hal().get(), resource->get_desc().as_texture().Format.to_srv(), b };
			}

		};
	}
	std::function<void(Handle)> Texture2DView::rtv(UINT mip)
	{
		return [this, mip](Handle  h)
		{if (array_index == -1)
		{
			h = HAL::Views::RenderTarget{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::RenderTarget::Texture2D
				{
					.MipSlice = mip,
					.PlaneSlice = 0
				}
			};
		}
		else
		{

			h = HAL::Views::RenderTarget{
		.Resource = resource->get_hal().get(),
		.Format = resource->get_desc().as_texture().Format.to_srv(),
		.View = HAL::Views::RenderTarget::Texture2DArray
			{
				.MipSlice = mip,
				.FirstArraySlice = uint(array_index),
				.ArraySize = 1,
				.PlaneSlice = 0
			}
			};


		}
		};
	}
	Handle Texture2DView::get_srv(UINT mip)
	{
		return srvs[mip + 1];
	}
	Handle Texture2DView::get_srv()
	{
		return srvs[0];
	}
	Handle Texture2DView::get_static_srv()
	{
		return static_srv[0];
	}
	Handle Texture2DView::get_static_uav()
	{
		return static_uav[0];
	}
	Handle Texture2DView::get_uav(UINT mip)
	{
		return uavs[mip];
	}
	Viewport Texture2DView::get_viewport(UINT mip)
	{
		return p[mip];
	}
	sizer_long Texture2DView::get_scissor(UINT mip)
	{
		return scissor[mip];
	}

	Handle Texture3DView::get_static_srv()
	{
		return static_srv[0];
	}

	Texture3DView::Texture3DView(Resource* _resource) : View(_resource)
	{
		srvs = StaticDescriptors::get().place(1 + resource->get_desc().as_texture().MipLevels);
		place_srv(srvs[0]);

		for (uint i = 0; i < resource->get_desc().as_texture().MipLevels; i++)
			place_srv(srvs[1 + i], i);

		if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess))
		{
			uavs = StaticDescriptors::get().place(resource->get_desc().as_texture().MipLevels);
			static_uav = StaticDescriptors::get().place(resource->get_desc().as_texture().MipLevels);

			//		for (int i = 0; i < resource->get_desc().MipLevels; i++)
			for (uint i = 0; i < resource->get_desc().as_texture().MipLevels; i++)
			{
				uavs[i] = uav(i);//place_uav(uavs[i], i);
				static_uav[i] = uav(i);
			}
		}


		static_srv = StaticDescriptors::get().place(1);
		place_srv(static_srv[0]);



		hlsl = StaticDescriptors::get().place(1 + 2 * resource->get_desc().as_texture().MipLevels);

		int offset = 0;
		texture3D = HLSL::Texture3D<float4>(hlsl[offset++]);

		place_srv(texture3D);


		texture3DMips.resize(resource->get_desc().as_texture().MipLevels);
		for (int i = 0; i < texture3DMips.size(); i++)
		{
			texture3DMips[i] = HLSL::Texture3D<>(hlsl[offset++]);
			srv(i, 1)(texture3DMips[i]);
		}

		if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwTexture3D.resize(resource->get_desc().as_texture().MipLevels);
			for (uint i = 0; i < resource->get_desc().as_texture().MipLevels; i++)
			{
				rwTexture3D[i] = HLSL::RWTexture3D<>(hlsl[offset++]);
				place_uav(rwTexture3D[i], i);
			}

		}



		/*   auto res_desc = resource->get_desc();
		   single_count = 6 * res_desc.MipLevels;
		   rtvs = DescriptorHeapManager::get().get_rt()->create_table(single_count);

		   for (int m = 0; m < res_desc.MipLevels; m++)
		   {
			   for (int i = 0; i < 6; i++)
			   {
				   D3D12_RENDER_TARGET_VIEW_DESC desc;
				   desc.Format = to_srv(res_desc.Format);
				   desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				   desc.Texture2DArray.PlaneSlice = 0;
				   desc.Texture2DArray.MipSlice = m;
				   desc.Texture2DArray.ArraySize = 1;
				   desc.Texture2DArray.FirstArraySlice = i;
				   Device::get().get_native_device()->CreateRenderTargetView(resource->get_native().Get(), &desc, rtvs[i + 6 * m].cpu);
			   }
		   }*/
	}
	Handle Texture3DView::get_rtv(UINT index, UINT mip)
	{
		return rtvs[index + 6 * mip];
	}
	void Texture3DView::place_srv(Handle  h, int level)
	{
		srv(level)(h);
	}
	void Texture3DView::place_uav(Handle  h, int level)
	{
		uav(level)(h);
	}
	Handle Texture3DView::get_srv(int i)
	{
		return srvs[i + 1];
	}

	Handle Texture3DView::get_srv()
	{
		return srvs[0];
	}

	Handle Texture3DView::get_uav(int i)
	{
		return uavs[i];
	}
	Handle Texture3DView::get_static_uav()
	{
		return static_uav[0];
	}
	std::function<void(Handle)> Texture3DView::srv(int level, int levels)
	{
		return [this, level, levels](Handle  h)
		{
			h = HAL::Views::ShaderResource{
				.Resource = resource->get_hal().get(),
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::ShaderResource::Texture3D
						{
							.MostDetailedMip = uint(level == -1 ? 0 : level),
							.MipLevels = (levels == -1) ? (level == -1 ? resource->get_desc().as_texture().MipLevels : resource->get_desc().as_texture().MipLevels - level) : (levels),
							.ResourceMinLODClamp = 0
						}
			};
		};
	}
	std::function<void(Handle)> Texture3DView::uav(int level)
	{
		return [this, level](Handle  h)
		{
			h = HAL::Views::UnorderedAccess{
				.Resource = resource->get_hal().get(),
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::UnorderedAccess::Texture3D
						{
							.MipSlice = uint(level),
							.FirstWSlice = 0,
							.WSize = uint(resource->get_desc().as_texture().Dimensions.z / (1 << level))
						}
			};
		};
	}
	Array2DView::Array2DView(Resource* _resource) : View(_resource)
	{
		if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess))
		{
			uavs = StaticDescriptors::get().place(1);
			//	for (int i = 0; i < resource->get_desc().MipLevels; i++)
			place_uav(uavs[0], 0, resource->get_desc().as_texture().ArraySize, 0, 0);
			static_uav = StaticDescriptors::get().place(1);
			//	for (int i = 0; i < resource->get_desc().MipLevels; i++)
			place_uav(static_uav[0], 0, resource->get_desc().as_texture().ArraySize, 0, 0);
		}

		srvs = StaticDescriptors::get().place(1);
		place_srv(srvs[0]);
		static_srv = StaticDescriptors::get().place(1);
		place_srv(static_srv[0]);
		/*  auto res_desc = resource->get_desc();
		  single_count = 6 * res_desc.MipLevels;
		  rtvs = DescriptorHeapManager::get().get_rt()->create_table(single_count);

		  for (int m = 0; m < res_desc.MipLevels; m++)
		  {
			  for (int i = 0; i < 6; i++)
			  {
				  D3D12_RENDER_TARGET_VIEW_DESC desc;
				  desc.Format = to_srv(res_desc.Format);
				  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				  desc.Texture2DArray.PlaneSlice = 0;
				  desc.Texture2DArray.MipSlice = m;
				  desc.Texture2DArray.ArraySize = 1;
				  desc.Texture2DArray.FirstArraySlice = i;
				  Device::get().get_native_device()->CreateRenderTargetView(resource->get_native().Get(), &desc, rtvs[i + 6 * m].cpu);
			  }
		  }*/
	}

	Handle Array2DView::get_rtv(UINT index, UINT mip)
	{
		return rtvs[index + 6 * mip];
	}

	void Array2DView::place_srv(Handle  h)
	{
		h = HAL::Views::ShaderResource{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::ShaderResource::Texture2DArray
					{
						.MostDetailedMip = 0,
						.MipLevels = resource->get_desc().as_texture().MipLevels,
						.FirstArraySlice = 0,
						.ArraySize = resource->get_desc().as_texture().ArraySize,
						.PlaneSlice = 0,
						.ResourceMinLODClamp = 0
					}
		};
	}

	void Array2DView::place_dsv(Handle  h, UINT mip, UINT slice, UINT slice_count)
	{
		h = HAL::Views::DepthStencil{
		.Resource = resource->get_hal().get(),
		.Format = resource->get_desc().as_texture().Format.to_srv(),
		.View = HAL::Views::DepthStencil::Texture2DArray
			{
				.MipSlice = mip,
				.FirstArraySlice = slice,
				.ArraySize = slice_count,
			}
		};
	}

	void Array2DView::place_uav(Handle  h, UINT first, UINT count, UINT mip, UINT slice)
	{
		h = HAL::Views::UnorderedAccess{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::UnorderedAccess::Texture2DArray
					{
						.MipSlice = mip,
						.FirstArraySlice = first,
						.ArraySize = count,
						.PlaneSlice = slice
					}
		};
	}

	std::function<void(Handle)> Array2DView::uav(UINT first, UINT count, UINT mip)
	{
		return [this, first, count, mip](Handle h)
		{
			if (!resource) return;
			h = HAL::Views::UnorderedAccess{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::UnorderedAccess::Texture2DArray
			{
				.MipSlice = mip,
				.FirstArraySlice = first,
				.ArraySize = count,
				.PlaneSlice = 0
			}
			};

		};
	}

	Handle Array2DView::get_srv()
	{
		return srvs[0];
	}

	Handle Array2DView::get_uav()
	{
		return uavs[0];
	}

	Handle Array2DView::get_static_srv()
	{
		return static_srv[0];
	}

	Handle Array2DView::get_static_uav()
	{
		return static_uav[0];
	}

}