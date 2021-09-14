
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
#include "autogen/Voxelization.h"

static const FrameInfo frameInfo = GetFrameInfo();
static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();
static const VoxelInfo voxel_info = GetVoxelization().GetInfo();


//#include "autogen/DebugInfo.h"
vertex_output transform(matrix node_global_matrix, mesh_vertex_input i)
{
    vertex_output o;
    float4 tpos = mul(node_global_matrix, float4(i.pos, 1));
    o.wpos.xyz = tpos.xyz / tpos.w;
    o.pos = float4(o.wpos,1);
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
    o.prev_pos = o.pos;
    return o;
}





float4 TransformPos(float3 In)
{
    return float4((In.xyz - voxel_info.GetMin().xyz) / voxel_info.GetSize().xyz, 1);
}
float4 TransformPosition(float2 In)
{
    return float4((In) * 2 - 1, 0, 1);
}


struct Payload
{
    uint MeshletIndices[32];
};



#ifdef BUILD_FUNC_AS
groupshared Payload s_Payload;

[NumThreads(32, 1, 1)]
void AS(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{

    bool visible = false;


    // Check bounds of meshlet cull data resource
    if (dtid < meshInfo.GetMeshlet_count())
    {
      //  node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
   //    matrix m = node.GetNode_global_matrix();
    
        // Do visibility testing for this thread
        visible =  true;//IsVisible(sceneData.GetMeshletCullData()[meshInfo.GetMeshlet_offset() + dtid], m, 1, frameInfo.GetCamera());
    }

    // Compact visible meshlets into the export payload array 
    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dtid;
    }
    
    // Dispatch the required number of MS threadgroups to render the visible meshlets
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 3, 1, s_Payload);
}
#endif

#include "autogen/DebugInfo.h"


[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void VS(
    uint gtid : SV_GroupThreadID,
      uint2 gid2 : SV_GroupID,
	  in payload Payload payload,
    out indices uint3 tris[126],
    out vertices vertex_output verts[64]
)
{
   uint gid = gid2.x;
   
    uint meshletIndex = payload.MeshletIndices[gid];
    if (meshletIndex >= meshInfo.GetMeshlet_count()) return;
    Meshlet m = sceneData.GetMeshlets()[meshInfo.GetMeshlet_offset() + meshletIndex];   
	
	
    uint primPart = ceil(float(m.GetPrimitiveCount()) / 3);
    uint primStart = min(gid2.y * primPart, m.GetPrimitiveCount());
    uint primEnd = min(primStart+ primPart, m.GetPrimitiveCount());

    uint primCount = primEnd - primStart;
    uint primIndex = primStart + gtid;

//if (meshletIndex >= meshInfo.GetMeshlet_count()) return;

    SetMeshOutputCounts(primCount*3, primCount);

if(primCount>=64)
{
GetDebugInfo().Log(1,uint4(1,1,1,1));
}
  
    if (gtid < primCount )
    {
	
	      tris[gtid] = uint3(3 * gtid, 3 * gtid + 1, 3 * gtid + 2);
        uint index_offset = meshInfo.GetMeshlet_vertex_offset() + 3 * (m.GetPrimitiveOffset() + primIndex);
        uint index_offset0 = sceneData.GetIndices()[index_offset];
        uint index_offset1 = sceneData.GetIndices()[index_offset + 1];
        uint index_offset2 = sceneData.GetIndices()[index_offset + 2];

        uint vi0 = meshInfo.GetVertex_offset() + sceneData.GetIndices()[meshInfo.GetMeshlet_unique_offset() + m.GetVertexOffset() + index_offset0];
        uint vi1 = meshInfo.GetVertex_offset() + sceneData.GetIndices()[meshInfo.GetMeshlet_unique_offset() + m.GetVertexOffset() + index_offset1];
        uint vi2 = meshInfo.GetVertex_offset() + sceneData.GetIndices()[meshInfo.GetMeshlet_unique_offset() + m.GetVertexOffset() + index_offset2];

        node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
        matrix m = node.GetNode_global_matrix();


        vertex_output v0 = transform(node.GetNode_global_matrix(), sceneData.GetVertexes()[vi0]);
        vertex_output v1 = transform(node.GetNode_global_matrix(), sceneData.GetVertexes()[vi1]);
        vertex_output v2 = transform(node.GetNode_global_matrix(), sceneData.GetVertexes()[vi2]);

        float3 p0 = TransformPos(v0.wpos).xyz;
        float3 p1 = TransformPos(v1.wpos).xyz;
        float3 p2 = TransformPos(v2.wpos).xyz;

        float3 Delta1 = p0 - p1;
        float3 Delta2 = p0 - p2;


        float3 normal = abs(normalize(cross(Delta1, Delta2)));

        if (normal.z>normal.x&& normal.z > normal.y)
        {
            v0.pos = TransformPosition(p0.xy);
            v1.pos = TransformPosition(p1.xy);
            v2.pos = TransformPosition(p2.xy);
        }
        else   if (normal.y > normal.x && normal.y > normal.z)
        {
            v0.pos = TransformPosition(p0.xz);
            v1.pos = TransformPosition(p1.xz);
            v2.pos = TransformPosition(p2.xz);
        }
        else
        {
            v0.pos = TransformPosition(p0.yz);
            v1.pos = TransformPosition(p1.yz);
            v2.pos = TransformPosition(p2.yz);
        }

        verts[3 * gtid] = v0;
        verts[3 * gtid+1] = v1;
        verts[3 * gtid + 2] = v2;
    }
}

