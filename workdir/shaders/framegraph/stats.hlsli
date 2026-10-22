// Layout of FrameGraph_Debug_Common.stats (uints), shared by the histogram and draw shaders and read back by resource_preview:
//   [0..3] picked texel rgba (float bits)   [4,5] its x,y   [6] 1 when the cursor is over the image
//   [7] histogram sample count              [8 ..] 4 channels x 256 bins
// Keep in sync with the constants in FrameGraph.Debug.cpp.
static const uint STATS_PICK_VALID = 6;
static const uint STATS_TOTAL      = 7;
static const uint STATS_HIST       = 8;

static const uint STATS_WORDS = 8 + 4 * 256;

static const uint FLAG_HISTOGRAM = 1;
static const uint FLAG_CLEAR     = 2;   // histogram shaders only: zero the stats words instead of accumulating

static const uint HIST_MAX_GRID = 512;

uint hist_bin(float v)
{
    return (uint)(saturate(v) * 255.0 + 0.5);
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

// Bottom-left corner overlay: log-scaled bars, one per enabled channel (alpha in grey).
float3 draw_histogram_overlay(float3 col, uint2 px)
{
    if ((GetFrameGraph_Debug_Common().GetFlags() & FLAG_HISTOGRAM) == 0)
        return col;

    const int W = 256, H = 80, M = 8;
    uint2 ts = GetFrameGraph_Debug_Common().GetTargetSize();
    if (ts.x < (uint)(W + 2 * M) || ts.y < (uint)(H + 2 * M))
        return col;

    int2 o = int2(px) - int2(M, (int)ts.y - M - H);
    if (o.x < 0 || o.y < 0 || o.x >= W || o.y >= H)
        return col;

    float denom = log(1.0 + max(GetFrameGraph_Debug_Common().GetStats()[STATS_TOTAL], 1u));
    float fromBottom = float(H - 1 - o.y);
    uint mask = GetFrameGraph_Debug_Common().GetChannelMask();

    float3 bars = 0;
    [unroll]
    for (uint c = 0; c < 4; ++c)
    {
        if ((mask & (1u << c)) == 0)
            continue;

        float h = log(1.0 + GetFrameGraph_Debug_Common().GetStats()[STATS_HIST + c * 256 + o.x]) / denom * H;
        if (fromBottom < h)
            bars += (c == 3) ? float3(0.45, 0.45, 0.45) : float3(c == 0, c == 1, c == 2) * 0.9;
    }

    return saturate(col * 0.35 + bars);
}
