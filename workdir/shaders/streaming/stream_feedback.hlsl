// What a material pixel shader reports about a streamed texture: the finest mip a sample asked for.
// The value is 16 - mip, so 0 means "not sampled" and the buffer is cleared to zero; InterlockedMax keeps the finest.
// `info` is TextureStreaming::Manager::stream_info: 0 for a texture that is not streamed, which is not reported.

#ifndef STREAM_FEEDBACK_HLSL
#define STREAM_FEEDBACK_HLSL

void stream_report(StructuredBuffer<uint> info, RWStructuredBuffer<uint> wanted, uint view, float lod)
{
    if (info[NonUniformResourceIndex(view)] == 0)
        return;

    uint value = 16u - min(uint(max(lod, 0.0)), 15u);

    // Most waves sample one texture: one atomic per wave instead of one per pixel.
    if (WaveActiveAllEqual(view))
    {
        value = WaveActiveMax(value);
        if (WaveIsFirstLane())
            InterlockedMax(wanted[view], value);
    }
    else
    {
        InterlockedMax(wanted[NonUniformResourceIndex(view)], value);
    }
}

#endif
