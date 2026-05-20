#include "../autogen/FrameGraph_Debug_TextureCube.h"
#include "../autogen/FrameGraph_Debug_Common.h"

// D3D cubemap face UV -> sample direction.
// st = uv * 2 - 1, maps [0,1] to [-1,1].
float3 cube_dir(uint face, float2 uv)
{
    float2 st = uv * 2.0 - 1.0;
    if (face == 0) return normalize(float3( 1.0, -st.y, -st.x)); // +X
    if (face == 1) return normalize(float3(-1.0, -st.y,  st.x)); // -X
    if (face == 2) return normalize(float3( st.x,  1.0,  st.y)); // +Y
    if (face == 3) return normalize(float3( st.x, -1.0, -st.y)); // -Y
    if (face == 4) return normalize(float3( st.x, -st.y,  1.0)); // +Z
    return               normalize(float3(-st.x, -st.y, -1.0)); // -Z
}

[numthreads(8, 8, 1)]
void CS(uint2 tid : SV_DispatchThreadID)
{
    uint2 tsize = GetFrameGraph_Debug_Common().GetTargetSize();
    if (any(tid >= tsize))
        return;

    // Lay out 6 faces in a 3 columns x 2 rows grid.
    uint2 cell = uint2(tsize.x / 3, tsize.y / 2);
    uint col = tid.x / cell.x;
    uint row = tid.y / cell.y;

    if (col >= 3 || row >= 2 || cell.x == 0 || cell.y == 0)
    {
        GetFrameGraph_Debug_Common().GetTarget()[tid] = 0.2 * (((tid.x / 8) + (tid.y / 8)) % 2 == 0);
        return;
    }

    uint face = row * 3 + col;
    float2 uv = float2(tid.x % cell.x, tid.y % cell.y) / float2(cell);

    float3 dir = cube_dir(face, uv);
    float4 color = GetFrameGraph_Debug_TextureCube().GetSource().SampleLevel(linearSampler, dir, GetFrameGraph_Debug_Common().GetSelectedMip());

    GetFrameGraph_Debug_Common().GetTarget()[tid] = float4(pow(max(color.xyz, 0.0), 1.0 / 2.2), 1.0);
}
