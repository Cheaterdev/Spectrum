
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

#include "Common.hlsl"

#include "autogen/FrameInfo.h"
#include "autogen/MeshInfo.h" 
#include "autogen/SceneData.h"
#include "autogen/MeshInstanceInfo.h" 

static const FrameInfo frameInfo = GetFrameInfo();
static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();
static const MeshInstanceInfo meshInstanceInfo = GetMeshInstanceInfo();

//#include "autogen/DebugInfo.h"
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

struct Payload
{
    uint MeshletIndices[32];
};



bool IsConeDegenerate(MeshletCullData c)
{
    return (c.GetNormalCone() >> 24) == 0xff;
}

float4 UnpackCone(uint packed)
{
    float4 v;
    v.x = float((packed >> 0) & 0xFF);
    v.y = float((packed >> 8) & 0xFF);
    v.z = float((packed >> 16) & 0xFF);
    v.w = float((packed >> 24) & 0xFF);

    v = v / 255.0;
    v.xyz = v.xyz * 2.0 - 1.0;

    return v;
}

float dist(float4 plane, float3 pt)
{
    return dot(pt, plane.xyz) + plane.w;
}



bool IsVisible(MeshletCullData c, float4x4 world, Camera camera)
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
           return false;
        }
    }

    // Do normal cone culling
    if (IsConeDegenerate(c))
        return true; // Cone is degenerate - spread is wider than a hemisphere.

    // Under strongly non-uniform scale the cone axis would need the
    // inverse-transpose — be conservative and skip the cone test.
    if (max(scales.x, max(scales.y, scales.z)) > 1.05 * min(scales.x, min(scales.y, scales.z)))
        return true;

    // Unpack the normal cone from its 8-bit uint compression
    float4 normalCone = UnpackCone(c.GetNormalCone());

    // Transform axis to world space. NOTE: engine convention is
    // mul(matrix, vector) — the sample's mul(vector, matrix) transformed by
    // the TRANSPOSE (inverse rotation for rigid transforms), so the cone
    // pointed the wrong way on any rotated node and culled front-facing
    // meshlets. That is why this whole test was disabled.
    float3 axis = normalize(mul(world, float4(normalCone.xyz, 0)).xyz);

    // Offset the normal cone axis from the meshlet center-point - make sure to account for world scaling
    float3 apex = center.xyz - axis * c.GetApexOffset() * scale;
    float3 view = normalize(camera.GetPosition() - apex);

    // The normal cone w-component stores -cos(angle + 90 deg)
    // This is the min dot product along the inverted axis from which all the meshlet's triangles are backface
    if (dot(view, -axis) > normalCone.w)
    {
        return false;
    }

    // All tests passed - it will merit pixels
    return true;
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

[NumThreads(32, 1, 1)]
void AS(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{

    bool visible = false;


    // Check bounds of meshlet cull data resource
    if (dtid < meshInfo.GetMeshlet_count())
    {
        node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
       matrix m = node.GetNode_global_matrix();
        MeshletCullData cull_data = meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + dtid];

        // Do visibility testing for this thread. Correct for every user of
        // THIS AS: gbuffer/stencil bind their own camera, PSSM binds the
        // light camera. Voxelization uses mesh_shader_voxel's own AS, where
        // frustum/cone culling stays disabled by design (3-axis raster).
        visible = IsVisible(cull_data, m, frameInfo.GetCamera());

#ifdef HIZ_OCCLUSION
        // Additive Hi-Z check. The PSO permutation decides where this is on:
        // stage 2 of the occlusion culler only (see scene.sig).
        if (visible)
            visible = !IsOccludedHiZ(cull_data, m, frameInfo.GetCamera());
#endif
    }

    // Compact visible meshlets into the export payload array.
    // NOTE: wave-op compaction assumes the 32-thread group fits ONE wave —
    // true on wave32/64 hardware (NV/AMD), broken on wave16 (would need
    // groupshared atomic compaction or [WaveSize(32)]).
    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dtid;
    }
    
    // Dispatch the required number of MS threadgroups to render the visible meshlets
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}
#endif


[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void VS(
    uint gtid : SV_GroupThreadID,
    uint2 gid2 : SV_GroupID,
    in payload Payload payload,
    out indices uint3 tris[64],
    out vertices vertex_output verts[128]
)
{

    uint gid = gid2.x;

    uint meshletIndex = payload.MeshletIndices[gid];
    if (meshletIndex >= meshInfo.GetMeshlet_count()) return;
    Meshlet m = meshInstanceInfo.GetMeshlets()[meshInfo.GetMeshlet_offset_local() + meshletIndex];   
    SetMeshOutputCounts(m.GetVertexCount(), m.GetPrimitiveCount());
 
    if (gtid < m.GetPrimitiveCount())
    {
        uint index_offset = 3*(m.GetPrimitiveOffset() + gtid);
        
        tris[gtid] = uint3(meshInstanceInfo.GetPrimitive_indices()[index_offset],
            meshInstanceInfo.GetPrimitive_indices()[index_offset + 1],
            meshInstanceInfo.GetPrimitive_indices()[index_offset + 2]);

    }
    
    if (gtid < m.GetVertexCount())
    {
        uint vertexIndex =  meshInfo.GetVertex_offset_local() + meshInstanceInfo.GetUnique_indices()[ m.GetVertexOffset() + gtid];
        node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
        matrix m = node.GetNode_global_matrix();

        verts[gtid] = transform(m, node.GetNode_global_matrix_prev(), frameInfo.GetCamera(),  meshInstanceInfo.GetVertexes()[vertexIndex]);
    }
  
}



