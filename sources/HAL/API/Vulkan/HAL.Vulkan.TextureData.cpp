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

    texture_data::ptr texture_data::load_texture(std::shared_ptr<file> file, int /*flags*/)
    {
        // WIC's stream decoder auto-detects the container (JPEG/PNG/BMP/…), so
        // from_png() handles any of them — decode straight from the file bytes.
        if (!file) return nullptr;
        auto bytes = file->load_all();
        return from_png(bytes.data(), bytes.size());
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
