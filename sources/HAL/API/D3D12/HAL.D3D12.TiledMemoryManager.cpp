module HAL:TiledMemoryManager;
import Core;
import d3d12;
import HAL;

// D3D12 implementation of TiledResourceManager::init_tilings().
// This function is excluded from the common HAL.TiledMemoryManager.cpp and
// lives here so that D3D12-specific tiling API calls stay in the D3D12/ folder.

namespace HAL
{
    void TiledResourceManager::init_tilings()
    {
        UINT num_tiles = 1;
        D3D12_PACKED_MIP_INFO mip_info;
        D3D12_TILE_SHAPE tile_shape;
        UINT num_sub_res = 20;
        D3D12_SUBRESOURCE_TILING tilings[20];

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
                    tiles[0][{x, 0, 0}].pos = { x, 0, 0 };

                gpu_tiles.resize(1);
                gpu_tiles[0].resize(uint3(tilings[0].WidthInTiles,
                                           tilings[0].HeightInTiles,
                                           tilings[0].DepthInTiles));
                for (uint x = 0; x < gpu_tiles[0].size().x; x++)
                    gpu_tiles[0][{x, 0, 0}].pos = { x, 0, 0 };
            }
            else
            {
                tiles.resize(mip_info.NumStandardMips);
                gpu_tiles.resize(mip_info.NumStandardMips);

                packed_tiles.pos        = { 0, 0, 0 };
                packed_tiles.subresource = mip_info.NumStandardMips;

                for (UINT i = 0; i < mip_info.NumStandardMips; i++)
                {
                    tiles[i].resize(uint3(tilings[i].WidthInTiles,
                                          tilings[i].HeightInTiles,
                                          tilings[i].DepthInTiles));
                    gpu_tiles[i].resize(uint3(tilings[i].WidthInTiles,
                                              tilings[i].HeightInTiles,
                                              tilings[i].DepthInTiles));

                    for (uint x = 0; x < tiles[i].size().x; x++)
                        for (uint y = 0; y < tiles[i].size().y; y++)
                            for (uint z = 0; z < tiles[i].size().z; z++)
                            {
                                tiles[i][{x, y, z}].pos        = { x, y, z };
                                tiles[i][{x, y, z}].subresource = i;
                                gpu_tiles[i][{x, y, z}].pos        = { x, y, z };
                                gpu_tiles[i][{x, y, z}].subresource = i;
                            }
                }
            }
        }
    }
}
