#include "pch.h"
#include "Texture.h"
namespace DX12
{

    void Texture::init()
    {
        if (tracked_info->m_Resource)
        {
            if (get_desc().ArraySize() == 6)
                cubemap_view = std::make_shared<CubemapView>(this);

			if (get_desc().ArraySize()%6==0)
				 array_cubemap_view = std::make_shared<CubemapArrayView>(this);
	

            if (get_desc().ArraySize() > 1)
                array_2d_view = std::make_shared<Array2DView>(this);

            if (get_desc().Depth() > 1)
                texture_3d_view = std::make_shared<Texture3DView>(this);
            else
                texture_2d_view = std::make_shared<Texture2DView>(this);
        }
    }

	 ivec3 Texture::get_size(int mip)
	{
		return ivec3(get_desc().Width, get_desc().Height, get_desc().DepthOrArraySize) / (1<<mip);
	}

	 CubemapView::ptr & Texture::cubemap()
	{
		if(!cubemap_view)
			cubemap_view = std::make_shared<CubemapView>(this);

		return cubemap_view;
	}

	 Texture2DView::ptr & Texture::texture_2d()
	{
		return texture_2d_view;
	}

	 Texture3DView::ptr & Texture::texture_3d()
	{
		return texture_3d_view;
	}

    bool Texture::is_rt()
    {
        return get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }


   

    const Texture::ptr Texture::null(new Texture(nullptr));


    bool operator<(const texure_header& l, const texure_header& r)
    {
        if (l.name < r.name)return true;

        if (r.name < l.name)return false;

        if (l.mips < r.mips)return true;

        if (r.mips < l.mips)return false;

        if (l.force_srgb < r.force_srgb)return true;

        if (r.force_srgb < l.force_srgb)return false;

        return false;
    }

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
  /*  CD3DX12_RESOURCE_DESC Texture::get_desc() const
    {
        return resource->get_desc();
    }*/
    Texture::Texture(ComPtr<ID3D12Resource> native, HandleTable rtv, ResourceState state): Resource(native, state)
    {
      //  resource.reset(new Resource(native));
        set_name("Texture");
        //this->rtv = rtv;
		//resource->debug = true;
        texture_2d_view = std::make_shared<Texture2DView>(this, rtv);
        //   init();
    }
    Texture::Texture(ComPtr<ID3D12Resource> native, ResourceState state) : Resource(native,state)
    {
    //    resource.reset(new Resource(native));
        set_name("Texture");
	//	resource->debug = true;
        init();
    }
	Texture::Texture(CD3DX12_RESOURCE_DESC desc, ResourceState state /*= ResourceState::PIXEL_SHADER_RESOURCE*/, HeapType heap_type, std::shared_ptr<texture_data> data /*= nullptr*/) :Resource(desc, heap_type, data ? ResourceState::COPY_DEST : state)
    {
        set_name(std::string("Texture_")+std::to_string(desc.Width)+"_"+std::to_string(desc.Height) + "_" + std::to_string(desc.DepthOrArraySize));
        desc = get_desc();

        if (data)
        {
            tex_data = data;
            auto list = Device::get().get_upload_list();

            for (unsigned int a = 0; a < desc.ArraySize(); a++)
            {
                tex_data->array[a]->mips.resize(desc.MipLevels);

                for (unsigned int m = 0; m < desc.MipLevels; m++)
                {
                    int i = D3D12CalcSubresource(m, a, 0, desc.MipLevels, desc.ArraySize());
                    list->get_copy().update_texture(this, { 0, 0, 0 }, { tex_data->array[a]->mips[m]->width, tex_data->array[a]->mips[m]->height, tex_data->array[a]->mips[m]->depth }, i, (const char* )tex_data->array[a]->mips[m]->data.data(), tex_data->array[a]->mips[m]->width_stride, tex_data->array[a]->mips[m]->slice_stride);
                }
            }

            list->end();
			 list->execute_and_wait();

        }

        init();
    }
	 texture_data::ptr Texture::get_data() const
	{
		auto desc = get_desc();
		//auto list = Device::get().get_upload_list();
	
		auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->begin("readback");

		desc.Format = to_typeless(desc.Format);

		//  auto res = list->read_texture(resource, ivec3(0, 0, 0), ivec3(desc.Width, desc.Height, 1), i);
		texture_data::ptr p_data(new texture_data(desc.ArraySize(), desc.MipLevels, static_cast<uint32_t>(desc.Width), desc.Height, desc.Depth(), desc.Format));
		auto& data = *p_data;
		std::vector<std::future<bool>> tasks;

		for (unsigned int a = 0; a < desc.ArraySize(); a++)
			for (unsigned int m = 0; m < desc.MipLevels; m++)
			{
				int i = D3D12CalcSubresource(m, a, 0, desc.MipLevels, desc.ArraySize());
				tasks.emplace_back(list->get_copy().read_texture(this, ivec3(0, 0, 0), ivec3(data.array[a]->mips[m]->width, data.array[a]->mips[m]->height, data.array[a]->mips[m]->depth), i, [desc, &data, a, m](const char* mem, UINT64 pitch, UINT64 slice, UINT64 size)
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

		for (auto && t : tasks)
			t.wait();

		return p_data;
	}
	 Texture::ptr Texture::compress()
	{
		auto data = texture_data::compress(get_data());

		if (!data) return nullptr;

		auto desc = get_desc();
		desc.Format = data->format;
		desc.MipLevels = data->mip_maps;
		desc.Width = data->width;
		desc.Height = data->height;
		return std::make_shared<Texture>(desc, ResourceState::COMMON,HeapType::DEFAULT,data);
	}
	 Array2DView::ptr & Texture::array2d()
	{
		return array_2d_view;
	}
	DX12::Texture::ptr Texture::load_native(const texure_header& header, resource_file_depender& depender)
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

            CD3DX12_RESOURCE_DESC desc;

            if (tex_data->depth > 1)
                desc = CD3DX12_RESOURCE_DESC::Tex3D(tex_data->format, tex_data->width, tex_data->height, tex_data->depth, tex_data->mip_maps);
            else
                desc = CD3DX12_RESOURCE_DESC::Tex2D(tex_data->format, tex_data->width, tex_data->height, tex_data->array_size, tex_data->mip_maps);

            return std::make_shared<Texture>(desc, ResourceState::PIXEL_SHADER_RESOURCE, HeapType::DEFAULT,tex_data);
        }

        return nullptr;
    }
	CubemapArrayView::CubemapArrayView(Resource* _resource) : View(_resource)
	{
		auto res_desc = resource->get_desc();

		int count = res_desc.DepthOrArraySize / 6;


		for (int i = 0; i < count; i++)
			views.emplace_back(new CubemapView(resource,i));

		if (!(res_desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
		{
			srvs = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1+ count);
			place_srv(srvs[0]);

			for(int i=0;i<count;i++)
				place_srv(srvs[1+i],i);

		}
	}

	void CubemapArrayView::place_srv( Handle  h)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = to_srv(resource->get_desc().Format);

		int count = resource->get_desc().DepthOrArraySize / 6;

			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			srvDesc.TextureCubeArray.First2DArrayFace = 0;
			srvDesc.TextureCubeArray.MipLevels = -1;

			srvDesc.TextureCubeArray.MostDetailedMip = 0;
			srvDesc.TextureCubeArray.NumCubes = count;
			srvDesc.TextureCubeArray.ResourceMinLODClamp = 0;

			Device::get().create_srv(h, resource, srvDesc);
	}

	void CubemapArrayView::place_srv( Handle h, UINT offset)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = to_srv(resource->get_desc().Format);

		int count = resource->get_desc().DepthOrArraySize / 6;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCubeArray.First2DArrayFace = offset*6;
		srvDesc.TextureCubeArray.MipLevels = -1;

		srvDesc.TextureCubeArray.MostDetailedMip = 0;
		srvDesc.TextureCubeArray.NumCubes = 1;
		srvDesc.TextureCubeArray.ResourceMinLODClamp = 0;

		Device::get().create_srv(h, resource, srvDesc);

	}
    CubemapView::CubemapView( Resource* _resource,int offset) : View(_resource)
    {
        auto res_desc = resource->get_desc();
	    single_count =  6*res_desc.MipLevels;

        if (res_desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {

		rtvs = Render::DescriptorHeapManager::get().get_rt()->create_table(single_count);
			
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
                    desc.Texture2DArray.FirstArraySlice = offset*6+ i;

					Device::get().create_rtv(rtvs[i + 6 * m], resource, desc);
				
                }
            }
        }

        if (!(res_desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
        {
            srvs = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1 + res_desc.MipLevels);
            place_srv(srvs[0]);

            for (int i = 0; i < res_desc.MipLevels; i++)
                place_srv(srvs[1 + i], i);

            static_srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
            place_srv(static_srv[0]);
        }

        p.resize(res_desc.MipLevels);
        scissor.resize(res_desc.MipLevels);

        for (int i = 0; i < res_desc.MipLevels; i++)
        {
            int mm = 1<<i;
            p[i].Width = std::max(1.0f, static_cast<float>(resource->get_desc().Width / mm));
            p[i].Height = std::max(1.0f, static_cast<float>(resource->get_desc().Height / mm));
            p[i].TopLeftX = 0;
            p[i].TopLeftY = 0;
            p[i].MinDepth = 0;
            p[i].MaxDepth = 1;
            scissor[i] = { 0, 0, p[i].Width , p[i].Height };
        }

		for(int i=0;i<6;i++)
		{
			faces[i] = std::make_shared<Texture2DView>(_resource,i+offset*6);
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

	void CubemapView::place_srv( Handle  h)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = to_srv(resource->get_desc().Format);

	
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = resource->get_desc().MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		
		Device::get().create_srv(h, resource, srvDesc);
	}

	void CubemapView::place_srv( Handle  h, UINT mip)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = to_srv(resource->get_desc().Format);
		int count = resource->get_desc().DepthOrArraySize / 6;
	//	if (count == 1)
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	//	else
	//		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = mip;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		Device::get().create_srv(h, resource, srvDesc);
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
        p.resize(res_desc.MipLevels);
        scissor.resize(res_desc.MipLevels);

        for (UINT i = 0; i < res_desc.MipLevels; i++)
        {
            UINT mm = 1<<i;
            p[i].Width = std::max(1.0f, static_cast<float>(resource->get_desc().Width / mm));
            p[i].Height = std::max(1.0f, static_cast<float>(resource->get_desc().Height / mm));
            p[i].TopLeftX = 0;
            p[i].TopLeftY = 0;
            p[i].MinDepth = 0;
            p[i].MaxDepth = 1;
            scissor[i] = { 0, 0, p[i].Width , p[i].Height };
        }
    }
    Texture2DView::Texture2DView( Resource* _resource, int array_index ) : View(_resource), array_index(array_index)
    {
        auto res_desc = resource->get_desc();
        //    single_count = 6 * res_desc.MipLevels;
        //      rtvs = Render::DescriptorHeapManager::get().get_rt()->create_table(single_count);
		if (!(res_desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
			srvs = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1 + res_desc.MipLevels);
			place_srv(srvs[0]);
			static_srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
			place_srv(static_srv[0]);

			for (int i = 0; i < res_desc.MipLevels; i++)
				place_srv(srvs[i + 1], i);

		}
        if (res_desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {
            rtvs = Render::DescriptorHeapManager::get().get_rt()->create_table(res_desc.MipLevels);

            for (int i = 0; i < res_desc.MipLevels; i++)
                place_rtv(rtvs[i], i);
        }

        if (resource->get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            uavs = DescriptorHeapManager::get().get_csu_static()->create_table(resource->get_desc().MipLevels);

            for (int i = 0; i < resource->get_desc().MipLevels; i++)
                place_uav(uavs[i], i, 0);

            static_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);
            place_uav(static_uav[0]);
        }

        p.resize(res_desc.MipLevels);
        scissor.resize(res_desc.MipLevels);

        for (UINT16 i = 0; i < res_desc.MipLevels; i++)
        {
            int mm = 1<<i;
            p[i].Width = (float)std::max(1ull, resource->get_desc().Width / mm);
            p[i].Height = (float)std::max(1u, resource->get_desc().Height / mm);
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
    }
	 Handle Texture2DView::get_rtv(UINT mip)
	{
		return rtvs[mip];
	}
	 void Texture2DView::place_srv(Handle  h)
	{
		srv()(h);
	}
	 void Texture2DView::place_srv( Handle  h, UINT mip)
	{
		srv(mip)(h);
	}
	 void Texture2DView::place_rtv( Handle  h, int i)
	{
		rtv(i)(h);
	}
	 void Texture2DView::place_dsv( Handle  h, UINT mip)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = to_dsv(resource->get_desc().Format);
		if(array_index==-1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mip;
		}else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = mip;
			desc.Texture2DArray.FirstArraySlice = array_index;
			desc.Texture2DArray.ArraySize = 1;
		}


		Device::get().create_dsv(h, resource, desc);


	}
	 void Texture2DView::place_uav( Handle  h, UINT mip, UINT slice)
	{
		uav(mip, slice)(h);
	}
	 std::function<void( Handle)> Texture2DView::uav(UINT mip, UINT slice)
	{
		return [this, mip, slice]( Handle  h)
		{
			if (!resource) return;

			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = resource->get_desc().Format;
			if (array_index == -1)
			{
				desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
				desc.Texture2D.PlaneSlice = slice;
				desc.Texture2D.MipSlice = mip;
			}else
			{
				desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.PlaneSlice = slice;
				desc.Texture2DArray.MipSlice = mip;
				desc.Texture2DArray.FirstArraySlice = array_index;
				desc.Texture2DArray.ArraySize = 1;
			}

			Device::get().create_uav(h, resource, desc);

		};
	}
	 std::function<void( Handle)> Texture2DView::srv(PixelSpace space)
	{
		return [this, space]( Handle h)
		{
			if (!resource) return;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);

			if(space==PixelSpace::MAKE_LINERAR)
				srvDesc.Format = to_linear(srvDesc.Format);
			if (array_index == -1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = resource->get_desc().MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			}else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = resource->get_desc().MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

				srvDesc.Texture2DArray.FirstArraySlice = array_index;
				srvDesc.Texture2DArray.PlaneSlice = 0;
				srvDesc.Texture2DArray.ArraySize = 1;
			}

			Device::get().create_srv(h, resource, srvDesc);
		};
	}
	 std::function<void(Handle)> Texture2DView::srv(UINT mip, UINT levels)
	{
		return [this, mip, levels]( Handle  h)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);
			if (array_index == -1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = levels;
				srvDesc.Texture2D.MostDetailedMip = mip;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			}else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = levels;
				srvDesc.Texture2DArray.MostDetailedMip = mip;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;	
				srvDesc.Texture2DArray.FirstArraySlice = array_index;
				srvDesc.Texture2DArray.PlaneSlice = 0;
				srvDesc.Texture2DArray.ArraySize = 1;
			}
			Device::get().create_srv(h, resource, srvDesc);
		};
	}
	 std::function<void( Handle)> Texture2DView::rtv(UINT mip)
	{
		return [this, mip]( Handle  h)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			desc.Format = to_srv(resource->get_desc().Format);
			if (array_index == -1)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = mip;
				desc.Texture2D.PlaneSlice = 0;
			}else
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = mip;
				desc.Texture2DArray.PlaneSlice = 0;
				desc.Texture2DArray.FirstArraySlice = array_index;
				desc.Texture2DArray.ArraySize = 1;
			}
			Device::get().create_rtv(h, resource, desc);
		};
	}
	 Handle Texture2DView::get_srv(UINT mip)
	{
		return srvs[mip + 1];
	}
	 Handle Texture2DView::get_srv()
	{
		return srvs.get_base();
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
        srvs = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1 + resource->get_desc().MipLevels);
        place_srv(srvs[0]);

        for (int i = 0; i < resource->get_desc().MipLevels; i++)
            place_srv(srvs[1 + i], i);

        if (resource->get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            uavs = DescriptorHeapManager::get().get_csu_static()->create_table(resource->get_desc().MipLevels);
            static_uav = DescriptorHeapManager::get().get_csu_static()->create_table(resource->get_desc().MipLevels);

            //		for (int i = 0; i < resource->get_desc().MipLevels; i++)
            for (int i = 0; i < resource->get_desc().MipLevels; i++)
            {
                uavs[i] = uav(i);//place_uav(uavs[i], i);
                static_uav[i] = uav(i);
            }
        }


		static_srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
		place_srv(static_srv[0]);

        /*   auto res_desc = resource->get_desc();
           single_count = 6 * res_desc.MipLevels;
           rtvs = Render::DescriptorHeapManager::get().get_rt()->create_table(single_count);

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
	 void Texture3DView::place_srv( Handle  h, int level)
	{
		srv(level)(h);
	}
	 void Texture3DView::place_uav( Handle  h, int level)
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
		return static_uav.get_base();
	}
	 std::function<void(Handle)> Texture3DView::srv(int level, int levels)
	{
		return [this, level, levels]( Handle  h)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = to_srv(resource->get_desc().Format);
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = (levels == -1) ? (level == -1 ? resource->get_desc().MipLevels : resource->get_desc().MipLevels - level) : (levels);
			srvDesc.Texture3D.MostDetailedMip = level == -1 ? 0 : level;
			srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

			Device::get().create_srv(h, resource, srvDesc);

		};
	}
	 std::function<void(Handle)> Texture3DView::uav(int level)
	{
		return [this, level]( Handle  h)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = resource->get_desc().Format;
			desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = level;
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.WSize = resource->get_desc().DepthOrArraySize / (1 << level);


			Device::get().create_uav(h, resource, desc);
			
		};
	}
    Array2DView::Array2DView(Resource* _resource) : View(_resource)
    {
        if (resource->get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            uavs = DescriptorHeapManager::get().get_csu_static()->create_table(1);
            //	for (int i = 0; i < resource->get_desc().MipLevels; i++)
            place_uav(uavs[0], 0, resource->get_desc().ArraySize(), 0, 0);
            static_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);
            //	for (int i = 0; i < resource->get_desc().MipLevels; i++)
            place_uav(static_uav[0], 0, resource->get_desc().ArraySize(), 0, 0);
        }

        srvs = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
        place_srv(srvs[0]);
        static_srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
        place_srv(static_srv[0]);
        /*  auto res_desc = resource->get_desc();
          single_count = 6 * res_desc.MipLevels;
          rtvs = Render::DescriptorHeapManager::get().get_rt()->create_table(single_count);

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
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = to_srv(resource->get_desc().Format);
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = resource->get_desc().MipLevels;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2DArray.ArraySize = resource->get_desc().DepthOrArraySize;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.PlaneSlice = 0;

		Device::get().create_srv(h, resource, srvDesc);
	
	}

	 void Array2DView::place_dsv(Handle  h, UINT mip, UINT slice, UINT slice_count)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = to_dsv(resource->get_desc().Format);
		desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip;
		desc.Texture2DArray.FirstArraySlice = slice;
		desc.Texture2DArray.ArraySize = slice_count;

		Device::get().create_dsv(h, resource, desc);
	
	}

	 void Array2DView::place_uav(Handle  h, UINT first, UINT count, UINT mip, UINT slice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = resource->get_desc().Format;
		desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.PlaneSlice = slice;
		desc.Texture2DArray.MipSlice = mip;
		desc.Texture2DArray.FirstArraySlice = first;
		desc.Texture2DArray.ArraySize = count;

		Device::get().create_uav(h, resource, desc);
		}

	 std::function<void(Handle)> Array2DView::uav(UINT first, UINT count, UINT mip)
	{
		return [this, first, count, mip](Handle h)
		{
			if (!resource) return;

			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = resource->get_desc().Format;
			desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.PlaneSlice = 0;
			desc.Texture2DArray.MipSlice = mip;
			desc.Texture2DArray.FirstArraySlice = first;
			desc.Texture2DArray.ArraySize = count > 0 ? count : resource->get_desc().ArraySize();

			Device::get().create_uav(h, resource, desc);

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