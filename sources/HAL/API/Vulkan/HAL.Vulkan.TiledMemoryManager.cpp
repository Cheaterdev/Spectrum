module HAL:TiledMemoryManager;
import Core;
import HAL;

// Vulkan stub for TiledResourceManager::init_tilings().
//
// Real Vulkan sparse resources (VK_KHR_sparse_resources) are a post-Phase-0
// feature.  However, we must populate the CPU-side `tiles` grid so that the
// common streaming code (load_tile, zero_tile, etc.) doesn't crash on
// out-of-bounds access — it always indexes tiles[subres][pos] without
// re-checking is_tiled() first.
//
// The tile mappings are never sent to the GPU (Queue::update_tile_mappings
// is a no-op), so RESERVED resources in Vulkan are simply invisible to
// shaders.  That is acceptable for Phase 0.

namespace HAL
{
    static constexpr uint TILE_SIZE = 64 * 1024; // 64 KB — standard Vulkan sparse tile

    void TiledResourceManager::init_tilings()
    {
        auto desc = resource->get_desc();

        if (desc.is_buffer())
        {
            // Buffers: a flat 1-D array of 64 KB tiles.
            const size_t bytes     = desc.as_buffer().SizeInBytes;
            const uint   num_tiles = std::max(1u, static_cast<uint>(
                                         (bytes + TILE_SIZE - 1) / TILE_SIZE));

            tile_shape              = { TILE_SIZE, 1, 1 };
            packed_mip_count        = 0;
            unpacked_mip_count      = 0;
            packed_subresource_offset = 0;

            tiles.resize(1);
            tiles[0].resize(uint3{ num_tiles, 1, 1 });
            for (uint x = 0; x < num_tiles; ++x)
                tiles[0][{x, 0, 0}].pos = { x, 0, 0 };

            gpu_tiles.resize(1);
            gpu_tiles[0].resize(uint3{ num_tiles, 1, 1 });
            for (uint x = 0; x < num_tiles; ++x)
                gpu_tiles[0][{x, 0, 0}].pos = { x, 0, 0 };
        }
        else if (desc.is_texture())
        {
            auto& tex = desc.as_texture();

            // Simple tile shape: 128 x 128 texels (works for most formats at 32 bpp).
            // Vulkan sparse block sizes are format-dependent; this approximation is
            // sufficient to keep the CPU tile grid alive without crashing.
            const uint tile_w = 128u;
            const uint tile_h = 128u;
            const uint tile_d = 1u;

            tile_shape = { tile_w, tile_h, tile_d };

            // Standard mip layout: all mips get a tile grid; no packed mips stub.
            const uint mips = std::max(1u, tex.MipLevels);
            packed_mip_count        = 0;
            unpacked_mip_count      = mips;
            packed_subresource_offset = mips;

            tiles.resize(mips);
            gpu_tiles.resize(mips);

            for (uint m = 0; m < mips; ++m)
            {
                uint3 mip_dim = uint3::max({ 1, 1, 1 },
                    tex.Dimensions / (uint)std::pow(2, m));

                uint xt = std::max(1u, (mip_dim.x + tile_w - 1) / tile_w);
                uint yt = std::max(1u, (mip_dim.y + tile_h - 1) / tile_h);
                uint zt = std::max(1u, tex.is3D()
                              ? (mip_dim.z + tile_d - 1) / tile_d : 1u);

                tiles[m].resize(uint3(xt, yt, zt));
                gpu_tiles[m].resize(uint3(xt, yt, zt));

                for (uint x = 0; x < xt; ++x)
                    for (uint y = 0; y < yt; ++y)
                        for (uint z = 0; z < zt; ++z)
                        {
                            tiles[m][{x,y,z}].pos        = { x, y, z };
                            tiles[m][{x,y,z}].subresource = m;
                            gpu_tiles[m][{x,y,z}].pos        = { x, y, z };
                            gpu_tiles[m][{x,y,z}].subresource = m;
                        }
            }
        }
    }
}
