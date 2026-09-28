module HAL:TiledMemoryManager;
import Core;
import HAL;

// Vulkan implementation of TiledResourceManager::init_tilings(): the same tile
// grid the D3D12 version builds from GetResourceTiling, derived from the
// resource's sparse layout (see API::Resource::init_sparse_layout).
//
// Vulkan's mip tail is D3D12's packed mips: the mips from mip_tail_first on,
// bound as one opaque range. As on D3D12, array resources get no per-slice
// tail support (VSM clamps its mip count to unpacked_mip_count).

namespace HAL
{
    void TiledResourceManager::init_tilings()
    {
        auto desc = resource->get_desc();
        const auto& layout = resource->get_sparse_layout();
        constexpr auto tile_size = API::Device::SPARSE_TILE_SIZE;

        auto fill = [](grid<uint3, ResourceTile>& g, uint3 size, UINT subresource, UINT storage_index) {
            g.resize(size);
            for (uint x = 0; x < size.x; x++)
                for (uint y = 0; y < size.y; y++)
                    for (uint z = 0; z < size.z; z++)
                    {
                        auto& tile = g[{x, y, z}];
                        tile.pos           = { x, y, z };
                        tile.subresource   = subresource;
                        tile.storage_index = storage_index;
                    }
        };

        if (desc.is_buffer())
        {
            const uint num_tiles = static_cast<uint>(layout.size / tile_size);

            tile_shape                = { static_cast<uint>(tile_size), 1, 1 };
            packed_mip_count          = 0;
            unpacked_mip_count        = 0;
            packed_subresource_offset = 0;

            tiles.resize(1);
            gpu_tiles.resize(1);
            fill(tiles[0], { num_tiles, 1, 1 }, 0, 0);
            fill(gpu_tiles[0], { num_tiles, 1, 1 }, 0, 0);
            return;
        }

        auto& tex = desc.as_texture();
        const UINT array_size = std::max<UINT>(1, tex.ArraySize);
        const UINT mip_count  = std::min<UINT>(layout.mip_tail_first, tex.MipLevels);

        tile_shape = { layout.granularity.width, layout.granularity.height, layout.granularity.depth };

        packed_mip_count          = static_cast<UINT>((layout.mip_tail_size + tile_size - 1) / tile_size);
        packed_subresource_offset = mip_count;
        unpacked_mip_count        = mip_count;

        tiles.resize((size_t)mip_count * array_size);
        gpu_tiles.resize((size_t)mip_count * array_size);

        packed_tiles.pos           = { 0, 0, 0 };
        packed_tiles.subresource   = mip_count;
        packed_tiles.storage_index = (UINT)tiles.size();

        for (UINT slice = 0; slice < array_size; slice++)
            for (UINT mip = 0; mip < mip_count; mip++)
            {
                const uint3 mip_dim = uint3::max({ 1, 1, 1 }, uint3(
                    tex.Dimensions.x >> mip,
                    tex.is1D() ? 1u : tex.Dimensions.y >> mip,
                    tex.is3D() ? tex.Dimensions.z >> mip : 1u));

                const uint3 tile_count = {
                    (mip_dim.x + tile_shape.x - 1) / tile_shape.x,
                    (mip_dim.y + tile_shape.y - 1) / tile_shape.y,
                    (mip_dim.z + tile_shape.z - 1) / tile_shape.z };

                // Same subresource numbering as D3D12 (mip + slice * MipLevels);
                // Queue::update_tile_mappings decodes it back to mip and layer.
                const UINT flat   = mip + slice * mip_count;
                const UINT subres = tex.CalcSubresource(mip, slice, 0);
                fill(tiles[flat], tile_count, subres, flat);
                fill(gpu_tiles[flat], tile_count, subres, flat);
            }
    }
}
