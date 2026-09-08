// Fused GBuffer half-res downsample + generic 8x8-tile Hi/Low classification.
// See TileClassifyData's own comment (pssm.sig) for the algorithm summary.
//
// One 8x8 thread group per full-res screen tile (= one 4x4 patch of the
// half-res output). Everything is derived from a single cooperative load of
// the tile's depth+normal into LDS: the half-res "real" sample per 2x2 block
// (closest wins, same rule the old per-mip PS downsample used -- never an
// average, so nothing downstream infers geometry that isn't really there),
// the tile's own Hi/Low bucket, and the per-pixel mask for Hi tiles.
//
// Deliberately raw-depth (reversed-Z, [0,1], nonlinear), not linearized view
// depth: this matches exactly what the PS this replaces did (Gather + pick
// max, no camera involved), stays consistent with the rest of the tile's
// comparisons, and sidesteps the raw_z==0 (sky) unprojection singularity
// entirely -- a tile straddling the sky/geometry silhouette naturally comes
// out Hi, which is the correct answer for that edge.
#include "../autogen/TileClassifyData.h"

static const TileClassifyData params = GetTileClassifyData();
static const GBuffer gbuffer = params.GetGbuffer();

// Tile classified Hi if the spread between the tile's 16 block
// representatives' raw depth exceeds this. Per-pixel mask (inside Hi tiles
// only, in principle) uses a tighter threshold since it is judging a single
// pixel against its own 2x2 block, not a whole 8x8 tile.
#define TILE_DEPTH_EDGE_THRESHOLD  0.02
#define PIXEL_DEPTH_EDGE_THRESHOLD 0.005

groupshared float  g_depth[64];
groupshared float4 g_normal[64];   // xyz = decoded normal, w = roughness
groupshared float  g_metallic[64];
groupshared float  g_blockDepth[16];
groupshared float  g_blockRoughness[16];
groupshared float  g_blockMetallic[16];

[numthreads(8, 8, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex)
{
    uint2 dims;
    gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

    uint2 pix = min(dispatchID.xy, dims - 1);

    float  depth      = gbuffer.GetDepth()[pix];
    float4 normal_raw = gbuffer.GetNormals()[pix];
    // .w = metallic (universal_material.hlsl: result.albedo = float4(albedo.rgb, metallic)).
    float  metallic   = gbuffer.GetAlbedo()[pix].w;

    g_depth[groupIndex]    = depth;
    g_normal[groupIndex]   = float4(normalize(normal_raw.xyz * 2 - 1), normal_raw.w);
    g_metallic[groupIndex] = metallic;

    GroupMemoryBarrierWithGroupSync();

    // One leader thread per 2x2 block (16 per 8x8 tile) picks the closest
    // real sample and writes the half-res output + this block's entry in
    // the tile-classification LDS.
    uint2 half_dims = (dims + 1) / 2;
    uint2 block     = groupThreadID.xy / 2;

    if ((groupThreadID.x & 1) == 0 && (groupThreadID.y & 1) == 0)
    {
        uint i00 = groupIndex;
        uint i10 = groupIndex + 1;
        uint i01 = groupIndex + 8;
        uint i11 = groupIndex + 9;

        float  d   = g_depth[i00];
        float4 n   = g_normal[i00];
        float  m   = g_metallic[i00];

        // reversed-Z: closest surface = max depth value. Metallic rides
        // along with whichever sample wins, same "real sample, never an
        // average" rule as depth/normal.
        if (g_depth[i10] > d) { d = g_depth[i10]; n = g_normal[i10]; m = g_metallic[i10]; }
        if (g_depth[i01] > d) { d = g_depth[i01]; n = g_normal[i01]; m = g_metallic[i01]; }
        if (g_depth[i11] > d) { d = g_depth[i11]; n = g_normal[i11]; m = g_metallic[i11]; }

        uint blockIdx = block.y * 4 + block.x;
        g_blockDepth[blockIdx]      = d;
        g_blockRoughness[blockIdx]  = n.w;
        g_blockMetallic[blockIdx]   = m;

        uint2 half_pix = groupID.xy * 4 + block;
        if (all(half_pix < half_dims))
        {
            params.GetHalf_depth()[half_pix]   = d;
            params.GetHalf_normals()[half_pix] = float4(n.xyz * 0.5 + 0.5, n.w);
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // Per-pixel mask: this pixel vs. its own 2x2 block's chosen representative.
    float block_depth = g_blockDepth[block.y * 4 + block.x];
    params.GetTile_mask()[pix] = (abs(depth - block_depth) > PIXEL_DEPTH_EDGE_THRESHOLD) ? 1 : 0;

    // Tile classification: one thread reduces the 16 blocks.
    if (groupIndex == 0)
    {
        float mind = g_blockDepth[0];
        float maxd = g_blockDepth[0];
        float min_roughness = g_blockRoughness[0];
        float max_metallic  = g_blockMetallic[0];

        [unroll]
        for (uint i = 1; i < 16; i++)
        {
            mind = min(mind, g_blockDepth[i]);
            maxd = max(maxd, g_blockDepth[i]);
            min_roughness = min(min_roughness, g_blockRoughness[i]);
            max_metallic  = max(max_metallic,  g_blockMetallic[i]);
        }

        uint hi = (maxd - mind > TILE_DEPTH_EDGE_THRESHOLD) ? 1 : 0;
        if (hi)
            params.GetTile_hi().Append(groupID.xy);
        else
            params.GetTile_low().Append(groupID.xy);

        // Point-queryable form of the same verdict, for a consumer that
        // just wants "is my tile Hi" without indirect-dispatch machinery
        // (IndirectRTX's raygen -- see its own comment, voxel.sig).
        params.GetTile_flags()[groupID.xy] = hi;

        // Second, independent axis: worth a full-res specular trace only if
        // some pixel is both glossy enough to show detail AND metallic
        // enough for that detail to survive the downstream multiply (see
        // TileClassifyData's own comment, pssm.sig).
        uint roughness_hi = (min_roughness < params.GetRoughness_threshold() &&
                              max_metallic  > params.GetMetallic_threshold()) ? 1 : 0;
        if (roughness_hi)
            params.GetTile_roughness_hi().Append(groupID.xy);
        else
            params.GetTile_roughness_low().Append(groupID.xy);

        params.GetTile_roughness_flags()[groupID.xy] = roughness_hi;
    }
}
