// Non-module TU: the GDeflate CPU codec of DirectStorage, shared by both Windows backends (the codec is a plain
// CPU object that needs no D3D12 device; dstoragecore.dll is copied next to every executable). Declared extern "C++"
// in HAL.Device.ixx; <dstorage.h> stays out of the module.

#include <dstorage.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace HAL
{
    namespace
    {
        // One per thread: dstorage.h says a compression codec is not thread safe and each thread needs its own, and
        // pages are expanded and compressed on many pool threads at once. A single worker thread each, the callers
        // being the ones that spread the work (a page is one 64 KB GDeflate tile, there is nothing to split).
        IDStorageCompressionCodec* gdeflate_codec()
        {
            static thread_local ComPtr<IDStorageCompressionCodec> codec = []() {
                ComPtr<IDStorageCompressionCodec> created;
                if (FAILED(DStorageCreateCompressionCodec(DSTORAGE_COMPRESSION_FORMAT_GDEFLATE, 1, IID_PPV_ARGS(&created))))
                    created.Reset();
                return created;
            }();
            return codec.Get();
        }
    }

    bool gdeflate_supported() { return gdeflate_codec() != nullptr; }

    std::vector<uint8_t> gdeflate_compress(const uint8_t* data, size_t size)
    {
        auto* codec = gdeflate_codec();
        if (!codec)
            return {};

        std::vector<uint8_t> out(codec->CompressBufferBound(static_cast<uint32_t>(size)));
        size_t compressed = 0;
        if (FAILED(codec->CompressBuffer(data, static_cast<uint32_t>(size), DSTORAGE_COMPRESSION_BEST_RATIO,
                                         out.data(), static_cast<uint32_t>(out.size()), &compressed)))
            return {};
        out.resize(compressed);
        return out;
    }

    bool gdeflate_decompress(const uint8_t* source, size_t source_size, uint8_t* dest, size_t dest_size)
    {
        auto* codec = gdeflate_codec();
        if (!codec)
            return false;

        size_t written = 0;
        return SUCCEEDED(codec->DecompressBuffer(source, source_size, dest, dest_size, &written)) && written == dest_size;
    }
}
