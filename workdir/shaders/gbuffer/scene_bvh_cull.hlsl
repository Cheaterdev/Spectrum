// Scene BVH culling: one thread per treelet (Scene::Bvh) walks its tree against a convex volume (six planes) and
// appends the mesh parts whose world box is not outside it. The test is conservative (a box touching a plane stays),
// so it never drops a part the box gather's own test (gather_pipeline.hlsl CS_boxes) would keep.

#include "../autogen/SceneBvhCull.h"

bool BoxOutside(Frustum f, float3 box_min, float3 box_max)
{
    [unroll]
    for (int i = 0; i < 6; i++)
    {
        float4 plane = f.GetPlanes(i);
        float3 corner = float3(plane.x >= 0 ? box_max.x : box_min.x, plane.y >= 0 ? box_max.y : box_min.y, plane.z >= 0 ? box_max.z : box_min.z);
        if (dot(corner, plane.xyz) + plane.w < 0)
            return true;
    }
    return false;
}

#ifdef BUILD_FUNC_CS
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    SceneBvhCull cull = GetSceneBvhCull();
    if (dispatchID.x >= cull.GetTreelet_count())
        return;

    StructuredBuffer<uint> nodes = cull.GetNodes();
    Frustum volume = cull.GetVolume();

    // A treelet holds at most 32 leaves, split at the median: its depth is 6 and the stack never holds more than 7.
    uint stack[16];
    uint top = 0;
    stack[top++] = cull.GetTreelets()[cull.GetTreelet_first() + dispatchID.x];

    while (top > 0)
    {
        uint base = stack[--top] * 8;
        float3 box_min = asfloat(uint3(nodes[base], nodes[base + 1], nodes[base + 2]));
        float3 box_max = asfloat(uint3(nodes[base + 4], nodes[base + 5], nodes[base + 6]));
        if (BoxOutside(volume, box_min, box_max))
            continue;

        uint a = nodes[base + 3];
        uint b = nodes[base + 7];
        if (b != 0)
        {
            stack[top++] = a;
            stack[top++] = b;
            continue;
        }

        uint index;
        InterlockedAdd(cull.GetCount()[0], 1, index);
        cull.GetIds()[index] = a;

        // Cleared to zero per frame; every appender stores the same y and z.
        InterlockedMax(cull.GetArgs()[0].counts.x, (index + 64) / 64);
        cull.GetArgs()[0].counts.y = 1;
        cull.GetArgs()[0].counts.z = 1;
    }
}
#endif
