// The cluster LOD cut and Quanta's shared buffers, for every draw that walks them: the main view's GBuffer pass
// (gbuffer/mesh_shader.hlsl) and the shadow pages (shadows/vsm/mesh_shader_vsm.hlsl).
//
// Expects Camera, MeshletGroup and MeshletCullData to be declared (common/draw_data.hlsl) and, for the Quanta part,
// the QUANTA define. The feedback is passed as the buffer and the frame's epoch, so each draw binds it at the root
// slot that is free in its own shaders (QuantaFeedback / QuantaShadowFeedback in meshrender.prism).

#ifndef QUANTA_LOD_HLSL
#define QUANTA_LOD_HLSL

// An orthographic camera (the shadow cascades and pages) has no distance falloff: the error is the same everywhere,
// and proj[3][3] is 1 for it and 0 for a perspective one.
bool IsOrthographic(Camera camera)
{
    return abs(camera.GetProj()[3][3]) > 0.5;
}

// meshopt clusterlod's screen-space error estimate, as a fraction of the screen
// height. Must be monotonic along the DAG for the cut to be crack-free, which
// is why it ignores the view direction.
float LodProjectedError(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
    if (IsOrthographic(camera))
        return error * scale * camera.GetProj()[1][1] * 0.5;

    float3 center = mul(world, float4(sphere.xyz, 1)).xyz;
    float d = length(center - camera.GetPosition().xyz) - sphere.w * scale;
    return error * scale / max(d, 1e-4) * camera.GetProj()[1][1] * 0.5;
}

// The same estimate from the sphere's farthest point: a lower bound for every
// sphere it encloses.
float LodProjectedErrorFar(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
    if (IsOrthographic(camera))
        return error * scale * camera.GetProj()[1][1] * 0.5;

    float3 center = mul(world, float4(sphere.xyz, 1)).xyz;
    float d = length(center - camera.GetPosition().xyz) + sphere.w * scale;
    return error * scale / max(d, 1e-4) * camera.GetProj()[1][1] * 0.5;
}

float MaxScale(float4x4 world)
{
    float3 scales = float3(
        length(float3(world[0].x, world[1].x, world[2].x)),
        length(float3(world[0].y, world[1].y, world[2].y)),
        length(float3(world[0].z, world[1].z, world[2].z)));
    return max(scales.x, max(scales.y, scales.z));
}

// A cluster is in the DAG cut when its group's own simplification is still
// accurate enough (the coarser version is not good enough) and the refined
// group it came from is not needed. threshold 0 = LOD off: original clusters.

// The group half, shared by all of a group's clusters.
bool LodGroupPasses(MeshletGroup g, float4x4 world, float scale, Camera camera, float threshold)
{
    return threshold <= 0 || LodProjectedError(g.GetLodSphere(), g.GetLodError(), world, scale, camera) > threshold;
}

#ifdef QUANTA
// Quanta's shared buffers (MeshInstanceInfo::quanta_*), laid out as in Quanta.ixx / QuantaResidency.ixx: dword
// records, group.page a page id that quanta_page_slots maps to the page's dword offset in quanta_pages.
static const uint Q_NODE_DWORDS = 12;     // lod_sphere[4] refined_sphere[4] lod_error refined_error first_group group_count
static const uint Q_GROUP_DWORDS = 16;    // same, then cluster_count depth page offset_in_page(bytes) size family
static const uint Q_CLUSTER_DWORDS = 16;  // vertex_offset triangle_offset vertex_count triangle_count bounding_sphere[4] cone lod_level refined_error refined_family refined_sphere[4]
static const uint Q_GROUP_HEADER_DWORDS = 4;
static const uint Q_INVALID = 0xFFFFFFFF;

// Dword offset of a group's blob in the pages buffer, Q_INVALID while its page is not resident.
uint QuantaBlob(StructuredBuffer<uint> slots, StructuredBuffer<uint> groups, uint group)
{
    uint g = group * Q_GROUP_DWORDS;
    uint slot = slots[groups[g + 12]];
    return slot == Q_INVALID ? Q_INVALID : slot + groups[g + 13] / 4;
}

float4 QuantaLoad4(StructuredBuffer<uint> b, uint i)
{
    return float4(asfloat(b[i]), asfloat(b[i + 1]), asfloat(b[i + 2]), asfloat(b[i + 3]));
}

// The LOD tests take a MeshletGroup: a node's record fits it as is (FirstCluster / ClusterCount = its group
// range); for a group only the two spheres and errors matter.
MeshletGroup QuantaRecord(StructuredBuffer<uint> b, uint dword)
{
    MeshletGroup r;
    r.LodSphere = QuantaLoad4(b, dword);
    r.RefinedSphere = QuantaLoad4(b, dword + 4);
    r.LodError = asfloat(b[dword + 8]);
    r.RefinedError = asfloat(b[dword + 9]);
    r.FirstCluster = b[dword + 10];
    r.ClusterCount = b[dword + 11];
    return r;
}

// A family (Quanta::pack_family, global group indices) is drawn and refined all-or-nothing, so the cut
// uses it only when every group's page is resident.
bool QuantaFamilyResident(StructuredBuffer<uint> slots, StructuredBuffer<uint> groups, uint first, uint count)
{
    bool resident = true;
    for (uint i = 0; i < count; i++)
        resident = resident && slots[groups[(first + i) * Q_GROUP_DWORDS + 12]] != Q_INVALID;
    return resident;
}

// The streamer's input (QuantaFeedback in meshrender.prism): a page the cut wanted but lacked, with the
// projected error that wanted it as the priority; and the last epoch a resident page was reached.
void QuantaWantFamily(RWStructuredBuffer<uint> feedback, uint epoch, StructuredBuffer<uint> groups, uint first, uint count, float error)
{
#ifndef QUANTA_NO_FEEDBACK
    uint stamp = ((epoch & 0xFFFF) << 16) | f32tof16(clamp(error, 0.0, 60000.0));
    for (uint i = 0; i < count; i++)
        InterlockedMax(feedback[groups[(first + i) * Q_GROUP_DWORDS + 12] * 2], stamp);
#endif
}

void QuantaTouch(RWStructuredBuffer<uint> feedback, uint epoch, StructuredBuffer<uint> groups, uint group)
{
#ifndef QUANTA_NO_FEEDBACK
    InterlockedMax(feedback[groups[group * Q_GROUP_DWORDS + 12] * 2 + 1], epoch);
#endif
}

MeshletCullData QuantaClusterCull(StructuredBuffer<uint> pages, uint header)
{
    MeshletCullData c;
    c.BoundingSphere = QuantaLoad4(pages, header + 4);
    c.NormalCone = pages[header + 8];
    c.LodLevel = pages[header + 9];
    c.RefinedError = asfloat(pages[header + 10]);
    c.RefinedSphere = QuantaLoad4(pages, header + 12);
    return c;
}

// Nothing under a node can be in the cut when every group is too fine. LodCanSkip's second test (every cluster
// needs its finer children) is not used: those children may be paged out, and then the cluster is drawn instead.
bool QuantaNodeSkip(MeshletGroup node, float4x4 world, float scale, Camera camera, float threshold)
{
    return threshold > 0 && LodProjectedError(node.GetLodSphere(), node.GetLodError(), world, scale, camera) * 1.01 <= threshold;
}

// LodClusterPasses with streaming: a cluster whose finer family is needed but not resident stays in the cut
// (and asks for the family), so the surface is never left empty.
bool QuantaClusterPasses(MeshletCullData c, uint refined_family, uint group_base, StructuredBuffer<uint> slots,
    StructuredBuffer<uint> groups, RWStructuredBuffer<uint> feedback, uint epoch, float4x4 world, float scale, Camera camera, float threshold)
{
    if (c.GetRefinedError() < 0)
        return true;

    float error = LodProjectedError(c.GetRefinedSphere(), c.GetRefinedError(), world, scale, camera);
    if (threshold > 0 && error <= threshold)
        return true;

    uint first = group_base + (refined_family & 0xFFFFFF);
    uint count = (refined_family >> 24) + 1;
    if (QuantaFamilyResident(slots, groups, first, count))
        return false;

    QuantaWantFamily(feedback, epoch, groups, first, count, error);
    return true;
}
#endif

#endif
