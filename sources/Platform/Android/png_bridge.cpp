// libpng behind two plain functions, for HAL.Android.TextureData.cpp.
//
// A plain (non-module) translation unit on purpose: png.h pulls in C headers (setjmp.h, stdio.h, zlib.h)
// that cannot sit in a module's global fragment next to the combined header unit, so the module code
// only sees these declarations. Uses libpng's simplified API (png_image_*).
#include <png.h>

#include <cstdint>
#include <cstring>
#include <vector>

// pixels: 8-bit, stride_bytes between rows; layout 0 = RGBA, 1 = BGRA, 2 = single-channel gray.
bool hal_png_encode(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride_bytes, int layout,
                    std::vector<uint8_t>& out)
{
    png_image image;
    std::memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.format = layout == 2 ? PNG_FORMAT_GRAY : layout == 1 ? PNG_FORMAT_BGRA : PNG_FORMAT_RGBA;

    // For 8-bit data a component is a byte, so the row stride in components is the stride in bytes.
    png_alloc_size_t size = (png_alloc_size_t)width * height * 4 + 4096;
    out.resize(size);
    if (!png_image_write_to_memory(&image, out.data(), &size, 0, pixels, (png_int_32)stride_bytes, nullptr))
    {
        // too small a buffer reports the size needed
        if (size <= out.size())
        {
            png_image_free(&image);
            return false;
        }
        out.resize(size);
        if (!png_image_write_to_memory(&image, out.data(), &size, 0, pixels, (png_int_32)stride_bytes, nullptr))
        {
            png_image_free(&image);
            return false;
        }
    }
    out.resize(size);
    png_image_free(&image);
    return true;
}

namespace
{
    struct MemoryReader
    {
        const uint8_t* p;
        size_t left;
    };

    void read_from_memory(png_structp png, png_bytep out, size_t n)
    {
        auto* r = static_cast<MemoryReader*>(png_get_io_ptr(png));
        if (n > r->left)
            png_error(png, "truncated PNG");
        std::memcpy(out, r->p, n);
        r->p += n;
        r->left -= n;
    }
}

// Any PNG to the stored 8-bit RGBA values (alpha 255 when the file has none; 16-bit is cut to 8).
// The classic API on purpose: the simplified one converts files that carry a gAMA chunk to sRGB, and WIC
// (which wrote the Windows goldens, tagging them gAMA 1.0) returns the stored values untouched, so a
// converting decoder would see every dark pixel of a golden much brighter than it is.
bool hal_png_decode(const void* data, size_t size, uint32_t& width, uint32_t& height, std::vector<uint8_t>& rgba)
{
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        return false;
    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        png_destroy_read_struct(&png, nullptr, nullptr);
        return false;
    }

    MemoryReader reader{ static_cast<const uint8_t*>(data), size };
    std::vector<png_bytep> rows;   // before setjmp: png_error longjmps back here, so nothing is skipped
    bool ok = false;

    if (setjmp(png_jmpbuf(png)) == 0)
    {
        png_set_read_fn(png, &reader, read_from_memory);
        png_read_info(png, info);

        const png_uint_32 w = png_get_image_width(png, info);
        const png_uint_32 h = png_get_image_height(png, info);
        const int depth = png_get_bit_depth(png, info);
        const int color = png_get_color_type(png, info);

        if (color == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);
        if (color == PNG_COLOR_TYPE_GRAY && depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);
        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);
        if (depth == 16)
            png_set_strip_16(png);
        if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);   // RGB -> RGBA; nothing to add when there is alpha already
        png_read_update_info(png, info);

        rgba.resize(static_cast<size_t>(w) * h * 4);
        rows.resize(h);
        for (png_uint_32 y = 0; y < h; ++y)
            rows[y] = rgba.data() + static_cast<size_t>(y) * w * 4;
        png_read_image(png, rows.data());

        width = w;
        height = h;
        ok = true;
    }

    png_destroy_read_struct(&png, &info, nullptr);
    return ok;
}
