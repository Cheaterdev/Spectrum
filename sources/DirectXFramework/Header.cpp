#include  "pch_dx.h"
#include <wincodec.h>

#include "DX12/Texture.h"

texture_data::ptr generate_tex_data(DirectX::ScratchImage& image)
{
    DirectX::TexMetadata metadata = image.GetMetadata();
    texture_data::ptr tex_data;

    // Fill out subresource array
    if (metadata.IsVolumemap())
    {
        size_t depth = metadata.depth;
        size_t idx = 0;
        tex_data.reset(new texture_data(1u, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, metadata.format));

        for (size_t level = 0; level < metadata.mipLevels; ++level)
        {
            size_t index = metadata.ComputeIndex(level, 0, 0);
            /*if (index >= nimages)
            return E_FAIL;*/
            const DirectX::Image& img = image.GetImages()[index];
            assert(idx < (metadata.mipLevels * metadata.arraySize));
            int  a = 0;
			size_t m = level;
            memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
            tex_data->array[a]->mips[m]->width_stride = static_cast<UINT>(img.rowPitch);
            tex_data->array[a]->mips[m]->slice_stride = static_cast<UINT>(img.slicePitch);
            /*             sub_data[idx].pData = img.pixels;
            sub_data[idx].RowPitch = static_cast<DWORD>(img.rowPitch);
            sub_data[idx].SlicePitch = static_cast<DWORD>(img.slicePitch);*/
            ++idx;

            if (depth > 1)
                depth >>= 1;
        }
    }

    else
    {
        //--- 1D or 2D texture case ---------------------------------------------------
        size_t idx = 0;
        tex_data.reset(new texture_data((UINT)metadata.arraySize, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, metadata.format));

        for (size_t item = 0; item < metadata.arraySize; ++item)
        {
            for (size_t level = 0; level < metadata.mipLevels; ++level)
            {
                size_t index = metadata.ComputeIndex(level, item, 0);

                if (index >= image.GetImageCount())
                    return nullptr;

                const DirectX::Image& img = image.GetImages()[index];

                if (img.format != metadata.format)
                    return nullptr;

                if (!img.pixels)
                    return nullptr;

                assert(idx < (metadata.mipLevels * metadata.arraySize));
				size_t  a = item;
				size_t m = level;
                memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
                tex_data->array[a]->mips[m]->width_stride = static_cast<UINT>(img.rowPitch);
                tex_data->array[a]->mips[m]->slice_stride = static_cast<UINT>(img.slicePitch);
                /*   sub_data[idx].pData = img.pixels;
                sub_data[idx].RowPitch = static_cast<DWORD>(img.rowPitch);
                sub_data[idx].SlicePitch = static_cast<DWORD>(img.slicePitch);*/
                ++idx;
            }
        }
    }

    return tex_data;
    //  DirectX::CreateTextureEx(Device::get().get_native_device(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, header.force_srgb, (ID3D11Resource**)&native_texture);
}

texture_data::ptr texture_data::compress(texture_data::ptr orig)
{
    DirectX::ScratchImage compressed;
    DirectX::TexMetadata metadata;
    metadata.width = orig->width;
    metadata.height = orig->height;
    metadata.arraySize = orig->array_size;
    metadata.depth = orig->depth;
    metadata.format = orig->format;
    metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
    metadata.mipLevels = orig->mip_maps - 2;
    metadata.miscFlags = 0;
    metadata.miscFlags2 = 0;
    std::vector<DirectX::Image> images(metadata.mipLevels * metadata.arraySize);

    for (unsigned int i = 0; i < orig->array.size(); i++)
        for (unsigned int j = 0; j <  metadata.mipLevels; j++)
        {
            auto& img = images[metadata.ComputeIndex(j, i, 0)];
            img.format = metadata.format;
            img.width = orig->array[i]->mips[j]->width;
            img.height = orig->array[i]->mips[j]->height;
            img.rowPitch = orig->array[i]->mips[j]->width_stride;
            img.slicePitch = orig->array[i]->mips[j]->slice_stride;
            img.pixels = reinterpret_cast<uint8_t*>(orig->array[i]->mips[j]->data.data());
        }
    assert(false);
  //  if (FAILED(DirectX::Compress(DX11::Device::get().get_native_device(), images.data(), images.size(), metadata, DXGI_FORMAT_BC7_UNORM_SRGB, 1, 1, compressed)))
        return nullptr;

    return generate_tex_data(compressed);
}

texture_data::ptr texture_data::load_texture(std::shared_ptr<file> file, int flags)
{
    bool mips = (flags & LoadFlags::GENERATE_MIPS);
    bool compress = (flags & LoadFlags::COMPRESS);

    if (file)
    {
        auto&& data = file->load_all();
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage orig_image;
        auto name = file->file_name;
		std::wstring ext = name.extension().generic_wstring();

		HRESULT hri = CoInitialize(NULL);

		HRESULT hr = NULL;
        if (ext == L".tga")
        {
			hr = DirectX::LoadFromTGAMemory(data.data(), data.size(), &metadata, orig_image);
        }

        else if (ext == L".dds")
        {
			hr = DirectX::LoadFromDDSMemory(data.data(), data.size(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, orig_image);
        }

		else 	hr = DirectX::LoadFromWICMemory(data.data(), data.size(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, orig_image);
	

 if (FAILED(hr))
            return nullptr;
        DirectX::ScratchImage mipChain;
        bool res = true;

        if (!mips || FAILED(DirectX::GenerateMipMaps(orig_image.GetImages(), orig_image.GetImageCount(), orig_image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
            res = false;

        DirectX::ScratchImage& image1 = res ? mipChain : orig_image;
        metadata = image1.GetMetadata();

        if (compress)
        {
            DirectX::ScratchImage compressed;
            res = true;
            metadata.mipLevels -= 2;

            assert(false);
           // if (FAILED(DirectX::Compress(DX11::Device::get().get_native_device(), image1.GetImages(), image1.GetImageCount() - 2, metadata, DXGI_FORMAT_BC7_UNORM_SRGB, 1, 1, compressed)))
                res = false;

            return generate_tex_data(res ? compressed : image1);
        }

        else
            return generate_tex_data(image1);
    }

    return nullptr;
}