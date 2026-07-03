struct ring_vertex_output
{
    float4 pos   : SV_POSITION;
    float3 world : TEXCOORD0;
};

#include "Common.hlsl"

#ifdef BUILD_FUNC_VS
#include "autogen/FrameInfo.h"
#include "autogen/DrawStencil.h"

static const FrameInfo   frameInfo = GetFrameInfo();
static const DrawStencil ringData  = GetDrawStencil();

ring_vertex_output VS(uint index: SV_VertexID)
{
    float3 v = ringData.GetVertices()[index].xyz;
    ring_vertex_output o;
    o.world = v;
    o.pos   = mul(frameInfo.GetCamera().GetViewProj(), float4(v, 1));
    return o;
}
#endif


#ifdef BUILD_FUNC_PS_COLOR
#include "autogen/FrameInfo.h"
#include "autogen/Color.h"

float4 PS_COLOR(ring_vertex_output i) : SV_TARGET0
{
    // Gizmo center is at the origin in this camera's space, so a ring point is on
    // the camera-facing half when it projects positively onto the camera position.
    float3 camPos = GetFrameInfo().GetCamera().GetPosition().xyz;
    if (dot(i.world, camPos) < 0)
        discard;
    return GetColor().GetColor();
}
#endif


#ifdef BUILD_FUNC_PS
#include "autogen/PickerBuffer.h"
#include "autogen/Instance.h"

static const RWStructuredBuffer<uint> pickBuffer = GetPickerBuffer().GetViewBuffer();

[earlydepthstencil]
void PS(ring_vertex_output i)
{
    pickBuffer[0] = GetInstance().GetInstanceId();
}
#endif
