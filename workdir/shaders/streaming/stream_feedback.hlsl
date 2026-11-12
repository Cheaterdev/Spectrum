// What a material pixel shader reports about a streamed texture: the mip a sample asked for, into the same
// two-dwords-per-id feedback the Quanta draw writes (see stream_compact.hlsl), with one id per standard mip of a texture.
//
// `info` is TextureStreaming::Manager::stream_info, two dwords per descriptor-heap index:
//   [2 * view]      0 for a texture that is not streamed, which is not reported;
//   [2 * view + 1]  the texture's first mip id in the low 24 bits, its coarsest standard mip in the bits above.
// `feedback` is TextureStreaming::Manager::feedback(), `epoch` the one of the frame being recorded
// (QuantaFeedback::texture_epoch). Not cleared: a report counts only for the epoch it carries.
//   [2 * id]      epoch << 16 | priority: that mip was sampled, and how urgent it is;
//   [2 * id + 1]  the last epoch it was sampled in (what the eviction compares against).

#ifndef STREAM_FEEDBACK_HLSL
#define STREAM_FEEDBACK_HLSL

// The compaction buckets the half-float priority by its exponent: bucket = clamp(exponent - 10, 0, 15) (PriorityBucket in
// stream_compact.hlsl), and the highest bucket is loaded first. A coarser mip must come first (the finer ones stand on
// it), so the bucket is the mip number, capped at 15: exponent = mip + 10, mantissa 0. The offset 10 comes from that
// bucket layout, it is not a choice.
uint stream_priority(uint mip)
{
    return (min(mip, 15u) + 10u) << 10;
}

void stream_report(StructuredBuffer<uint> info, RWStructuredBuffer<uint> feedback, uint view, float lod, uint epoch)
{
    if (info[NonUniformResourceIndex(view * 2)] == 0)
        return;

    const uint target = info[NonUniformResourceIndex(view * 2 + 1)];
    const uint mip = min(uint(max(lod, 0.0)), target >> 24);
    const uint id = (target & 0xFFFFFFu) + mip;
    const uint stamp = (epoch << 16) | stream_priority(mip);

    // Most waves sample one mip of one texture: one pair of atomics per wave instead of one per pixel.
    if (WaveActiveAllEqual(id))
    {
        if (WaveIsFirstLane())
        {
            InterlockedMax(feedback[id * 2], stamp);
            InterlockedMax(feedback[id * 2 + 1], epoch);
        }
    }
    else
    {
        InterlockedMax(feedback[NonUniformResourceIndex(id * 2)], stamp);
        InterlockedMax(feedback[NonUniformResourceIndex(id * 2 + 1)], epoch);
    }
}

#endif
