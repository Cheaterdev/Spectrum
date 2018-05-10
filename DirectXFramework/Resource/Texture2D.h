
struct texture2d_desc : public D3D11_TEXTURE2D_DESC
{
    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& ArraySize;
        ar& BindFlags;
        ar& CPUAccessFlags;
        ar& Format;
        ar& Height;
        ar& MipLevels;
        ar& MiscFlags;
        ar& SampleDesc;
        ar& Usage;
        ar& Width;
    }

    bool operator==(const texture2d_desc& R)
    {
        return (Width == R.Width) && (Height == R.Height) && (Format == R.Format) && (ArraySize == R.ArraySize) && (BindFlags == R.BindFlags) && (MipLevels == R.MipLevels) && (SampleDesc.Quality == R.SampleDesc.Quality) && (SampleDesc.Count == R.SampleDesc.Count) && (CPUAccessFlags == R.CPUAccessFlags);
    }

    texture2d_desc()
    {
        ZeroMemory(this, sizeof(*this));
        ArraySize = 1;
        SampleDesc.Count = 1;
        SampleDesc.Quality = 0;
        MipLevels = 1;
    }

};
class Context;
struct texure_2d_header
{
        std::wstring name;
        bool force_srgb;
        texure_2d_header()
        {}

        texure_2d_header(std::wstring name, bool force_srgb = false)
        {
            this->name = name;
            this->force_srgb = force_srgb;
        }
    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(name)&NVP(force_srgb);
        }
};
bool operator<(const texure_2d_header& l, const texure_2d_header& r);


class Texture2D : public Resource, public resource_manager<Texture2D, texure_2d_header>, protected NativeContextAccessor
{
        friend class resource_manager<Texture2D, texure_2d_header>;
    protected:
        texture2d_desc desc;
        DX11_Texture2D native_texture;
        std::vector<RenderTargetView> render_target_views;
        std::vector<ShaderResourceView> shader_resource_views;
        std::vector<DepthStencilView> depth_stencil_views;

        virtual void init_views();
        static std::shared_ptr<Texture2D> load_native(const texure_2d_header& header, resource_file_depender& depender);
        Texture2D() {}

    public:
        texure_2d_header get_header()
        {
            return header;
        }
        using ptr = s_ptr<Texture2D>;
        static const ptr null;


        Texture2D(DX11_Texture2D tex)
        {
            native_texture = tex;

            if (native_texture)
            {
                native_texture->GetDesc(&desc);
                init_views();
            }

            else
            {
                ZeroMemory(&desc, sizeof(desc));
                shader_resource_views.emplace_back();
            }
        }

        Texture2D(texture2d_desc desc, std::vector<D3D11_SUBRESOURCE_DATA>* data = nullptr);
        virtual ~Texture2D() {}

        texture2d_desc get_desc()
        {
            return desc;
        }
        ShaderResourceView& get_shader_view(unsigned int i = 0)
        {
            return shader_resource_views[i];
        }

        RenderTargetView& get_render_target(unsigned int i = 0)
        {
            return render_target_views[i];
        }

        DepthStencilView& get_depth_stencil(unsigned int i = 0)
        {
            return depth_stencil_views[i];
        }

        virtual native_resource get_native() override
        {
            return native_texture;
        }

        void save_screenshot(Context& context, std::string file_name);

    private:
        friend class boost::serialization::access;
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive& ar, const unsigned int) const
        {
            texture_data data(desc.ArraySize, desc.MipLevels, desc.Width, desc.Height, 1, desc.Format);
            DX11_Texture2D new_tex;
            D3D11_TEXTURE2D_DESC newDesc = desc;
            newDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            newDesc.Usage = D3D11_USAGE_STAGING;
            newDesc.BindFlags = 0;
            newDesc.MiscFlags = 0;
            {
                auto& lock = Device::get().lock();
                Device::get().get_native_device()->CreateTexture2D(&newDesc, nullptr, &new_tex);
                NativeContextAccessor::get_native_context(Device::get().get_context())->CopyResource(new_tex, native_texture);

                for (unsigned int a = 0; a < desc.ArraySize; a++)
                    for (unsigned int m = 0; m < desc.MipLevels; m++)
                    {
                        int i = D3D11CalcSubresource(m, a, desc.MipLevels);
                        D3D11_MAPPED_SUBRESOURCE  res;
                        NativeContextAccessor::get_native_context(Device::get().get_context())->Map(new_tex, i, D3D11_MAP_READ, 0, &res);

                        for (int w = 0; w < data.array[a].mips[m].num_rows; w++)
                            memcpy(data.array[a].mips[m].data.data() + w * data.array[a].mips[m].width_stride, (char*)res.pData + w * res.RowPitch, data.array[a].mips[m].width_stride);

                        NativeContextAccessor::get_native_context(Device::get().get_context())->Unmap(new_tex, i);
                    }
            }
            ar& NVP(data);
        }
        template<class Archive>
        void load(Archive& ar, const unsigned int)
        {
            texture_data data;
            ar& NVP(data);
            D3D11_TEXTURE2D_DESC newDesc;
            newDesc.ArraySize = data.array_size;
            newDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            newDesc.CPUAccessFlags = 0;
            newDesc.Format = data.format;
            newDesc.Height = data.height;
            newDesc.MipLevels = data.mip_maps;
            newDesc.MiscFlags = 0;
            newDesc.SampleDesc.Count = 1;
            newDesc.SampleDesc.Quality = 0;
            newDesc.Usage = D3D11_USAGE_IMMUTABLE;
            newDesc.Width = data.width;
            vector<D3D11_SUBRESOURCE_DATA> sub_data(newDesc.ArraySize * newDesc.MipLevels);

            for (unsigned int a = 0; a < newDesc.ArraySize; a++)
                for (unsigned int m = 0; m < newDesc.MipLevels; m++)
                {
                    int i = m * newDesc.ArraySize + a;
                    sub_data[i].pSysMem = data.array[a].mips[m].data.data();
                    sub_data[i].SysMemPitch = data.array[a].mips[m].width_stride;
                    sub_data[i].SysMemSlicePitch = data.array[a].mips[m].slice_stride;
                }

            {
                auto& lock = Device::get().lock();
                Device::get().get_native_device()->CreateTexture2D(&newDesc, sub_data.data(), &native_texture);
            }

            if (native_texture)
            {
                native_texture->GetDesc(&desc);
                init_views();
            }
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()
};