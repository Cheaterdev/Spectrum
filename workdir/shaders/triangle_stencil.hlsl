
struct vertex_output
{
    float4 pos : SV_POSITION;
 };

#include "Common.hlsl"

#ifdef BUILD_FUNC_VS
#include "autogen/FrameInfo.h"
#include "autogen/DrawStencil.h" 
#include "autogen/SceneData.h"
#include "autogen/MeshInfo.h"

static const FrameInfo frameInfo = GetFrameInfo();
static const SceneData sceneData = GetSceneData();
static const DrawStencil stencilData = GetDrawStencil();
static const MeshInfo meshInfo = GetMeshInfo();

vertex_output VS(uint index: SV_VertexID)
{
    node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];
    AABB aabb = node.GetAabb();
    float3 min = aabb.GetMin().xyz;
    float3 max = aabb.GetMax().xyz;


    float3 pos = 0.5+0.5*stencilData.GetVertices()[index];

  


    pos = min + pos*(max-min);
    vertex_output o;
    o.pos = float4(pos, 1);
    o.pos = mul(node.GetNode_global_matrix(), o.pos);
   o.pos = mul(frameInfo.GetCamera().GetViewProj(), o.pos);
    return o;// transform(m, frameInfo.GetCamera(), stencilData.GetVertices()[index]);
}
#endif


float4 PS(vertex_output i) :SV_TARGET0
{
return 0.05;
}
