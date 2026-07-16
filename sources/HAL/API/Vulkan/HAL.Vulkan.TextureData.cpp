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

    // ---- Minimal DDS parser -------------------------------------------------
    // WIC cannot decode HDR-float or volume DDS (atmospheric LUTs etc.).  The GPU
    // consumes DDS surface data directly (BC blocks, float, 3D — no CPU decode),
    // so we just parse the header and hand the raw subresources to texture_data.
    namespace
    {
        constexpr uint32_t DDS_MAGIC        = 0x20534444; // "DDS "
        constexpr uint32_t DDPF_ALPHA       = 0x2;
        constexpr uint32_t DDPF_FOURCC      = 0x4;
        constexpr uint32_t DDPF_RGB         = 0x40;
        constexpr uint32_t DDPF_LUMINANCE   = 0x20000;
        constexpr uint32_t DDSCAPS2_VOLUME  = 0x200000;
        constexpr uint32_t DDSCAPS2_CUBEMAP = 0x200;
        constexpr uint32_t DDS_DIMENSION_TEXTURE3D = 4;

        constexpr uint32_t make_fourcc(char a, char b, char c, char d)
        { return uint32_t(a) | (uint32_t(b) << 8) | (uint32_t(c) << 16) | (uint32_t(d) << 24); }

    #pragma pack(push, 1)
        struct DDS_PIXELFORMAT { uint32_t size, flags, fourCC, RGBBitCount, RBitMask, GBitMask, BBitMask, ABitMask; };
        struct DDS_HEADER
        {
            uint32_t size, flags, height, width, pitchOrLinearSize, depth, mipMapCount;
            uint32_t reserved1[11];
            DDS_PIXELFORMAT ddspf;
            uint32_t caps, caps2, caps3, caps4, reserved2;
        };
        struct DDS_HEADER_DXT10 { uint32_t dxgiFormat, resourceDimension, miscFlag, arraySize, miscFlags2; };
    #pragma pack(pop)

        // Legacy D3DFMT fourCC / numeric formats → HAL Format (== DXGI numbering).
        Format legacy_format(const DDS_PIXELFORMAT& pf)
        {
            if (pf.flags & DDPF_FOURCC)
            {
                switch (pf.fourCC)
                {
                case make_fourcc('D','X','T','1'): return Format::BC1_UNORM;
                case make_fourcc('D','X','T','3'): return Format::BC2_UNORM;
                case make_fourcc('D','X','T','5'): return Format::BC3_UNORM;
                case 111: return Format::R16_FLOAT;
                case 112: return Format::R16G16_FLOAT;
                case 113: return Format::R16G16B16A16_FLOAT;
                case 114: return Format::R32_FLOAT;
                case 115: return Format::R32G32_FLOAT;
                case 116: return Format::R32G32B32A32_FLOAT;
                default:  break;
                }
            }
            // Uncompressed formats — pick by pixel-format flags and bit count so the
            // per-pixel size matches the file (a wrong size fails the length check).
            if ((pf.flags & DDPF_ALPHA) && pf.RGBBitCount == 8)
                return Format::A8_UNORM;                      // alpha-only (e.g. best-fit normals)
            if ((pf.flags & DDPF_LUMINANCE) && pf.RGBBitCount == 8)
                return Format::R8_UNORM;                      // single-channel
            if (pf.flags & DDPF_RGB)
            {
                if (pf.RGBBitCount == 32)
                    return (pf.RBitMask == 0x00ff0000)        // BGRA vs RGBA channel order
                         ? Format::B8G8R8A8_UNORM : Format::R8G8B8A8_UNORM;
            }
            return Format::R8G8B8A8_UNORM;
        }
    }

    texture_data::ptr texture_data::load_from_memory(const void* blob, size_t size, std::string /*format_hint*/, int /*flags*/)
    {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(blob);

        // DDS is detected by magic, so the format hint is not needed here.
        const bool is_dds = size >= 4 && *reinterpret_cast<const uint32_t*>(data) == DDS_MAGIC;
        if (!is_dds)
        {
            // WIC's stream decoder auto-detects PNG/JPEG/BMP containers.
            return from_png(blob, size);
        }

        if (size < 4 + sizeof(DDS_HEADER)) return nullptr;
        const DDS_HEADER& hdr = *reinterpret_cast<const DDS_HEADER*>(data + 4);
        size_t offset = 4 + sizeof(DDS_HEADER);

        Format   format     = legacy_format(hdr.ddspf);
        uint32_t array_size = 1;
        uint32_t depth      = (hdr.caps2 & DDSCAPS2_VOLUME) ? std::max<uint32_t>(hdr.depth, 1u) : 1u;

        const bool has_dx10 = (hdr.ddspf.flags & DDPF_FOURCC) &&
                              hdr.ddspf.fourCC == make_fourcc('D','X','1','0');
        if (has_dx10)
        {
            if (size < offset + sizeof(DDS_HEADER_DXT10)) return nullptr;
            const DDS_HEADER_DXT10& dx10 = *reinterpret_cast<const DDS_HEADER_DXT10*>(data + offset);
            offset += sizeof(DDS_HEADER_DXT10);
            format     = Format(static_cast<Format::Formats>(dx10.dxgiFormat));
            array_size = std::max<uint32_t>(dx10.arraySize, 1u);
            if (dx10.resourceDimension == DDS_DIMENSION_TEXTURE3D)
                depth = std::max<uint32_t>(hdr.depth, 1u);
        }
        if (hdr.caps2 & DDSCAPS2_CUBEMAP) array_size *= 6;

        const uint32_t width  = std::max<uint32_t>(hdr.width, 1u);
        const uint32_t height = std::max<uint32_t>(hdr.height, 1u);
        const uint32_t mips   = std::max<uint32_t>(hdr.mipMapCount, 1u);

        auto result = std::make_shared<texture_data>(array_size, mips, width, height, depth, format);

        // DDS surface order: array-major, then mip (largest first), depth slices
        // packed per mip.  texture_data already sized each mip's data buffer with
        // the same format's surface_info, so DDS packing matches — copy in order.
        const uint8_t* src = data + offset;
        const uint8_t* end = data + size;
        for (uint32_t a = 0; a < array_size; ++a)
            for (uint32_t m = 0; m < mips; ++m)
            {
                auto& dst = result->array[a]->mips[m]->data;
                if (src + dst.size() > end) return nullptr; // truncated file
                std::memcpy(dst.data(), src, dst.size());
                src += dst.size();
            }

        return result;
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
