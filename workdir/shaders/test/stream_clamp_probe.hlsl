// See texture_streaming.prism. One thread per mip level.

#include "../autogen/StreamClampProbe.h"
#include "../streaming/stream_feedback.hlsl"

#ifdef BUILD_FUNC_CS
[numthreads(16, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    StreamClampProbe data = GetStreamClampProbe();
    const uint level = dispatchID.x;

    Texture2D<float4> source = data.GetSource();

    uint2 size;
    uint levels;
    source.GetDimensions(0, size.x, size.y, levels);

    // The same value the material shaders compute: stream_info holds the finest resident mip plus one, 0 for a
    // texture that is not streamed.
    const float min_mip = float(max(data.GetStream_info()[data.GetView()], 1u) - 1u);

    const float4 raised = source.SampleLevel(pointClampSampler, data.GetUv(), max(float(level), min_mip));

    // Axis-aligned gradients, so the footprint is exactly 2^level texels.
    const float2 step = float(1u << level) / float2(size);
    const float4 clamped = source.SampleGrad(pointClampSampler, data.GetUv(), float2(step.x, 0), float2(0, step.y), int2(0, 0), min_mip);

    if (level >= data.GetReport_level() && level < data.GetReport_level() + 3)
        stream_report(data.GetStream_info(), data.GetWanted(), data.GetView(), float(level));

    data.GetResult()[level * 2] = uint(raised.r * 255.0 + 0.5);
    data.GetResult()[level * 2 + 1] = uint(clamped.r * 255.0 + 0.5);
}
#endif
