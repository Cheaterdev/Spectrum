// The GDeflate codec on Android: Microsoft's CPU reference (vcpkg port `gdeflate`, custom-overlay/gdeflate), which reads
// and writes the tile stream DirectStorage's codec writes on Windows. A plain (non-module) translation unit, like the
// Windows one in ../Windows; HAL.Device.ixx declares the three functions extern "C++".
#include <gdeflate/libdeflate.h>
#include <gdeflate/TileStream.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

namespace HAL
{
    namespace
    {
        constexpr size_t tile_size = GDeflate::kDefaultTileSize;

        // DSTORAGE_COMPRESSION_BEST_RATIO, which the Windows codec is called with, is libdeflate's highest level.
        constexpr int best_ratio_level = 12;

        struct CompressorDelete
        {
            void operator()(libdeflate_gdeflate_compressor* c) const { libdeflate_free_gdeflate_compressor(c); }
        };
        struct DecompressorDelete
        {
            void operator()(libdeflate_gdeflate_decompressor* d) const { libdeflate_free_gdeflate_decompressor(d); }
        };

        // A decompressor holds the Huffman tables of the tile being decoded, so it cannot be shared between threads;
        // one per pool thread, reused for every page that thread expands.
        libdeflate_gdeflate_decompressor* thread_decompressor()
        {
            static thread_local std::unique_ptr<libdeflate_gdeflate_decompressor, DecompressorDelete> d(
                libdeflate_alloc_gdeflate_decompressor());
            return d.get();
        }

        // Allocating a compressor fills a table shared by all of them (deflate_length_slot in deflate_compress.c)
        // behind a plain, non-atomic flag. Two threads allocating at once race on it, and on ARM the second one can
        // see the flag set and still read zeros from the table. The first allocation happens here, once, so every
        // later one only reads it.
        std::unique_ptr<libdeflate_gdeflate_compressor, CompressorDelete> make_compressor()
        {
            static std::once_flag tables_ready;
            std::call_once(tables_ready, [] { libdeflate_free_gdeflate_compressor(libdeflate_alloc_gdeflate_compressor(best_ratio_level)); });
            return std::unique_ptr<libdeflate_gdeflate_compressor, CompressorDelete>(
                libdeflate_alloc_gdeflate_compressor(best_ratio_level));
        }
    }

    bool gdeflate_supported() { return true; }

    // The stream, as GDeflate::Compress in the reference writes it: the 8-byte TileStream header, one uint32 per tile,
    // then the tiles' compressed bytes back to back. Entry i > 0 of the table is the offset of tile i in that data;
    // entry 0 (tile 0 always starts at 0) holds the compressed size of the last tile instead.
    std::vector<uint8_t> gdeflate_compress(const uint8_t* data, size_t size)
    {
        if (!data || size == 0 || size > tile_size * GDeflate::TileStream::kMaxTiles)
            return {};

        auto compressor = make_compressor();
        if (!compressor)
            return {};

        const size_t tiles = (size + tile_size - 1) / tile_size;
        size_t pages = 0;
        std::vector<uint8_t> scratch(libdeflate_gdeflate_compress_bound(compressor.get(), tile_size, &pages));

        const GDeflate::TileStream header(size);
        const size_t data_start = sizeof(header) + tiles * sizeof(uint32_t);
        std::vector<uint32_t> table(tiles);
        std::vector<uint8_t> out(data_start);

        for (size_t i = 0; i < tiles; i++)
        {
            const size_t in_size = std::min(tile_size, size - i * tile_size);
            libdeflate_gdeflate_out_page page{ scratch.data(), scratch.size() };
            if (libdeflate_gdeflate_compress(compressor.get(), data + i * tile_size, in_size, &page, 1) == 0)
                return {};

            table[i] = static_cast<uint32_t>(out.size() - data_start);
            out.insert(out.end(), scratch.data(), scratch.data() + page.nbytes);
            if (i + 1 == tiles)
                table[0] = static_cast<uint32_t>(page.nbytes);
        }

        std::memcpy(out.data(), &header, sizeof(header));
        std::memcpy(out.data() + sizeof(header), table.data(), tiles * sizeof(uint32_t));
        return out;
    }

    bool gdeflate_decompress(const uint8_t* source, size_t source_size, uint8_t* dest, size_t dest_size)
    {
        if (!source || !dest || dest_size == 0 || source_size < sizeof(GDeflate::TileStream))
            return false;

        // memcpy, not a cast: a page's bytes sit at any offset inside the asset blob.
        GDeflate::TileStream header(0);
        std::memcpy(&header, source, sizeof(header));
        if (!header.IsValid() || header.id != GDeflate::kGDeflateId || header.tileSizeIdx != 1 || header.numTiles == 0)
            return false;
        if (header.GetUncompressedSize() != dest_size)
            return false;

        const size_t tiles = header.numTiles;
        const size_t data_start = sizeof(header) + tiles * sizeof(uint32_t);
        if (source_size < data_start)
            return false;
        std::vector<uint32_t> table(tiles);
        std::memcpy(table.data(), source + sizeof(header), tiles * sizeof(uint32_t));

        const uint8_t* tile_data = source + data_start;
        const size_t tile_data_size = source_size - data_start;

        libdeflate_gdeflate_decompressor* decompressor = thread_decompressor();
        if (!decompressor)
            return false;

        for (size_t i = 0; i < tiles; i++)
        {
            const bool last = i + 1 == tiles;
            const size_t begin = i == 0 ? 0 : table[i];
            const size_t end = last ? begin + table[0] : table[i + 1];
            if (end < begin || end > tile_data_size)
                return false;

            libdeflate_gdeflate_in_page page{ tile_data + begin, end - begin };
            const size_t out_size = last ? dest_size - i * tile_size : tile_size;
            // A tile that would need more than out_size fails with LIBDEFLATE_INSUFFICIENT_SPACE, but one that ends
            // short succeeds: libdeflate_gdeflate_decompress() always asks its decoder for the size written, even
            // when given no pointer, so LIBDEFLATE_SHORT_OUTPUT never comes back. Hence the size check.
            size_t produced = 0;
            if (libdeflate_gdeflate_decompress(decompressor, &page, 1, dest + i * tile_size, out_size, &produced) !=
                    LIBDEFLATE_SUCCESS ||
                produced != out_size)
                return false;
        }
        return true;
    }
}
