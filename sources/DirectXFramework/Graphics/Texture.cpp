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
			/*srvs = StaticDescriptors::get().place(1 + res_desc.as_texture().MipLevels);
			place_srv(srvs[0]);

			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
				place_srv(srvs[1 + i], i);

			static_srv = StaticDescriptors::get().place(1);
			place_srv(static_srv[0]);*/
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

	Viewport CubemapView::get_viewport(UINT mip)
	{
		return p[mip];
	}

	sizer_long CubemapView::get_scissor(UINT mip)
	{
		return scissor[mip];
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

		hlsl = StaticDescriptors::get().place(1 + 2 * res_desc.as_texture().MipLevels);
		int offset = 0;

		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource)) {

			texture2D = HLSL::Texture2D<>(hlsl[offset++]);
			texture2D.create(resource, 0, res_desc.as_texture().MipLevels, array_index);

			texture2DMips.resize(res_desc.as_texture().MipLevels);
			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
			{
				texture2DMips[i] = HLSL::Texture2D<>(hlsl[offset++]);
				texture2DMips[i].create(resource, i, 1, array_index);
			}
		}

		if (check(res_desc.Flags & HAL::ResFlags::RenderTarget))
		{
			rtvs = DescriptorHeapManager::get().get_rt()->create_table(res_desc.as_texture().MipLevels);

			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
				place_rtv(rtvs[i], i);
		}

		if (check(res_desc.Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwTexture2D.resize(res_desc.as_texture().MipLevels);
			for (uint i = 0; i < res_desc.as_texture().MipLevels; i++)
			{
				rwTexture2D[i] = HLSL::RWTexture2D<>(hlsl[offset++]);
				rwTexture2D[i].create(resource, i, array_index);
			}
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
	}

	Handle Texture2DView::get_rtv(UINT mip)
	{
		return rtvs[mip];
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

	Viewport Texture2DView::get_viewport(UINT mip)
	{
		return p[mip];
	}
	sizer_long Texture2DView::get_scissor(UINT mip)
	{
		return scissor[mip];
	}

	Texture3DView::Texture3DView(Resource* _resource) : View(_resource)
	{

		auto res_desc = resource->get_desc();

		hlsl = StaticDescriptors::get().place(1 + 2 * res_desc.as_texture().MipLevels);
		int offset = 0;

		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource)) {

			texture3D = HLSL::Texture3D<>(hlsl[offset++]);
			texture3D.create(resource, 0, res_desc.as_texture().MipLevels);


			texture3DMips.resize(resource->get_desc().as_texture().MipLevels);
			for (int i = 0; i < texture3DMips.size(); i++)
			{
				texture3DMips[i] = HLSL::Texture3D<>(hlsl[offset++]);
				texture3DMips[i].create(resource, i, 1);
			}
		}

		if (check(res_desc.Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwTexture3D.resize(resource->get_desc().as_texture().MipLevels);
			for (uint i = 0; i < resource->get_desc().as_texture().MipLevels; i++)
			{
				rwTexture3D[i] = HLSL::RWTexture3D<>(hlsl[offset++]);
				rwTexture3D[i].create(resource, i);
			}
		}

	}


	Array2DView::Array2DView(Resource* _resource) : View(_resource)
	{
		auto res_desc = resource->get_desc();

		hlsl = StaticDescriptors::get().place(1 + 2 * res_desc.as_texture().MipLevels);
		int offset = 0;

		if (check(res_desc.Flags & HAL::ResFlags::ShaderResource)) {

			texture2DArray = HLSL::Texture2DArray<>(hlsl[offset++]);
			texture2DArray.create(resource, 0, res_desc.as_texture().MipLevels, 0, res_desc.as_texture().ArraySize);

		}

		if (check(res_desc.Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwTexture2DArray = HLSL::RWTexture2DArray<>(hlsl[offset++]);
			rwTexture2DArray.create(resource, 0, res_desc.as_texture().MipLevels, 0, res_desc.as_texture().ArraySize);
		}
	}


}