#include "../autogen/BackdropBlur.h"

// Dual Kawase blur step (Marius Bjorge, "Bandwidth-efficient rendering").
// Down: dst is half the size of src. Up: dst is twice the size of src.
// `o` is src texel size scaled by the caller's offset, so a fractional offset
// gives radii between whole octaves without another pass.

[numthreads(8, 8, 1)]
void CS(uint2 dtid : SV_DispatchThreadID)
{
    auto d = GetBackdropBlur();
    uint2 size = d.GetDst_size();

    if (dtid.x >= size.x || dtid.y >= size.y)
        return;

    float2 uv = (float2(dtid) + 0.5) / float2(size);
    float2 o = d.GetSrc_texel() * d.GetOffset();
    Texture2D<float4> src = d.GetSrc();

    float4 result;

#ifdef BACKDROP_UP
    result  = src.SampleLevel(linearClampSampler, uv + float2(-o.x * 2.0, 0.0), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2(-o.x,  o.y), 0) * 2.0;
    result += src.SampleLevel(linearClampSampler, uv + float2(0.0,  o.y * 2.0), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2( o.x,  o.y), 0) * 2.0;
    result += src.SampleLevel(linearClampSampler, uv + float2( o.x * 2.0, 0.0), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2( o.x, -o.y), 0) * 2.0;
    result += src.SampleLevel(linearClampSampler, uv + float2(0.0, -o.y * 2.0), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2(-o.x, -o.y), 0) * 2.0;
    result /= 12.0;
#else
    result  = src.SampleLevel(linearClampSampler, uv, 0) * 4.0;
    result += src.SampleLevel(linearClampSampler, uv + float2(-o.x, -o.y), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2( o.x, -o.y), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2(-o.x,  o.y), 0);
    result += src.SampleLevel(linearClampSampler, uv + float2( o.x,  o.y), 0);
    result /= 8.0;
#endif

    d.GetDst()[dtid] = result;
}
