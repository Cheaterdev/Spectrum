module HAL:Texture;

import HAL;


namespace HAL
{

	void Texture::init()
	{
		if (native_resource)
		{

			auto desc = get_desc().as_texture();
			if (desc.ArraySize == 6)
				cube_view = CubeView(this, HAL::Device::get().get_static_gpu_data());

			/*		if (desc.ArraySize % 6 == 0)
						array_cubemap_view = std::make_shared<CubemapArrayView>(this);*/


						/*		if (desc.ArraySize > 1)
									array_2d_view = std::make_shared<Array2DView>(this);*/

			if (desc.is3D())
				texture_3d_view = Texture3DView(this, HAL::Device::get().get_static_gpu_data());
			else
				texture_2d_view = TextureView(this, HAL::Device::get().get_static_gpu_data());
		}
	}

	ivec3 Texture::get_size(int mip)
	{
		auto& desc = get_desc().as_texture();
		return uint3::max(uint3(1, 1, 1), desc.Dimensions / (1 << mip));
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

	Texture::Texture(D3D::Resource native, ResourceState state) : Resource(native, state)
	{
		//    resource.reset(new Resource(native));
		set_name("Texture");
		//	resource->debug = true;
		init();
	}
	Texture::Texture(HAL::ResourceDesc desc, ResourceState state /*= ResourceState::PIXEL_SHADER_RESOURCE*/, HeapType heap_type) :Resource(desc, heap_type,  state)
	{
		init();
	}

	texture_data::ptr Texture::get_data() const
	{
		auto desc = get_desc().as_texture();
		//auto list = Device::get().get_upload_list();

		auto list = (HAL::Device::get().get_queue(CommandListType::COPY)->get_free_list());
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


		Resource::_init(desc, HeapType::DEFAULT, (data.array_size * data.mip_maps) ? ResourceState::COMMON : ResourceState::PIXEL_SHADER_RESOURCE);
		auto list = (HAL::Device::get().get_upload_list());

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

		return create(data, HeapType::DEFAULT); //std::make_shared<Texture>(HAL::ResourceDesc{ desc, HAL::ResFlags::ShaderResource }, ResourceState::COMMON, HeapType::DEFAULT, data);
	}

	Texture::ptr Texture::load_native(const texure_header& header, resource_file_depender& depender)
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


			return create(tex_data, HeapType::DEFAULT);
		}

		return nullptr;
	}
	 Texture::ptr Texture::create(HAL::texture_data::ptr& tex_data, HeapType heap_type)
	{
		HAL::ResourceDesc desc;

		if (tex_data->depth > 1)
			desc = HAL::ResourceDesc::Tex3D(tex_data->format, { tex_data->width, tex_data->height, tex_data->depth }, tex_data->mip_maps);
		else
			desc = HAL::ResourceDesc::Tex2D(tex_data->format, { tex_data->width, tex_data->height }, tex_data->array_size, tex_data->mip_maps);
		auto texture = std::make_shared<Texture>(desc, ResourceState::COPY_DEST, HeapType::DEFAULT);


		texture->upload_data(tex_data);

		return texture;
	}


}