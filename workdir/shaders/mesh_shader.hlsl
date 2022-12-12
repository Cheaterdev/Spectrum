
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



bool IsVisible(MeshletCullData c, float4x4 world, float scale, Camera camera)
{
   // if ((Instance.Flags & CULL_FLAG) == 0)
    //    return true;

    float4 BoundingSphere = c.GetBoundingSphere();
  //  float4 BoundingSphere = c.GetBoundingSphere();

    // Do a cull test of the bounding sphere against the view frustum planes.
    float4 center = mul(world, float4(BoundingSphere.xyz, 1));
    center.xyz /= center.w;
    float radius = BoundingSphere.w * scale;

    [unroll]
    for (int i = 0; i < 6; ++i)
    {

        float d = dist(camera.GetFrustum().GetPlanes(i), center);

        if (d < -radius)
        {
           return false;
        }
    }

    // Do normal cone culling
    if (IsConeDegenerate(c))
        return true; // Cone is degenerate - spread is wider than a hemisphere.

    // Unpack the normal cone from its 8-bit uint compression
    float4 normalCone = UnpackCone(c.GetNormalCone());

    // Transform axis to world space
    float3 axis = normalize(mul(float4(normalCone.xyz, 0), world)).xyz;

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
    
        // Do visibility testing for this thread
        visible =  IsVisible(meshInstanceInfo.GetMeshletCullData()[meshInfo.GetMeshlet_offset_local() + dtid], m, 1, frameInfo.GetCamera());
    }

    // Compact visible meshlets into the export payload array 
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



