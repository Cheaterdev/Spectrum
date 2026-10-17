module;
// PNG encode/decode via the Windows Imaging Component.  WIC is portable across
// graphics backends (it is not a D3D/Vulkan dependency), so it gives the Vulkan
// backend the same golden-image PNG support the D3D12 backend gets through
// DirectXTex — without needing `import d3d12`.
#include <wincodec.h>
#include <wrl/client.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
module HAL:TextureData;

import Core;
import :Utils;
import :Types;
import :Device;

// The image-codec half of the Vulkan texture_data, for Windows (DirectXTex + WIC). The rest of
// texture_data (sizes, readback, the loader that calls load_from_memory) is portable and lives in
// HAL.Vulkan.TextureData.cpp; Android has its own codec half on libpng
// (sources/Platform/Android/HAL.Android.TextureData.cpp).

// HAL.DirectXTexLoad.cpp
bool hal_dxtex_load(const void* data, size_t size, std::string ext, bool generate_mips,
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

    // Encode mip[0] as PNG bytes via WIC.
    // Supported formats: R8G8B8A8_UNORM, B8G8R8A8_UNORM/SRGB, R8_UNORM (grayscale).
    std::vector<uint8_t> texture_data::to_png() const
    {
        using Microsoft::WRL::ComPtr;

        if (array.empty() || array[0]->mips.empty())
            return {};
        auto& mip = array[0]->mips[0];

        CoInitializeEx(nullptr, COINIT_MULTITHREADED); // S_FALSE if already inited — fine

        ComPtr<IWICImagingFactory> factory;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
            return {};

        ComPtr<IStream> stream;
        if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)))
            return {};

        ComPtr<IWICBitmapEncoder> encoder;
        if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)))
            return {};
        encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);

        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> props;
        encoder->CreateNewFrame(&frame, &props);
        frame->Initialize(props.Get());
        frame->SetSize(mip->width, mip->height);

        // Pick the WIC pixel format that matches the raw byte layout of mip->data.
        WICPixelFormatGUID src_fmt;
        WICPixelFormatGUID dst_fmt;
        if (format == Format::R8_UNORM)
        {
            src_fmt = GUID_WICPixelFormat8bppGray;
            dst_fmt = GUID_WICPixelFormat8bppGray;
        }
        else if (format == Format::B8G8R8A8_UNORM || format == Format::B8G8R8A8_UNORM_SRGB)
        {
            src_fmt = GUID_WICPixelFormat32bppBGRA;
            dst_fmt = GUID_WICPixelFormat32bppRGBA;
        }
        else
        {
            src_fmt = GUID_WICPixelFormat32bppRGBA;
            dst_fmt = GUID_WICPixelFormat32bppRGBA;
        }

        WICPixelFormatGUID fmt = dst_fmt;
        frame->SetPixelFormat(&fmt);

        const UINT stride   = mip->width_stride;
        const UINT buf_size = stride * mip->height;

        // Wrap the readback bytes in a WIC bitmap tagged with their real channel
        // order, then WriteSource — WIC converts to whatever format the PNG frame
        // actually chose.  (WritePixels would blindly reinterpret the bytes as the
        // frame's native format, swapping R<->B when WIC falls back to BGRA.)
        ComPtr<IWICBitmap> bitmap;
        if (FAILED(factory->CreateBitmapFromMemory(mip->width, mip->height,
                src_fmt, stride, buf_size,
                reinterpret_cast<BYTE*>(const_cast<unsigned char*>(mip->data.data())),
                &bitmap)))
            return {};

        if (FAILED(frame->WriteSource(bitmap.Get(), nullptr)))
            return {};
        frame->Commit();
        encoder->Commit();

        STATSTG stat{};
        if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)))
            return {};
        const ULONG size = static_cast<ULONG>(stat.cbSize.QuadPart);

        std::vector<uint8_t> out(size);
        LARGE_INTEGER zero{};
        stream->Seek(zero, STREAM_SEEK_SET, nullptr);
        ULONG read = 0;
        stream->Read(out.data(), size, &read);
        out.resize(read);
        return out;
    }

    // Decode PNG bytes into an R8G8B8A8_UNORM texture_data via WIC.
    texture_data::ptr texture_data::from_png(const void* data, size_t size)
    {
        using Microsoft::WRL::ComPtr;

        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        ComPtr<IWICImagingFactory> factory;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
            return nullptr;

        ComPtr<IStream> stream;
        if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)))
            return nullptr;
        ULONG written = 0;
        stream->Write(data, static_cast<ULONG>(size), &written);
        LARGE_INTEGER zero{};
        stream->Seek(zero, STREAM_SEEK_SET, nullptr);

        ComPtr<IWICBitmapDecoder> decoder;
        if (FAILED(factory->CreateDecoderFromStream(stream.Get(), nullptr,
                WICDecodeMetadataCacheOnDemand, &decoder)))
            return nullptr;

        ComPtr<IWICBitmapFrameDecode> frame;
        if (FAILED(decoder->GetFrame(0, &frame)))
            return nullptr;

        UINT w = 0, h = 0;
        frame->GetSize(&w, &h);

        // Convert whatever the PNG is to straight 32bpp RGBA.
        ComPtr<IWICFormatConverter> converter;
        factory->CreateFormatConverter(&converter);
        if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
            return nullptr;

        auto result = std::make_shared<texture_data>(1, 1, w, h, 1, Format::R8G8B8A8_UNORM);
        auto& mip = result->array[0]->mips[0];

        const UINT stride   = w * 4;
        const UINT buf_size = stride * h;
        if (FAILED(converter->CopyPixels(nullptr, stride, buf_size, mip->data.data())))
            return nullptr;

        return result;
    }
}
