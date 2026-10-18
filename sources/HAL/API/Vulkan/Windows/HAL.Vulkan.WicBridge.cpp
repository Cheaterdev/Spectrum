// PNG encode/decode through the Windows Imaging Component, behind the same two functions the Android libpng
// bridge (Platform/Android/png_bridge.cpp) provides. A plain (non-module) translation unit so <wincodec.h>
// and its macros stay out of the HAL module, which declares them with extern "C++".
#include <wincodec.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;

namespace
{
    ComPtr<IWICImagingFactory> create_factory()
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED); // S_FALSE if already inited — fine

        ComPtr<IWICImagingFactory> factory;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
            return nullptr;
        return factory;
    }
}

// pixels: 8-bit, stride_bytes between rows; layout 0 = RGBA, 1 = BGRA, 2 = single-channel gray.
bool hal_png_encode(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride_bytes, int layout,
                    std::vector<uint8_t>& out)
{
    auto factory = create_factory();
    if (!factory)
        return false;

    ComPtr<IStream> stream;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)))
        return false;

    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)))
        return false;
    encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);

    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    encoder->CreateNewFrame(&frame, &props);
    frame->Initialize(props.Get());
    frame->SetSize(width, height);

    WICPixelFormatGUID src_fmt = GUID_WICPixelFormat32bppRGBA;
    WICPixelFormatGUID fmt     = GUID_WICPixelFormat32bppRGBA;
    if (layout == 2)
    {
        src_fmt = GUID_WICPixelFormat8bppGray;
        fmt     = GUID_WICPixelFormat8bppGray;
    }
    else if (layout == 1)
        src_fmt = GUID_WICPixelFormat32bppBGRA;
    frame->SetPixelFormat(&fmt);

    // Wrap the bytes in a WIC bitmap tagged with their real channel order, then WriteSource — WIC converts to
    // whatever format the PNG frame actually chose. (WritePixels would blindly reinterpret the bytes as the
    // frame's native format, swapping R<->B when WIC falls back to BGRA.)
    ComPtr<IWICBitmap> bitmap;
    if (FAILED(factory->CreateBitmapFromMemory(width, height, src_fmt, stride_bytes, stride_bytes * height,
            const_cast<BYTE*>(pixels), &bitmap)))
        return false;

    if (FAILED(frame->WriteSource(bitmap.Get(), nullptr)))
        return false;
    frame->Commit();
    encoder->Commit();

    STATSTG stat{};
    if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)))
        return false;
    const ULONG size = static_cast<ULONG>(stat.cbSize.QuadPart);

    out.resize(size);
    LARGE_INTEGER zero{};
    stream->Seek(zero, STREAM_SEEK_SET, nullptr);
    ULONG read = 0;
    stream->Read(out.data(), size, &read);
    out.resize(read);
    return true;
}

// Any PNG to straight 8-bit RGBA.
bool hal_png_decode(const void* data, size_t size, uint32_t& width, uint32_t& height, std::vector<uint8_t>& rgba)
{
    auto factory = create_factory();
    if (!factory)
        return false;

    ComPtr<IStream> stream;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)))
        return false;
    ULONG written = 0;
    stream->Write(data, static_cast<ULONG>(size), &written);
    LARGE_INTEGER zero{};
    stream->Seek(zero, STREAM_SEEK_SET, nullptr);

    ComPtr<IWICBitmapDecoder> decoder;
    if (FAILED(factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder)))
        return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, &frame)))
        return false;

    UINT w = 0, h = 0;
    frame->GetSize(&w, &h);

    ComPtr<IWICFormatConverter> converter;
    factory->CreateFormatConverter(&converter);
    if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
        return false;

    rgba.resize(static_cast<size_t>(w) * h * 4);
    if (FAILED(converter->CopyPixels(nullptr, w * 4, static_cast<UINT>(rgba.size()), rgba.data())))
        return false;

    width  = w;
    height = h;
    return true;
}
