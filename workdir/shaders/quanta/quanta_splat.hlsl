#include "../autogen/FrameInfo.h"
#include "../autogen/QuantaSplatData.h"

// Debug view of Quanta's LOD cut (see quanta.prism). One thread group walks one hierarchy node; each lane
// takes one of the node's groups, applies the same LOD tests the mesh shader's cluster-LOD walk does
// (gbuffer/mesh_shader.hlsl: LodCanSkip / LodGroupPasses / LodClusterPasses), and splats the vertices of
// the clusters that are in the cut.

// Record layouts in dwords; must match Quanta.ixx (static_asserts there fix the sizes). group.page is a page
// id; page_slots maps it to the page's dword offset in `pages`, or 0xFFFFFFFF while it is not resident.
static const uint NODE_DWORDS = 12;      // lod_sphere[4] refined_sphere[4] lod_error refined_error first_group group_count
static const uint GROUP_DWORDS = 16;     // lod_sphere[4] refined_sphere[4] lod_error refined_error cluster_count depth page offset_in_page(bytes) size family
static const uint CLUSTER_DWORDS = 16;   // vertex_offset triangle_offset vertex_count triangle_count bounding_sphere[4] cone lod_level refined_error refined_family refined_sphere[4]
static const uint GROUP_HEADER_DWORDS = 4;
static const uint VERTEX_DWORDS = 12;    // position[3] normal[3] uv[2] tangent[4]

float4 Load4(StructuredBuffer<uint> b, uint i)
{
    return float4(asfloat(b[i]), asfloat(b[i + 1]), asfloat(b[i + 2]), asfloat(b[i + 3]));
}

float MaxScale(float4x4 world)
{
    float3 scales = float3(
        length(float3(world[0].x, world[1].x, world[2].x)),
        length(float3(world[0].y, world[1].y, world[2].y)),
        length(float3(world[0].z, world[1].z, world[2].z)));
    return max(scales.x, max(scales.y, scales.z));
}

// meshopt clusterlod's screen-space error estimate, as a fraction of the screen height.
float LodProjectedError(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
    float3 center = mul(world, float4(sphere.xyz, 1)).xyz;
    float d = length(center - camera.GetPosition().xyz) - sphere.w * scale;
    return error * scale / max(d, 1e-4) * camera.GetProj()[1][1] * 0.5;
}

// The same from the sphere's farthest point: a lower bound for every sphere it encloses.
float LodProjectedErrorFar(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
    float3 center = mul(world, float4(sphere.xyz, 1)).xyz;
    float d = length(center - camera.GetPosition().xyz) + sphere.w * scale;
    return error * scale / max(d, 1e-4) * camera.GetProj()[1][1] * 0.5;
}

float3 LevelColor(uint level)
{
    float h = frac(level * 0.61803398 + 0.1);
    return 0.5 + 0.5 * cos(6.2831853 * (h + float3(0.0, 0.33, 0.67)));
}

[numthreads(32, 1, 1)]
void CS(uint gtid : SV_GroupThreadID, uint gid : SV_GroupID)
{
    const QuantaSplatData data = GetQuantaSplatData();
    const FrameInfo frame = GetFrameInfo();
    Camera camera = frame.GetCamera();
    float threshold = frame.GetLodThreshold();
    float4x4 world = data.GetWorld();
    float scale = MaxScale(world);

    if (gid >= data.GetNode_count())
        return;

    StructuredBuffer<uint> nodes = data.GetNodes();
    StructuredBuffer<uint> groups = data.GetGroups();
    StructuredBuffer<uint> pages = data.GetPages();

    uint n = (data.GetNode_offset() + gid) * NODE_DWORDS;
    uint first_group = nodes[n + 10];
    uint group_count = nodes[n + 11];
    if (gtid >= group_count)
        return;

    uint g = (first_group + gtid) * GROUP_DWORDS;
    float4 lod_sphere = Load4(groups, g);
    float4 refined_sphere = Load4(groups, g + 4);
    float lod_error = asfloat(groups[g + 8]);
    float refined_error = asfloat(groups[g + 9]);

    // The group half of the cut: its own simplification must be too coarse to stop at, or LOD is off.
    bool passes = threshold <= 0 || LodProjectedError(lod_sphere, lod_error, world, scale, camera) > threshold;
    if (!passes)
        return;


    // A family is used all-or-nothing, as the mesh shader's walk does (gbuffer/mesh_shader.hlsl).
    StructuredBuffer<uint> slots = data.GetPage_slots();
    uint family = groups[g + 15];
    for (uint f = 0; f < (family >> 24) + 1; f++)
        if (slots[groups[((family & 0xFFFFFF) + f) * GROUP_DWORDS + 12]] == 0xFFFFFFFF)
            return;

    uint cluster_count = groups[g + 10];
    uint blob = slots[groups[g + 12]] + groups[g + 13] / 4;

    uint width, height;
    data.GetTarget().GetDimensions(width, height);
    uint depth_w, depth_h;
    data.GetDepth().GetDimensions(depth_w, depth_h);

    for (uint c = 0; c < cluster_count; c++)
    {
        uint h = blob + GROUP_HEADER_DWORDS + c * CLUSTER_DWORDS;

        float cluster_error = asfloat(pages[h + 10]);
        bool in_cut = cluster_error < 0
            || (threshold > 0 && LodProjectedError(Load4(pages, h + 12), cluster_error, world, scale, camera) <= threshold);
        if (!in_cut)
        {
            // The finer family it was simplified from is not there yet: the cluster stays.
            uint refined = pages[h + 11];
            uint refined_first = data.GetGroup_offset() + (refined & 0xFFFFFF);
            bool finer_resident = true;
            for (uint r = 0; r < (refined >> 24) + 1; r++)
                finer_resident = finer_resident && slots[groups[(refined_first + r) * GROUP_DWORDS + 12]] != 0xFFFFFFFF;
            in_cut = !finer_resident;
        }
        if (!in_cut)
            continue;

        uint vertex_start = blob + pages[h] / 4;
        uint vertex_count = pages[h + 2];
        float4 color = float4(LevelColor(pages[h + 9]), 1);

        for (uint v = 0; v < vertex_count; v++)
        {
            uint p = vertex_start + v * VERTEX_DWORDS;
            float3 local = float3(asfloat(pages[p]), asfloat(pages[p + 1]), asfloat(pages[p + 2]));
            float4 clip = mul(camera.GetViewProj(), mul(world, float4(local, 1)));
            if (clip.w <= 0.0)
                continue;

            float3 ndc = clip.xyz / clip.w;
            if (any(abs(ndc.xy) > 1.0) || ndc.z < 0.0 || ndc.z > 1.0)
                continue;

            float2 uv = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);
            int2 depth_pixel = int2(uv * float2(depth_w, depth_h));
            if (depth_pixel.x < 0 || depth_pixel.y < 0 || (uint)depth_pixel.x >= depth_w || (uint)depth_pixel.y >= depth_h)
                continue;

            // Reversed-Z: a larger value is closer. The small bias keeps points that sit exactly on a
            // surface from flickering against it.
            if (ndc.z + 0.0005 < data.GetDepth()[depth_pixel])
                continue;

            int2 pixel = int2(uv * float2(width, height));
            for (int y = -2; y <= 2; y++)
                for (int x = -2; x <= 2; x++)
                {
                    int2 q = pixel + int2(x, y);
                    if (q.x >= 0 && q.y >= 0 && (uint)q.x < width && (uint)q.y < height)
                        data.GetTarget()[q] = color;
                }
        }
    }
}
