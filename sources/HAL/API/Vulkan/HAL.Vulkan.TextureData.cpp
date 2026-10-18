module HAL:TextureData;

import Core;
import :Utils;
import :Types;
import :Device;

// Vulkan implementation of HAL::texture_data.
// Mirrors D3D12/HAL.D3D12.TextureData.cpp.  The size/layout math is portable
// (uses Format::surface_info), so the constructors are identical.  This file is the
// platform-neutral half; the image codecs (load_from_memory, to_png, from_png) are per platform:
// HAL.Vulkan.TextureData.Windows.cpp (DirectXTex + WIC) and, for Android,
// sources/Platform/Android/HAL.Android.TextureData.cpp (libpng).

namespace HAL
{
    texture_mip_data::texture_mip_data(UINT w, UINT h, UINT d, Format format)
    {
        width = w;
        height = h;
        depth = d;
        auto info = format.surface_info({ w, h });
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
            width  /= 2; if (width < 1)  width = 1;
            height /= 2; if (height < 1) height = 1;
            depth  /= 2; if (depth < 1)  depth = 1;
        }
    }

    texture_data::texture_data(uint32_t array_count, uint32_t num_mips, uint32_t width,
                               uint32_t height, uint32_t depth, Format format)
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

    texture_data::ptr texture_data::compress(texture_data::ptr orig)
    {
        // Phase 4+: BC compression via a portable encoder.  For now return the
        // original uncompressed data.
        return orig;
    }

    texture_data::ptr texture_data::load_texture(std::shared_ptr<file> file, int flags)
    {
        if (!file) return nullptr;
        auto bytes = file->load_all();
        return load_from_memory(bytes.data(), bytes.size(), file->file_name.extension().generic_string(), flags);
    }

    // Build from GPU readback data: strips row padding (layout.row_stride →
    // width_stride).  Portable — no graphics-API dependency.  Identical to the
    // D3D12 backend implementation.
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
}
