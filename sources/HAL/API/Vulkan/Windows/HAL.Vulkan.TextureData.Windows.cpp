module HAL:TextureData;

import Core;
import :Utils;
import :Types;
import :Device;

// The image-codec half of the Vulkan texture_data, for Windows (DirectXTex + WIC, the latter in HAL.Vulkan.WicBridge.cpp). The rest of
// texture_data (sizes, readback, the loader that calls load_from_memory) is portable and lives in
// HAL.Vulkan.TextureData.cpp; Android has its own codec half on libpng
// (sources/Platform/Android/HAL.Android.TextureData.cpp).

// WIC is portable across graphics backends (it is not a D3D/Vulkan dependency), so it gives the Vulkan
// backend the same golden-image PNG support the D3D12 backend gets through DirectXTex.
// HAL.Vulkan.WicBridge.cpp; extern "C++" keeps the declarations out of this module.
extern "C++" bool hal_png_encode(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride_bytes,
                                 int layout, std::vector<uint8_t>& out);
extern "C++" bool hal_png_decode(const void* data, size_t size, uint32_t& width, uint32_t& height,
                                 std::vector<uint8_t>& rgba);

// HAL.DirectXTexLoad.cpp
extern "C++" bool hal_dxtex_load(const void* data, size_t size, std::string ext, bool generate_mips,
                    std::vector<uint32_t>& desc,
                    std::vector<std::vector<uint8_t>>& subresources,
                    std::vector<uint32_t>& row_pitches,
                    std::vector<uint32_t>& slice_pitches);

namespace HAL
{
    texture_data::ptr texture_data::load_from_memory(const void* data, size_t size, std::string format_hint, int flags)
    {
        std::vector<uint32_t> desc, row_pitches, slice_pitches;
        std::vector<std::vector<uint8_t>> subresources;
        if (!hal_dxtex_load(data, size, std::move(format_hint), (flags & LoadFlags::GENERATE_MIPS) != 0,
                            desc, subresources, row_pitches, slice_pitches))
            return nullptr;

        const Format format(static_cast<Format::Formats>(desc[0]));
        const uint32_t array_size = desc[4], mips = desc[5];
        auto result = std::make_shared<texture_data>(array_size, mips, desc[1], desc[2], desc[3], format);

        // Same copy as D3D12's generate_tex_data: the buffer texture_data sized,
        // with DirectXTex's own pitches.
        size_t i = 0;
        for (uint32_t a = 0; a < array_size; ++a)
            for (uint32_t m = 0; m < mips; ++m, ++i)
            {
                auto& mip = *result->array[a]->mips[m];
                std::memcpy(mip.data.data(), subresources[i].data(), std::min(mip.data.size(), subresources[i].size()));
                mip.width_stride = row_pitches[i];
                mip.slice_stride = slice_pitches[i];
            }
        return result;
    }

    // Supported formats: R8G8B8A8_UNORM, B8G8R8A8_UNORM/SRGB, R8_UNORM (grayscale).
    std::vector<uint8_t> texture_data::to_png() const
    {
        if (array.empty() || array[0]->mips.empty())
            return {};
        auto& mip = array[0]->mips[0];

        int layout = 0;
        if (format == Format::R8_UNORM)
            layout = 2;
        else if (format == Format::B8G8R8A8_UNORM || format == Format::B8G8R8A8_UNORM_SRGB)
            layout = 1;

        std::vector<uint8_t> out;
        if (!hal_png_encode(mip->data.data(), mip->width, mip->height, mip->width_stride, layout, out))
            return {};
        return out;
    }

    // Decode PNG bytes into an R8G8B8A8_UNORM texture_data.
    texture_data::ptr texture_data::from_png(const void* data, size_t size)
    {
        uint32_t w = 0, h = 0;
        std::vector<uint8_t> rgba;
        if (!hal_png_decode(data, size, w, h, rgba))
            return nullptr;

        auto result = std::make_shared<texture_data>(1, 1, w, h, 1, Format::R8G8B8A8_UNORM);
        auto& mip = result->array[0]->mips[0];
        std::memcpy(mip->data.data(), rgba.data(), std::min(mip->data.size(), rgba.size()));
        return result;
    }
}
