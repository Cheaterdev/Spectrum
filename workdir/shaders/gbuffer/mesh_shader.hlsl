
struct vertex_output
{
    float4 pos : SV_POSITION;
    float3 wpos : POSITION;
    float3 normal : NORMAL;
    float3 binormal : BINORMAL;
    float3 tangent : TANGENT;
    float2 tc : TEXCOORD;
    float4 cur_pos : CUR_POSITION;

    float4 prev_pos : PREV_POSITION;
    float dist : DISTANCE;

};

#include "../common/common.hlsl"

#include "../autogen/FrameInfo.h"
#include "../autogen/MeshInfo.h" 
#include "../autogen/SceneData.h"
#include "../autogen/MeshInstanceInfo.h" 

static const FrameInfo frameInfo = GetFrameInfo();
static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();
static const MeshInstanceInfo meshInstanceInfo = GetMeshInstanceInfo();

//#include "../autogen/DebugInfo.h"
vertex_output transform(matrix node_global_matrix, matrix node_global_matrix_prev, Camera camera, mesh_vertex_input i)
{
    vertex_output o;
    float4 tpos = mul(node_global_matrix, float4(i.pos, 1));
    o.wpos.xyz = tpos.xyz / tpos.w;
    o.pos = mul(frameInfo.GetCamera().GetViewProj(), tpos);
    o.normal = normalize(mul((float3x3)node_global_matrix, i.normal));
    o.tangent = normalize(mul((float3x3) node_global_matrix, i.tangent.xyz) * i.tangent.w);
    o.binormal = normalize(cross(i.normal, i.tangent.xyz) * i.tangent.w);
    o.tc = i.tc;
    float3 v = o.wpos.xyz - frameInfo.GetCamera().GetPosition();
    float dist = length(v);
    o.dist = (1 - clamp(dist, 10, 40) / 40);
    o.dist *= 1 + pow(1 - abs(dot(o.normal, v / dist)), 4);
    o.dist = clamp(o.dist, 0, 1);
    o.cur_pos = o.pos;

    float4 ppos = mul(node_global_matrix_prev, float4(i.pos, 1));

    o.prev_pos = mul(frameInfo.GetPrevCamera().GetViewProj(), ppos);
    return o;
}

// A meshlet's cull result, as the CAPTURE_MESHLETS permutation records it
// ((frame << 4) | status, see MeshletCaptureWrite in meshrender.prism).
static const uint MESHLET_CULLED_FRUSTUM  = 0;
static const uint MESHLET_CULLED_BACKFACE = 1;
static const uint MESHLET_CULLED_HIZ      = 2;
static const uint MESHLET_DRAWN           = 3;
// Not in the cluster LOD cut: a coarser or finer cluster covers it.
static const uint MESHLET_LOD_REJECTED    = 4;
// Debug view only: no recorded result for this part.
static const uint MESHLET_NOT_CAPTURED    = 15;

#ifdef CAPTURE_MESHLETS
#include "../autogen/MeshletCaptureWrite.h"
#endif

#ifdef DEBUG_VIEW
#include "../autogen/DebugViewDraw.h"

// Read by debug/debug_view_mesh.hlsl, which declares the same struct.
struct primitive_output
{
    uint meshlet : MESHLET_ID;
    uint status : MESHLET_STATUS;
    uint lod_level : MESHLET_LOD;
};
#endif

#if defined(DEBUG_VIEW) && !defined(CLUSTER_LOD)
#define CLUSTER_LOD
#endif

// LOD hierarchy shape; must match MeshletGeneration.h's MESHLET_LOD_*.
static const uint LOD_GROUP_CLUSTERS = 16;
static const uint LOD_NODE_GROUPS = 32;
// One ClusterLod AS group walks one node: at most this many clusters.
static const uint PAYLOAD_CLUSTERS = LOD_GROUP_CLUSTERS * LOD_NODE_GROUPS;

// Sized for the LOD walk even in the flat permutations (which fill 32): the
// ClusterLod define is AS-only, and the MS must declare the same payload.
struct Payload
{
    uint MeshletIndices[PAYLOAD_CLUSTERS];
#ifdef DEBUG_VIEW
    // status | lod level << 4
    uint MeshletInfo[PAYLOAD_CLUSTERS];
#endif
};



// snorm8 x4 (meshopt's cone_axis_s8 / cone_cutoff_s8).
float4 UnpackCone(uint packed)
{
    int4 s = int4(packed << 24, packed << 16, packed << 8, packed) >> 24;
    return float4(s) / 127.0;
}

float dist(float4 plane, float3 pt)
{
    return dot(pt, plane.xyz) + plane.w;
}



// MESHLET_DRAWN, or which of the frustum/backface-cone tests rejected it.
uint MeshletCull(MeshletCullData c, float4x4 world, Camera camera)
{
    float4 BoundingSphere = c.GetBoundingSphere();

    // World-space lengths of the local basis vectors: the engine uses the
    // column-vector convention (mul(world, v)), so the basis vectors are the
    // matrix COLUMNS. Max component = conservative uniform scale for the
    // sphere radius / apex offset (was hardcoded 1, breaking scaled imports).
    float3 scales = float3(
        length(float3(world[0].x, world[1].x, world[2].x)),
        length(float3(world[0].y, world[1].y, world[2].y)),
        length(float3(world[0].z, world[1].z, world[2].z)));
    float scale = max(scales.x, max(scales.y, scales.z));

    // Do a cull test of the bounding sphere against the view frustum planes.
    float4 center = mul(world, float4(BoundingSphere.xyz, 1));
    center.xyz /= center.w;
    float radius = BoundingSphere.w * scale;

    [unroll]
    for (int i = 0; i < 6; ++i)
    {

        float d = dist(camera.GetFrustum().GetPlanes(i), center.xyz);

        if (d < -radius)
        {
           return MESHLET_CULLED_FRUSTUM;
        }
    }

    float4 cone = UnpackCone(c.GetNormalCone());

    // Normals spread wider than the cone can bound.
    if (cone.w >= 1)
        return MESHLET_DRAWN;

    // Under strongly non-uniform scale the cone axis would need the
    // inverse-transpose — be conservative and skip the cone test.
    if (max(scales.x, max(scales.y, scales.z)) > 1.05 * min(scales.x, min(scales.y, scales.z)))
        return MESHLET_DRAWN;

    // meshopt's apex-free test for the 8-bit cone. The axis is divided by the
    // scale rather than normalized: the s8 cutoff padding assumes the decoded
    // axis as-is. Engine convention is mul(matrix, vector).
    float3 axis = mul(world, float4(cone.xyz, 0)).xyz / scale;
    float3 view = center.xyz - camera.GetPosition();
    if (dot(view, axis) >= cone.w * length(view) + radius)
        return MESHLET_CULLED_BACKFACE;

    // All tests passed - it will merit pixels
    return MESHLET_DRAWN;
}

bool IsVisible(MeshletCullData c, float4x4 world, Camera camera)
{
    return MeshletCull(c, world, camera) == MESHLET_DRAWN;
}

// meshopt clusterlod's screen-space error estimate, as a fraction of the screen
// height. Must be monotonic along the DAG for the cut to be crack-free, which
// is why it ignores the view direction.
float LodProjectedError(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
    float3 center = mul(world, float4(sphere.xyz, 1)).xyz;
    float d = length(center - camera.GetPosition().xyz) - sphere.w * scale;
    return error * scale / max(d, 1e-4) * camera.GetProj()[1][1] * 0.5;
}

// The same estimate from the sphere's farthest point: a lower bound for every
// sphere it encloses.
float LodProjectedErrorFar(float4 sphere, float error, float4x4 world, float scale, Camera camera)
{
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

// The cluster half.
bool LodClusterPasses(MeshletCullData c, float4x4 world, float scale, Camera camera, float threshold)
{
    if (c.GetRefinedError() < 0)
        return true;
    return threshold > 0 && LodProjectedError(c.GetRefinedSphere(), c.GetRefinedError(), world, scale, camera) <= threshold;
}

// Whether nothing under a node or group can be in the cut. The bounds are
// conservative (enclosing spheres, max own / min refined error) and the 1%
// margins lean toward visiting: a wrong skip is a hole, a wrong visit only
// costs the exact per-group and per-cluster tests.
bool LodCanSkip(MeshletGroup g, float4x4 world, float scale, Camera camera, float threshold)
{
    if (threshold <= 0)
        return g.GetRefinedError() >= 0;
    // Every group is too fine.
    if (LodProjectedError(g.GetLodSphere(), g.GetLodError(), world, scale, camera) * 1.01 <= threshold)
        return true;
    // Every cluster needs its finer children.
    return g.GetRefinedError() >= 0
        && LodProjectedErrorFar(g.GetRefinedSphere(), g.GetRefinedError(), world, scale, camera) * 0.99 > threshold;
}

// Additive per-meshlet check on top of IsVisible()'s frustum+cone test,
// against the pyramid MeshRenderer.cpp builds from the main camera's own
// depth (mip 0 = last frame's HiZ during stage 1's AS dispatch, this
// frame's stage-1-drawn depth during stage 2's -- same timing the existing
// instance-level box test already relies on). See vsm_is_occluded in
// mesh_shader_vsm.hlsl for the same approach at page granularity.
bool IsOccludedHiZ(MeshletCullData c, float4x4 world, Camera camera)
{
    float4 BoundingSphere = c.GetBoundingSphere();

    float3 scales = float3(
        length(float3(world[0].x, world[1].x, world[2].x)),
        length(float3(world[0].y, world[1].y, world[2].y)),
        length(float3(world[0].z, world[1].z, world[2].z)));
    float scale = max(scales.x, max(scales.y, scales.z));

    float4 center = mul(world, float4(BoundingSphere.xyz, 1));
    center.xyz /= center.w;
    float radius = BoundingSphere.w * scale;

    float3 forward = camera.GetDirection().xyz;
    float3 near_world = center.xyz - forward * radius;

    float4 near_clip = mul(camera.GetViewProj(), float4(near_world, 1));
    float4 center_clip = mul(camera.GetViewProj(), float4(center.xyz, 1));

    // Crossing / behind the near plane: w <= 0 makes the perspective divide
    // flip signs, so both the screen rect and the depth come out garbage and
    // the test culls things that are right in front of the camera. Bail out
    // as visible instead of testing nonsense.
    if (near_clip.w <= 0 || center_clip.w <= 0)
        return false;

    near_clip.xyz /= near_clip.w;
    center_clip.xyz /= center_clip.w;

    // The sphere's actual NDC bounding rect, from projecting +-radius along
    // BOTH screen-perpendicular axes -- not just one direction assumed
    // isotropic (wrong under perspective) and not just the center point.
    // Thin/sparse geometry (a fence, where the bounding sphere is mostly
    // empty space between bars) needs the whole measured footprint checked,
    // or the nearest-point sample can land somewhere unrepresentative and
    // wrongly cull geometry that's genuinely in front.
    float3 world_up = (abs(forward.y) > 0.99) ? float3(1, 0, 0) : float3(0, 1, 0);
    float3 right = normalize(cross(world_up, forward));
    float3 up = normalize(cross(forward, right));

    float2 ndc_min = center_clip.xy;
    float2 ndc_max = center_clip.xy;
    float3 offsets[4] = { right * radius, -right * radius, up * radius, -up * radius };
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        float4 p = mul(camera.GetViewProj(), float4(center.xyz + offsets[i], 1));
        if (p.w <= 0)
            return false;
        p.xyz /= p.w;
        ndc_min = min(ndc_min, p.xy);
        ndc_max = max(ndc_max, p.xy);
    }

    // NDC->UV flips Y, so min/max swap; re-sort after converting.
    float2 uv_a = ndc_min * float2(0.5, -0.5) + float2(0.5, 0.5);
    float2 uv_b = ndc_max * float2(0.5, -0.5) + float2(0.5, 0.5);
    float2 screen_uv_min = min(uv_a, uv_b);
    float2 screen_uv_max = max(uv_a, uv_b);

    // Entirely off-screen: let the frustum test's own result stand.
    if (any(screen_uv_max < 0) || any(screen_uv_min > 1))
        return false;

    Texture2D<float> pyramid = frameInfo.GetMainHiZ();
    uint pw, ph, numLevels;
    pyramid.GetDimensions(0, pw, ph, numLevels);

    // Standard HZB test: pick the LOD where the rect spans at most 2x2
    // texels, then sample its four corners. Constant 4 taps, no loop.
    float2 rect_texels = (screen_uv_max - screen_uv_min) * float2(pw, ph);
    float  mip_f = ceil(log2(max(max(rect_texels.x, rect_texels.y), 1.0) * 0.5));
    uint   mip = (uint)clamp(mip_f, 0.0, (float)(numLevels - 1));

    // Combine with MIN, not max. Each texel holds the FARTHEST depth in its
    // footprint (min-reduction under reversed-Z), so the meshlet is occluded
    // only if it is behind the farthest surface of EVERY texel it covers:
    // near_z < min(taps). Using max asks "behind the nearest of them", which
    // calls it occluded as soon as any single texel does -- over-culling that
    // gets worse the more taps are added. (The reference article is standard
    // Z, where both the reduction and this combine are max; reversed-Z flips
    // both, and only flipping the pyramid build is the easy mistake.)
    float sampled = 1.0;
    sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float2(screen_uv_min.x, screen_uv_min.y), mip));
    sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float2(screen_uv_max.x, screen_uv_min.y), mip));
    sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float2(screen_uv_min.x, screen_uv_max.y), mip));
    sampled = min(sampled, pyramid.SampleLevel(pointClampSampler, float2(screen_uv_max.x, screen_uv_max.y), mip));

    return near_clip.z < sampled;
}

#ifdef BUILD_FUNC_AS
groupshared Payload s_Payload;

// The camera tests for one cluster. Correct for every user of THIS AS:
// gbuffer/stencil bind their own camera, PSSM binds the light camera.
// Voxelization uses mesh_shader_voxel's own AS, where frustum/cone culling
// stays disabled by design (3-axis raster). MaterialPreview3D (see
// material_preview.prism) also disables it: its mesh instance's cull data lives
// in the same shared global buffers the main editor scene concurrently
// reads/writes every frame, and boundary meshlets were flickering in/out -- a
// borderline/racy IsVisible() result, not anything about the geometry itself.
uint CameraCull(MeshletCullData cull_data, float4x4 m)
{
#ifdef DISABLE_MESHLET_CULL
    return MESHLET_DRAWN;
#else
    uint cull = MeshletCull(cull_data, m, frameInfo.GetCamera());
#ifdef HIZ_OCCLUSION
    // Additive Hi-Z check. The PSO permutation decides where this is on:
    // stage 2 of the occlusion culler only (see scene.prism).
    if (cull == MESHLET_DRAWN && IsOccludedHiZ(cull_data, m, frameInfo.GetCamera()))
        cull = MESHLET_CULLED_HIZ;
#endif
    return cull;
#endif
}

// The capture's frame tag is 28 bits: 4 are the status.
static const uint CAPTURE_FRAME_MASK = 0x0FFFFFFF;

#ifdef CAPTURE_MESHLETS
void CaptureStatus(uint cluster, uint status)
{
    MeshletCaptureWrite capture = GetMeshletCaptureWrite();
    capture.GetMasks()[meshInfo.GetMeshlet_mask_offset() + cluster] = ((capture.GetFrame() & CAPTURE_FRAME_MASK) << 4) | status;
}
#endif

#ifdef DEBUG_VIEW
// A cluster the main view never visited on the captured frame was skipped by
// its LOD hierarchy walk: not in the cut.
uint CapturedStatus(uint cluster)
{
    DebugViewDraw debug_draw = GetDebugViewDraw();
    uint entry = debug_draw.GetMeshlet_masks()[meshInfo.GetMeshlet_mask_offset() + cluster];
    return (entry >> 4) == (debug_draw.GetCapture_frame() & CAPTURE_FRAME_MASK) ? (entry & 15) : MESHLET_LOD_REJECTED;
}
#endif

#ifndef CLUSTER_LOD

// The original clusters, 32 per group (MeshCommandData::draw_commands).
[NumThreads(32, 1, 1)]
void AS(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{
    uint cull = MESHLET_CULLED_FRUSTUM;
    bool in_range = dtid < meshInfo.GetMeshlet_count();

    if (in_range)
    {
        node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
        matrix m = node.GetNode_global_matrix();
        MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + dtid];

        cull = CameraCull(cull_data, m);
#ifdef CAPTURE_MESHLETS
        CaptureStatus(dtid, cull);
#endif
    }

    bool visible = in_range && cull == MESHLET_DRAWN;

    // Compact visible meshlets into the export payload array.
    // NOTE: wave-op compaction assumes the 32-thread group fits ONE wave —
    // true on wave32/64 hardware (NV/AMD), broken on wave16 (would need
    // groupshared atomic compaction or [WaveSize(32)]).
    uint index = WavePrefixCountBits(visible);
    if (visible)
        s_Payload.MeshletIndices[index] = dtid;

    DispatchMesh(WaveActiveCountBits(visible), 1, 1, s_Payload);
}

#else

// Per lane, one group's cluster range: its first cluster and the end of its
// span in the node's concatenated cluster list.
groupshared uint s_GroupFirst[LOD_NODE_GROUPS];
groupshared uint s_GroupEnd[LOD_NODE_GROUPS];

// One LOD hierarchy node per group (MeshCommandData::lod_draw_commands): the
// node test can skip all of it, each lane tests one of its groups, then the
// lanes walk the surviving groups' clusters 32 at a time with the exact
// per-cluster tests. Same one-wave-per-group assumption as the flat AS.
[NumThreads(32, 1, 1)]
void AS(uint gtid : SV_GroupThreadID, uint gid : SV_GroupID)
{
    float lod_threshold = frameInfo.GetLodThreshold();
    Camera camera = frameInfo.GetCamera();
    node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
    matrix m = node.GetNode_global_matrix();
    float scale = MaxScale(m);

#ifdef DEBUG_VIEW
    DebugViewDraw debug_draw = GetDebugViewDraw();
    // Replaying the main view's cut: every cluster is visited and its recorded
    // status decides, so the debug camera's own LOD plays no part.
    bool replay = debug_draw.GetRead_meshlet_masks() != 0;
#else
    bool replay = false;
#endif

    bool node_live = gid < meshInfo.GetLod_node_count();
    MeshletGroup lod_node = (MeshletGroup)0;
    if (node_live)
    {
        lod_node = meshInstanceInfo.GetMeshletGroups()[meshInfo.GetLod_node_offset() + gid];
        node_live = replay || !LodCanSkip(lod_node, m, scale, camera, lod_threshold);
    }

    uint visible_count = 0;
    if (node_live)
    {
        uint first = 0;
        uint count = 0;
        if (gtid < lod_node.GetClusterCount())
        {
            MeshletGroup group = meshInstanceInfo.GetMeshletGroups()[meshInfo.GetLod_group_offset() + lod_node.GetFirstCluster() + gtid];
            if (replay || (LodGroupPasses(group, m, scale, camera, lod_threshold) && !LodCanSkip(group, m, scale, camera, lod_threshold)))
            {
                first = group.GetFirstCluster();
                count = group.GetClusterCount();
            }
        }

        uint total = WaveActiveSum(count);
        s_GroupFirst[gtid] = first;
        s_GroupEnd[gtid] = WavePrefixSum(count) + count;
        GroupMemoryBarrierWithGroupSync();

        for (uint base = 0; base < total; base += 32)
        {
            uint item = base + gtid;
            bool visible = false;
            uint cluster = 0;
#ifdef DEBUG_VIEW
            uint info = 0;
#endif
            if (item < total)
            {
                // The lane whose span holds the item: the first whose span ends
                // past it (ends never decrease; empty spans are skipped over).
                uint lane = 0;
                [unroll]
                for (uint step = 16; step > 0; step >>= 1)
                    if (s_GroupEnd[lane + step - 1] <= item)
                        lane += step;

                uint start = lane > 0 ? s_GroupEnd[lane - 1] : 0;
                cluster = s_GroupFirst[lane] + (item - start);
                MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + cluster];

#ifdef DEBUG_VIEW
                // The debug camera's own cull only skips clusters off its screen;
                // the main view's culled ones are drawn, tinted by the PS.
                uint status = replay ? CapturedStatus(cluster) : MESHLET_NOT_CAPTURED;
                bool lod_selected = replay ? status != MESHLET_LOD_REJECTED
                    : LodClusterPasses(cull_data, m, scale, camera, lod_threshold);
                visible = lod_selected && MeshletCull(cull_data, m, camera) != MESHLET_CULLED_FRUSTUM;
                if (debug_draw.GetMeshlet_filter() == 1)
                    visible = visible && (status == MESHLET_DRAWN || status == MESHLET_NOT_CAPTURED);
                else if (debug_draw.GetMeshlet_filter() == 2)
                    visible = visible && status != MESHLET_DRAWN;
                info = status | (cull_data.GetLodLevel() << 4);
#else
                uint cull = LodClusterPasses(cull_data, m, scale, camera, lod_threshold)
                    ? CameraCull(cull_data, m) : MESHLET_LOD_REJECTED;
#ifdef CAPTURE_MESHLETS
                CaptureStatus(cluster, cull);
#endif
                visible = cull == MESHLET_DRAWN;
#endif
            }

            uint index = visible_count + WavePrefixCountBits(visible);
            if (visible)
            {
                s_Payload.MeshletIndices[index] = cluster;
#ifdef DEBUG_VIEW
                s_Payload.MeshletInfo[index] = info;
#endif
            }
            visible_count += WaveActiveCountBits(visible);
        }
    }

    DispatchMesh(visible_count, 1, 1, s_Payload);
}

#endif
#endif


[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void VS(
    uint gtid : SV_GroupThreadID,
    uint2 gid2 : SV_GroupID,
    in payload Payload payload,
    out indices uint3 tris[124],
    out vertices vertex_output verts[64]
#ifdef DEBUG_VIEW
    , out primitives primitive_output prims[124]
#endif
)
{

    uint gid = gid2.x;

    uint meshletIndex = payload.MeshletIndices[gid];
    if (meshletIndex >= meshInfo.GetLod_meshlet_count()) return;
    Meshlet m = meshInstanceInfo.GetMeshlets()[meshInfo.GetMeshlet_offset_local() + meshletIndex];
    SetMeshOutputCounts(m.GetVertexCount(), m.GetPrimitiveCount());
 
    if (gtid < m.GetPrimitiveCount())
    {
        uint index_offset = 3*(m.GetPrimitiveOffset() + gtid);
        
        tris[gtid] = uint3(meshInstanceInfo.GetPrimitive_indices()[index_offset],
            meshInstanceInfo.GetPrimitive_indices()[index_offset + 1],
            meshInstanceInfo.GetPrimitive_indices()[index_offset + 2]);

#ifdef DEBUG_VIEW
        prims[gtid].meshlet = meshletIndex;
        prims[gtid].status = payload.MeshletInfo[gid] & 15;
        prims[gtid].lod_level = payload.MeshletInfo[gid] >> 4;
#endif

    }
    
    if (gtid < m.GetVertexCount())
    {
        uint vertexIndex =  meshInfo.GetVertex_offset_local() + meshInstanceInfo.GetUnique_indices()[ m.GetVertexOffset() + gtid];
        node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
        matrix m = node.GetNode_global_matrix();

        verts[gtid] = transform(m, node.GetNode_global_matrix_prev(), frameInfo.GetCamera(),  meshInstanceInfo.GetVertexes()[vertexIndex]);
    }
  
}



