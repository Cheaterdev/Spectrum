#include "pch.h"

#include <wincodec.h>

namespace DX11
{

	const Texture2D::ptr Texture2D::null(new Texture2D(nullptr));


    void Texture2D::save_screenshot(Context &context, std::string file_name)
    {
		HRESULT hr = DirectX::SaveWICTextureToFile(NativeContextAccessor::get_native_context(context), get_native(),
                     GUID_ContainerFormatPng, convert(file_name).c_str());
    }

	Texture2D::Texture2D(texture2d_desc desc, std::vector<D3D11_SUBRESOURCE_DATA>* data)
    {
		Device::get().get_native_device()->CreateTexture2D(&desc, (data) ? &(*data)[0] : nullptr, &native_texture);

		if (native_texture)
		{
			native_texture->GetDesc(&this->desc);
			init_views();
		}
		else
		{
			ZeroMemory(&desc, sizeof(this->desc));
			shader_resource_views.emplace_back();
		}
    }

    bool operator<(const texure_2d_header &l, const texure_2d_header &r)
    {
		if (l.name == r.name)
			return l.force_srgb < r.force_srgb;
		else
        return l.name < r.name;
    }

    void Texture2D::init_views()
    {
        D3D11_RENDER_TARGET_VIEW_DESC DescRT; ZeroMemory(&DescRT, sizeof(DescRT));
        DescRT.Format = desc.Format;

        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
        {
            if (desc.ArraySize < 2)
            {
                DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                DescRT.Texture2D.MipSlice = 0;
            }
            else
            {
                DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                DescRT.Texture2DArray.ArraySize = desc.SampleDesc.Count;
                DescRT.Texture2DArray.FirstArraySlice = 0;
                DescRT.Texture2DArray.MipSlice = 0;
            }

            render_target_views.emplace_back(this, DescRT);
        }

        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC DescSR;
            DescSR.Format = desc.Format;

            if (desc.ArraySize < 2)
            {
                DescSR.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                DescSR.Texture2D.MipLevels = -1;//Desc.MipLevels;
                DescSR.Texture2D.MostDetailedMip = 0;
            }
            else
            {
                DescSR.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                DescSR.Texture2DArray.ArraySize = desc.SampleDesc.Count;
                DescSR.Texture2DArray.FirstArraySlice = 0;
                DescSR.Texture2DArray.MipLevels = -1;
                DescSR.Texture2DArray.MostDetailedMip = 0;
            }

            DescSR.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

            if (desc.MipLevels == 0)
                desc.MipLevels = 1;

            DescSR.TextureCube.MipLevels = desc.MipLevels;
            DescSR.TextureCube.MostDetailedMip = 0;
            shader_resource_views.emplace_back(this, DescSR);
        }

		if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC DescDS; ZeroMemory(&DescDS, sizeof(DescDS));
			DescDS.Flags = 0;
			DescDS.Format = desc.Format;
			DescDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			DescDS.Texture2D.MipSlice = 0;
			if (desc.SampleDesc.Count > 1)
				DescDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			depth_stencil_views.emplace_back(this, DescDS);
		}


    }

    std::shared_ptr<Texture2D> Texture2D::load_native(const texure_2d_header &header, resource_file_depender &depender)
    {
        DX11_Texture2D native_texture;

        auto file = FileSystem::get().get_file(header.name);

        if (file)
        {
			depender.add_depend(file);
            auto &&data = file->load_all();

			DirectX::TexMetadata data_t;
			DirectX::ScratchImage image;
			DirectX::LoadFromWICMemory(data.data(), data.size(), 0, &data_t, image);


			DirectX::ScratchImage mipChain;
			DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);

			DirectX::CreateTextureEx(Device::get().get_native_device(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, header.force_srgb, (ID3D11Resource**)&native_texture);
        }

        if (native_texture)
        {
			return  std::shared_ptr<Texture2D>(new Texture2D(native_texture));
        }

        return std::shared_ptr<Texture2D>();
    }


}
