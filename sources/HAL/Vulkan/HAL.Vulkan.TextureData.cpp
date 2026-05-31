module HAL:TextureData;

import Core;
import :Utils;
import :Types;
import :Device;

// Vulkan implementation of HAL::texture_data.
// Mirrors D3D12/HAL.D3D12.TextureData.cpp.  The size/layout math is portable
// (uses Format::surface_info), so the constructors are identical.  The file
// loaders use DirectXTex on D3D12; on Vulkan they are stubbed for now (Phase
// 4+: replace with a portable image loader or reuse DirectXTex which is
// API-agnostic for decode and only needs a format table).

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

    texture_data::ptr texture_data::load_texture(std::shared_ptr<file> /*file*/, int /*flags*/)
    {
        // Phase 4+: portable image decode (DirectXTex is API-agnostic for the
        // decode path and can be reused here).  Stub for Phase 0.
        return nullptr;
    }
}
