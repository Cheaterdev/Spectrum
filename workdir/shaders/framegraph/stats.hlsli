// Layout of FrameGraph_Debug_Common.stats (uints), shared by the histogram and draw shaders and read back by resource_preview:
//   [0..3]   picked texel rgba (float bits)   [4,5] its x,y   [6] 1 when the cursor is over the image
//   [7]      histogram sample count
//   [8..23]  per channel c (r,g,b,a), 4 words at 8 + c*4:  min, max (ordered float bits), sum whole part, sum fraction
//   [24 ..]  histogram: 4 channels x 256 bins
// resource_preview reads words 0..23 in one go. Keep in sync with preview_stats in FrameGraph.Debug.cpp.
static const uint STATS_PICK_VALID = 6;
static const uint STATS_TOTAL      = 7;
static const uint STATS_MINMAX     = 8;
static const uint STATS_HIST       = 24;
static const uint STATS_WORDS      = 24 + 4 * 256;

static const uint FLAG_CLEAR = 2;   // histogram shaders only: zero the stats words instead of accumulating (bit 1 of flags is free)

static const uint HIST_MAX_GRID = 512;

// Samples are summed as whole part + 12-bit fraction (each clamped to 0..255) so 512 x 512 of them fit 32-bit words.
static const uint SUM_FRACTION_SCALE = 4096;
static const float SUM_MAX = 255.0;

uint hist_bin(float v)
{
    return (uint)(saturate(v) * 255.0 + 0.5);
}

// Float order == uint order, negatives included, so InterlockedMin / InterlockedMax work on the bits.
uint float_to_ordered(float f)
{
    uint u = asuint(f);
    return (u & 0x80000000u) ? ~u : (u | 0x80000000u);
}

float ordered_to_float(uint u)
{
    return asfloat((u & 0x80000000u) ? (u & 0x7FFFFFFFu) : ~u);
}

// Histogram shaders, first dispatch (flag FLAG_CLEAR): 128 x N threads zero the words; the minima start at "max".
void stats_clear(uint2 g)
{
    uint word = g.y * 128 + g.x;
    if (word >= STATS_WORDS)
        return;

    const bool is_min = word >= STATS_MINMAX && word < STATS_HIST && ((word - STATS_MINMAX) & 3) == 0;
    GetFrameGraph_Debug_Common().GetStats()[word] = is_min ? 0xFFFFFFFFu : 0u;
}

// Histogram shaders, second dispatch: one sample's bins, min, max and sum for each channel.
void stats_accumulate(float4 v)
{
    uint ignored;
    [unroll]
    for (uint c = 0; c < 4; ++c)
    {
        float x = v[c];
        if (isnan(x))
            x = 0.0;

        InterlockedAdd(GetFrameGraph_Debug_Common().GetStats()[STATS_HIST + c * 256 + hist_bin(x)], 1, ignored);

        const uint at = STATS_MINMAX + c * 4;
        const uint ordered = float_to_ordered(x);
        InterlockedMin(GetFrameGraph_Debug_Common().GetStats()[at + 0], ordered);
        InterlockedMax(GetFrameGraph_Debug_Common().GetStats()[at + 1], ordered);

        const float s = clamp(x, 0.0, SUM_MAX);
        const float whole = floor(s);
        InterlockedAdd(GetFrameGraph_Debug_Common().GetStats()[at + 2], (uint)whole, ignored);
        InterlockedAdd(GetFrameGraph_Debug_Common().GetStats()[at + 3], (uint)((s - whole) * SUM_FRACTION_SCALE), ignored);
    }
}

// Called by the draw shader for every output pixel; only the one under the cursor stores.
void pick_store(uint2 px, float4 raw, int2 texel, bool valid)
{
    if (any(int2(px) != GetFrameGraph_Debug_Common().GetCursor()))
        return;

    GetFrameGraph_Debug_Common().GetStats()[0] = asuint(raw.x);
    GetFrameGraph_Debug_Common().GetStats()[1] = asuint(raw.y);
    GetFrameGraph_Debug_Common().GetStats()[2] = asuint(raw.z);
    GetFrameGraph_Debug_Common().GetStats()[3] = asuint(raw.w);
    GetFrameGraph_Debug_Common().GetStats()[4] = asuint(texel.x);
    GetFrameGraph_Debug_Common().GetStats()[5] = asuint(texel.y);
    GetFrameGraph_Debug_Common().GetStats()[STATS_PICK_VALID] = valid ? 1u : 0u;
}
