module HAL:TiledMemoryManager;
import Core;
import d3d12;
import HAL;

// D3D12 implementation of TiledResourceManager::init_tilings().
// This function is excluded from the common HAL.TiledMemoryManager.cpp and
// lives here so that D3D12-specific tiling API calls stay in the D3D12/ folder.

namespace HAL
{
    // Phase 5.9: array-aware rewrite. The previous version indexed
    // tilings[] (and built tiles[]) by mip alone, which only happened to be
    // correct for ArraySize==1 (VoxelGI's Texture3D usage, the only
    // consumer until now) -- for a real Texture2DArray, subresource =
    // mip + arraySlice*MipLevels, and every array slice beyond the first
    // was silently never tiled/tracked at all.
    //
    // Key simplification: GetResourceTiling's per-subresource tile-shape
    // geometry (WidthInTiles/HeightInTiles/DepthInTiles) is identical for
    // every array slice at a given mip -- only WHICH heap tile backs each
    // subresource varies per-slice. So one call, requesting just array
    // slice 0's standard-mip subresources, is enough to learn the shape for
    // every slice; the array dimension is then multiplied out purely in
    // this class's own bookkeeping (tiles[]/gpu_tiles[]), not via extra API
    // calls.
    //
    // tiles[]/gpu_tiles[] are flat-indexed as (mip + slice*unpacked_mip_count)
    // -- mip varies fastest -- so existing recursive parent-mip code
    // (load_tile/zero_tile, "subres+1" walks toward the next coarser mip)
    // keeps working unmodified as long as it never crosses a slice
    // boundary (see the unpacked_mip_count-based boundary checks added to
    // load_tile/zero_tile in HAL.TiledMemoryManager.cpp).
    //
    // Packed mips (the smallest mip tail, too small to tile normally) are
    // deliberately NOT given array support here -- per-array-slice packed
    // regions are a genuinely obscure corner of the D3D12 tiled-resources
    // API, and VSM (this feature's only real-array consumer) simply clamps
    // its usable mip count to unpacked_mip_count at the call site instead
    // of ever touching packed subresources for an array resource. Single-
    // slice (array_size==1) packed-mip behavior, as already used by
    // VoxelGI, is unaffected.
    void TiledResourceManager::init_tilings()
    {
        UINT num_tiles = 1;
        D3D12_PACKED_MIP_INFO mip_info;
        D3D12_TILE_SHAPE tile_shape;
        // Mip count alone, not mip*array -- generous upper bound on any
        // sane texture's mip chain (see the array-slice-0-suffices comment
        // above for why per-slice entries aren't needed here).
        UINT num_sub_res = 32;
        D3D12_SUBRESOURCE_TILING tilings[32];

        auto desc = resource->get_desc();

        resource->get_device().get_native_device()->GetResourceTiling(
            resource->get_dx(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);

        packed_mip_count         = mip_info.NumTilesForPackedMips;
        packed_subresource_offset = mip_info.NumStandardMips;
        unpacked_mip_count        = mip_info.NumStandardMips;

        if (num_tiles > 0)
        {
            this->tile_shape = { tile_shape.WidthInTexels,
                                 tile_shape.HeightInTexels,
                                 tile_shape.DepthInTexels };

            if (desc.is_buffer())
            {
                tiles.resize(1);
                tiles[0].resize(uint3(tilings[0].WidthInTiles,
                                      tilings[0].HeightInTiles,
                                      tilings[0].DepthInTiles));
                for (uint x = 0; x < tiles[0].size().x; x++)
                {
                    tiles[0][{x, 0, 0}].pos = { x, 0, 0 };
                    tiles[0][{x, 0, 0}].subresource = 0;
                    tiles[0][{x, 0, 0}].storage_index = 0;
                }

                gpu_tiles.resize(1);
                gpu_tiles[0].resize(uint3(tilings[0].WidthInTiles,
                                           tilings[0].HeightInTiles,
                                           tilings[0].DepthInTiles));
                for (uint x = 0; x < gpu_tiles[0].size().x; x++)
                {
                    gpu_tiles[0][{x, 0, 0}].pos = { x, 0, 0 };
                    gpu_tiles[0][{x, 0, 0}].subresource = 0;
                    gpu_tiles[0][{x, 0, 0}].storage_index = 0;
                }
            }
            else
            {
                auto& tex_desc  = desc.as_texture();
                UINT array_size = std::max<UINT>(1, tex_desc.ArraySize);
                UINT mip_count  = mip_info.NumStandardMips;

                tiles.resize((size_t)mip_count * array_size);
                gpu_tiles.resize((size_t)mip_count * array_size);

                packed_tiles.pos        = { 0, 0, 0 };
                packed_tiles.subresource = mip_info.NumStandardMips;
                packed_tiles.storage_index = (UINT)tiles.size();

                for (UINT slice = 0; slice < array_size; slice++)
                {
                    for (UINT mip = 0; mip < mip_count; mip++)
                    {
                        UINT flat = mip + slice * mip_count;
                        // Real D3D12 subresource -- differs from `flat`
                        // once packed mips exist (MipLevels > mip_count) or
                        // there's more than one array slice; reuses the
                        // canonical CalcSubresource this codebase already
                        // has rather than re-deriving the stride here.
                        UINT d3d_subres = tex_desc.CalcSubresource(mip, slice, 0);

                        tiles[flat].resize(uint3(tilings[mip].WidthInTiles,
                                              tilings[mip].HeightInTiles,
                                              tilings[mip].DepthInTiles));
                        gpu_tiles[flat].resize(uint3(tilings[mip].WidthInTiles,
                                                  tilings[mip].HeightInTiles,
                                                  tilings[mip].DepthInTiles));

                        for (uint x = 0; x < tiles[flat].size().x; x++)
                            for (uint y = 0; y < tiles[flat].size().y; y++)
                                for (uint z = 0; z < tiles[flat].size().z; z++)
                                {
                                    tiles[flat][{x, y, z}].pos        = { x, y, z };
                                    tiles[flat][{x, y, z}].subresource = d3d_subres;
                                    tiles[flat][{x, y, z}].storage_index = flat;
                                    gpu_tiles[flat][{x, y, z}].pos        = { x, y, z };
                                    gpu_tiles[flat][{x, y, z}].subresource = d3d_subres;
                                    gpu_tiles[flat][{x, y, z}].storage_index = flat;
                                }
                    }
                }
            }
        }
    }
}
