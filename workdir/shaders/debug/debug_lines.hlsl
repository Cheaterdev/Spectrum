#include "../common/common.hlsl"
#include "../autogen/FrameInfo.h"
#include "../autogen/DebugLines.h"

struct line_vertex
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    // Signed pixel distance from the segment's centre line, and the solid half
    // width: the PS fades the last pixel for antialiasing.
    float edge : EDGE;
    float half_width : HALF_WIDTH;
};

#ifdef BUILD_FUNC_VS
static const float2 corners[6] =
{
    float2(0, -1), float2(1, -1), float2(1, 1),
    float2(0, -1), float2(1, 1), float2(0, 1)
};

line_vertex VS(uint vertex_id : SV_VertexID)
{
    DebugLines lines = GetDebugLines();
    DebugLineSegment s = lines.GetSegments()[vertex_id / 6];
    float2 corner = corners[vertex_id % 6];

    float4x4 view_proj = GetFrameInfo().GetCamera().GetViewProj();
    float4 a = mul(view_proj, float4(s.a.xyz, 1));
    float4 b = mul(view_proj, float4(s.b.xyz, 1));

    line_vertex o = (line_vertex)0;

    // Clip to just in front of the eye before the w-divide: an endpoint behind
    // the camera would otherwise project mirrored and smear the quad across the
    // screen. The hardware near-plane clip handles the rest.
    const float min_w = 1e-3;
    if (a.w < min_w && b.w < min_w)
        return o;
    if (a.w < min_w)
        a = lerp(a, b, (min_w - a.w) / (b.w - a.w));
    else if (b.w < min_w)
        b = lerp(b, a, (min_w - b.w) / (a.w - b.w));

    float2 half_viewport = 0.5 * lines.GetViewport_size();
    float2 pa = a.xy / a.w * half_viewport;
    float2 pb = b.xy / b.w * half_viewport;

    float2 dir = pb - pa;
    float len = length(dir);
    dir = len > 1e-5 ? dir / len : float2(1, 0);
    float2 normal = float2(-dir.y, dir.x);

    float half_width = max(s.a.w, 1) * 0.5;
    float extent = half_width + 1;

    float4 p = corner.x > 0.5 ? b : a;
    float2 offset_px = normal * corner.y * extent + dir * (corner.x > 0.5 ? extent : -extent);
    p.xy += offset_px / half_viewport * p.w;

    o.pos = p;
    o.color = s.color;
    o.edge = corner.y * extent;
    o.half_width = half_width;
    return o;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(line_vertex i) : SV_TARGET0
{
    float coverage = saturate(i.half_width + 0.5 - abs(i.edge));
    return float4(i.color.rgb, i.color.a * coverage * GetDebugLines().GetAlpha_scale());
}
#endif
