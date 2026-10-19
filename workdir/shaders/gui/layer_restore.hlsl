// Group opacity (Renderer::end_layer): draws the target's pre-group snapshot
// back over the finished group with alpha (1 - opacity). The snapshot is a
// copy of the target, so it is already in output space and is read texel for
// texel (Load at the pixel's own position), not filtered.

struct vs_out
{
    float4 pos : SV_POSITION;
};

#include "../autogen/UILayerRestore.h"

#ifdef BUILD_FUNC_VS
vs_out VS(uint index : SV_VERTEXID)
{
    auto d = GetUILayerRestore();

    float2 corner = float2(index & 1, index >> 1);
    float2 wp = d.GetQuad().xy + corner * d.GetQuad().zw;

    vs_out o;
    o.pos = float4(wp.x / d.GetWindow_size().x * 2.0 - 1.0,
                   1.0 - wp.y / d.GetWindow_size().y * 2.0,
                   0.5, 1.0);
    return o;
}
#endif

#ifdef BUILD_FUNC_PS
float4 PS(vs_out i) : SV_TARGET0
{
    auto d = GetUILayerRestore();

    int2 texel = int2(i.pos.xy) - int2(d.GetOrigin());
    float4 s = d.GetSnapshot().Load(int3(texel, 0));

    return float4(s.rgb, d.GetAlpha());
}
#endif
