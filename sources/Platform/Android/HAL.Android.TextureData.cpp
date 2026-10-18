module HAL:TextureData;

import Core;
import :Utils;
import :Types;
import :Device;

// The image-codec half of texture_data on Android: PNG through libpng (png_bridge.cpp). It replaces
// HAL.Vulkan.TextureData.Windows.cpp (DirectXTex + WIC). Only PNG is decoded here: the device has no other
// decoders, which is why spectrum_tool pre-bakes textures into the cache; this is for the golden-image
// tests (to_png / from_png) and for PNG blobs passed to load_from_memory.

// png_bridge.cpp. extern "C++" keeps the declarations out of this module, so they link to the plain definitions.
extern "C++" bool hal_png_encode(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride_bytes,
                                 int layout, std::vector<uint8_t>& out);
extern "C++" bool hal_png_decode(const void* data, size_t size, uint32_t& width, uint32_t& height,
                                 std::vector<uint8_t>& rgba);

namespace HAL
{
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

    // Supported like the Windows one: R8G8B8A8_UNORM (and anything else, taken as RGBA), B8G8R8A8_UNORM/SRGB, R8_UNORM.
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

    // PNG only (see above); anything else is not decodable on the device.
    texture_data::ptr texture_data::load_from_memory(const void* data, size_t size, std::string format_hint, int flags)
    {
        static const uint8_t png_magic[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
        if (size < sizeof(png_magic) || std::memcmp(data, png_magic, sizeof(png_magic)) != 0)
            return nullptr;
        auto base = from_png(data, size);
        if (!base || !(flags & LoadFlags::GENERATE_MIPS))
            return base;

        // DirectXTex builds the chain on Windows; here a 2x2 box filter does (R8G8B8A8, edge texels clamped).
        auto& top = *base->array[0]->mips[0];
        uint32_t mips = 1;
        for (uint32_t s = std::max(top.width, top.height); s > 1; s >>= 1)
            ++mips;

        auto result = std::make_shared<texture_data>(1, mips, top.width, top.height, 1, base->format);
        std::memcpy(result->array[0]->mips[0]->data.data(), top.data.data(),
                    std::min(result->array[0]->mips[0]->data.size(), top.data.size()));
        for (uint32_t m = 1; m < mips; ++m)
        {
            auto& src = *result->array[0]->mips[m - 1];
            auto& dst = *result->array[0]->mips[m];
            for (uint32_t y = 0; y < dst.height; ++y)
                for (uint32_t x = 0; x < dst.width; ++x)
                {
                    const uint32_t x0 = std::min(x * 2, src.width - 1),  x1 = std::min(x * 2 + 1, src.width - 1);
                    const uint32_t y0 = std::min(y * 2, src.height - 1), y1 = std::min(y * 2 + 1, src.height - 1);
                    for (uint32_t c = 0; c < 4; ++c)
                    {
                        const auto at = [&](uint32_t px, uint32_t py) { return uint32_t(src.data[py * src.width_stride + px * 4 + c]); };
                        dst.data[y * dst.width_stride + x * 4 + c] =
                            uint8_t((at(x0, y0) + at(x1, y0) + at(x0, y1) + at(x1, y1) + 2) / 4);
                    }
                }
        }
        return result;
    }
}
