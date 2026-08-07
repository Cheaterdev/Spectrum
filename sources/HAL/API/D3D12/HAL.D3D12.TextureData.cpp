
module HAL:TextureData;

import :Utils;
import :Types;
import :Device;
import Core;
import d3d12;

// Non-module bridge: DirectXTex calls that take DXGI_FORMAT can't be made
// directly from module code (DXGI_FORMAT is module-attached via import d3d12).
// These wrappers live in HAL.D3D12.DirectXTexBridge.cpp (a plain .cpp TU)
// where the header-based DXGI_FORMAT matches DirectXTex's ABI.
std::vector<uint8_t> hal_r8_to_png(const uint8_t* src, uint32_t width, uint32_t height);

namespace HAL
{

    std::unique_ptr<DirectXTex::TexMetadata> get_metadata(std::shared_ptr<file> file)
    {
        auto name = file->file_name.generic_wstring();
        auto ext = file->file_name.extension().generic_wstring();
        //  std::wstring ext = to_lower(name.substr(name.find_last_of(L".") + 1));
        DirectXTex::TexMetadata metadata;

        if (ext == L"tga")
        {
            if (SUCCEEDED(DirectXTex::GetMetadataFromTGAFile(name.c_str(), metadata)))
                return std::make_unique<DirectXTex::TexMetadata>(metadata);
        }

        else if (SUCCEEDED(DirectXTex::GetMetadataFromWICFile(name.c_str(), DirectXTex::WIC_FLAGS::WIC_FLAGS_NONE, metadata)))
            return std::make_unique<DirectXTex::TexMetadata>(metadata);

        return nullptr;
    }

    texture_mip_data::texture_mip_data(UINT w, UINT h, UINT d, Format format)
    {
        width = w;
        height = h;
        depth = d;
        auto info = format.surface_info({w,h});
        width_stride = info.rowBytes;
        slice_stride = static_cast<uint>(info.numBytes);
        num_rows = info.numRows;
        data.resize(slice_stride * d);
    }

        mip::mip(uint32_t count, uint32_t width, uint32_t height, uint32_t depth, Format format)
    {
        mips.reserve(count);

        for (uint32_t i = 0; i < count; i++)
        {
            mips.emplace_back(std::make_shared<texture_mip_data>(width, height, depth, format));
            width /= 2;
            height /= 2;
            depth /= 2;

            if (width < 1) width = 1;

            if (height < 1) height = 1;

            if (depth < 1) depth = 1;
        }
    }

    texture_data::texture_data(uint32_t array_count, uint32_t num_mips, uint32_t width, uint32_t height, uint32_t depth, Format format)
    {
        array_size = array_count;
        this->depth = depth;
        this->format = format;
        this->height = height;
        this->mip_maps = num_mips;
        this->width = width;
        array.reserve(array_count);

        for (uint32_t i = 0; i < array_count; i++)
            array.emplace_back(std::make_shared<mip>(num_mips, width, height, depth, format));
    }

}

namespace HAL
{

    texture_data::ptr generate_tex_data(DirectXTex::ScratchImage& image)
    {
        DirectXTex::TexMetadata metadata = image.GetMetadata();
        texture_data::ptr tex_data;

        // Fill out subresource array
        if (metadata.IsVolumemap())
        {
            size_t depth = metadata.depth;
            size_t idx = 0;
            tex_data.reset(new texture_data(1u, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, from_native(metadata.format)));

            for (size_t level = 0; level < metadata.mipLevels; ++level)
            {
                size_t index = metadata.ComputeIndex(level, 0, 0);
                /*if (index >= nimages)
                return E_FAIL;*/
                const DirectXTex::Image& img = image.GetImages()[index];
                ASSERT(idx < (metadata.mipLevels* metadata.arraySize));
                int  a = 0;
                size_t m = level;
                std::memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
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
            tex_data.reset(new texture_data((UINT)metadata.arraySize, (UINT)metadata.mipLevels, (UINT)metadata.width, (UINT)metadata.height, (UINT)metadata.depth, from_native(metadata.format)));

            for (size_t item = 0; item < metadata.arraySize; ++item)
            {
                for (size_t level = 0; level < metadata.mipLevels; ++level)
                {
                    size_t index = metadata.ComputeIndex(level, item, 0);

                    if (index >= image.GetImageCount())
                        return nullptr;

                    const DirectXTex::Image& img = image.GetImages()[index];

                    if (img.format != metadata.format)
                        return nullptr;

                    if (!img.pixels)
                        return nullptr;

                    ASSERT(idx < (metadata.mipLevels* metadata.arraySize));
                    size_t  a = item;
                    size_t m = level;
                    std::memcpy(tex_data->array[a]->mips[m]->data.data(), img.pixels, tex_data->array[a]->mips[m]->data.size());
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
        //  DirectXTex::CreateTextureEx(Device::get().get_native_device(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, header.force_srgb, (ID3D11Resource**)&native_texture);
    }

    texture_data::ptr texture_data::compress(texture_data::ptr orig)
    {
        DirectXTex::ScratchImage compressed;
        DirectXTex::TexMetadata metadata;
        metadata.width = orig->width;
        metadata.height = orig->height;
        metadata.arraySize = orig->array_size;
        metadata.depth = orig->depth;
        metadata.format = to_native(orig->format);
        metadata.dimension = DirectXTex::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;
        metadata.mipLevels = orig->mip_maps - 2;
        metadata.miscFlags = 0;
        metadata.miscFlags2 = 0;
        std::vector<DirectXTex::Image> images(metadata.mipLevels * metadata.arraySize);

        for (unsigned int i = 0; i < orig->array.size(); i++)
            for (unsigned int j = 0; j < metadata.mipLevels; j++)
            {
                auto& img = images[metadata.ComputeIndex(j, i, 0)];
                img.format = metadata.format;
                img.width = orig->array[i]->mips[j]->width;
                img.height = orig->array[i]->mips[j]->height;
                img.rowPitch = orig->array[i]->mips[j]->width_stride;
                img.slicePitch = orig->array[i]->mips[j]->slice_stride;
                img.pixels = reinterpret_cast<uint8_t*>(orig->array[i]->mips[j]->data.data());
            }
        //ASSERT(false);
      //  if (FAILED(DirectXTex::Compress(images.data(), images.size(), metadata,  to_native(Format::BC7_UNORM_SRGB), DirectXTex::TEX_COMPRESS_FLAGS::TEX_COMPRESS_DEFAULT | DirectXTex::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL, 1, compressed)))
     //       return nullptr;

        return generate_tex_data(compressed);
    }

    texture_data::ptr texture_data::from_readback(uint width, uint height, Format fmt,
                                                    std::span<const std::byte> gpu_data,
                                                    const texture_layout& layout)
    {
        auto result = std::make_shared<texture_data>(1, 1, width, height, 1, fmt);
        auto& mip = result->array[0]->mips[0];

        uint row_bytes = mip->width_stride;
        for (uint row = 0; row < mip->num_rows; ++row)
        {
            auto src = reinterpret_cast<const uint8_t*>(gpu_data.data()) + row * layout.row_stride;
            auto dst = mip->data.data() + row * row_bytes;
            std::memcpy(dst, src, row_bytes);
        }
        return result;
    }

    std::vector<uint8_t> texture_data::to_png() const
    {
        if (array.empty() || array[0]->mips.empty())
            return {};

        auto& mip_data = array[0]->mips[0];

        DirectXTex::Image img;
        img.width      = mip_data->width;
        img.height     = mip_data->height;
        img.format     = to_native(format);
        img.rowPitch   = mip_data->width_stride;
        img.slicePitch = mip_data->slice_stride;
        img.pixels     = reinterpret_cast<uint8_t*>(const_cast<unsigned char*>(mip_data->data.data()));

        const DirectXTex::Image* save_img = &img;
        DirectXTex::ScratchImage converted;

        CoInitialize(nullptr);

        if (format == Format::R8_UNORM)
            return hal_r8_to_png(img.pixels, (uint32_t)img.width, (uint32_t)img.height);

        DirectXTex::Blob blob;
        if (FAILED(DirectXTex::SaveToWICMemory(*save_img, DirectXTex::WIC_FLAGS_NONE,
	        GUID_ContainerFormatPng, blob)))
            return {};

        auto* bytes = static_cast<const uint8_t*>(blob.GetBufferPointer());
        return std::vector<uint8_t>(bytes, bytes + blob.GetBufferSize());
    }

    texture_data::ptr texture_data::from_png(const void* data, size_t size)
    {
        CoInitialize(nullptr);

        DirectXTex::TexMetadata metadata;
        DirectXTex::ScratchImage image;
        if (FAILED(DirectXTex::LoadFromWICMemory(
                static_cast<const uint8_t*>(data), size,
                DirectXTex::WIC_FLAGS_NONE, &metadata, image)))
            return nullptr;

        return generate_tex_data(image);
    }

    texture_data::ptr texture_data::load_from_memory(const void* data, size_t size, std::string format_hint, int flags)
    {
        bool mips = (flags & LoadFlags::GENERATE_MIPS);

        if (!format_hint.empty() && format_hint[0] == '.')
            format_hint.erase(0, 1);

        for (auto& c : format_hint)
            if (c >= 'A' && c <= 'Z') c += 'a' - 'A';

        DirectXTex::TexMetadata metadata;
        DirectXTex::ScratchImage orig_image;

        CoInitialize(nullptr);

        auto bytes = reinterpret_cast<const uint8_t*>(data);
        HRESULT hr = 0;
        if (format_hint == "tga")
            hr = DirectXTex::LoadFromTGAMemory(bytes, size, &metadata, orig_image);
        else if (format_hint == "dds")
            hr = DirectXTex::LoadFromDDSMemory(bytes, size, DirectXTex::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, orig_image);
        else
            hr = DirectXTex::LoadFromWICMemory(bytes, size, DirectXTex::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, orig_image);

        if (FAILED(hr))
            return nullptr;

        DirectXTex::ScratchImage mipChain;
        bool res = true;

        if (!mips || FAILED(DirectXTex::GenerateMipMaps(orig_image.GetImages(), orig_image.GetImageCount(), orig_image.GetMetadata(), DirectXTex::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT, 0, mipChain)))
            res = false;

        return generate_tex_data(res ? mipChain : orig_image);
    }

    texture_data::ptr texture_data::load_texture(std::shared_ptr<file> file, int flags)
    {
        if (!file)
            return nullptr;

        auto&& data = file->load_all();
        return load_from_memory(data.data(), data.size(), file->file_name.extension().generic_string(), flags);
    }

}