#include "../autogen/StatGraph.h"

static const float GLOW_RADIUS = 5.0;
static const float LINE_WIDTH  = 0.5;

float smooth_sample(float u)
{
    auto sg = GetStatGraph();
    uint count = sg.GetCount();
    if (count == 0) return 0.0;
    if (count == 1) return sg.GetSamples()[0];

    float idx_f = u * (float)(count - 1);
    int   i0    = (int)floor(idx_f);
    int   i1    = i0 + 1;
    float t     = frac(idx_f);
    t = t * t * (3.0 - 2.0 * t);

    float v0 = sg.GetSamples()[clamp(i0, 0, (int)count - 1)];
    float v1 = sg.GetSamples()[clamp(i1, 0, (int)count - 1)];
    return lerp(v0, v1, t);
}

float graph_y_at(float x, float w, float h, float vmin, float range)
{
    float val  = smooth_sample(saturate(x / w));
    float nval = saturate((val - vmin) / range);
    return (1.0 - nval) * h;
}

// Signed distance to a vertical segment [y_lo, y_hi]:
//   negative above, 0 inside, positive below
float dist_to_segment(float py, float y_lo, float y_hi)
{
    if (py < y_lo) return py - y_lo; // negative = above
    if (py > y_hi) return py - y_hi; // positive = below
    return 0.0;
}

[numthreads(8, 8, 1)]
void CS(uint2 dtid : SV_DispatchThreadID)
{
    auto sg = GetStatGraph();
    uint w = sg.GetWidth();
    uint h = sg.GetHeight();

    if (dtid.x >= w || dtid.y >= h) return;

    float px   = (float)dtid.x + 0.5;
    float py   = (float)dtid.y + 0.5;
    float fw   = (float)w;
    float fh   = (float)h;
    float bg_t = py / fh;

    uint  count = sg.GetCount();
    float vmin  = sg.GetVmin();
    float vmax  = sg.GetVmax();
    float range = max(vmax - vmin, 0.001);

    // Graph y at left and right edges of this pixel column.
    // The segment [y_lo, y_hi] covers the full vertical range the line
    // passes through this column, bridging gaps on steep slopes.
    float gy_l = graph_y_at(px - 0.5, fw, fh, vmin, range);
    float gy_r = graph_y_at(px + 0.5, fw, fh, vmin, range);
    float y_lo = min(gy_l, gy_r);
    float y_hi = max(gy_l, gy_r);

    float dist     = dist_to_segment(py, y_lo, y_hi);
    float abs_dist = abs(dist);

    // ── Background gradient ──────────────────────────────────────────────────
    float4 color = lerp(sg.GetBgTop(), sg.GetBgBot(), bg_t);

    // ── Subtle horizontal grid lines at 25% / 50% / 75% ────────────────────
    [unroll] for (int g = 1; g <= 3; g++)
    {
        float gy   = (float)g * 0.25 * fh;
        float gd   = abs(py - gy);
        float galp = saturate(1.3 - gd) * 0.07;
        color.rgb += galp;
    }

    // ── Area fill: gradient from line downward ───────────────────────────────
    float fill_cover = saturate(py - y_lo + 1.0);
    if (fill_cover > 0.0)
    {
        float  fill_t = saturate((py - y_lo) / (fh - y_lo));
        float  alpha  = sg.GetFillTop().a * (1.0 - fill_t);
        color.rgb     = lerp(color.rgb, sg.GetFillTop().rgb, alpha * fill_cover);
    }

    // ── Soft glow around the segment ─────────────────────────────────────────
    float glow_raw   = saturate(1.0 - abs_dist / GLOW_RADIUS);
    float glow_alpha = glow_raw * glow_raw * 0.45;
    color.rgb = lerp(color.rgb, sg.GetLineColor().rgb, glow_alpha * sg.GetLineColor().a);

    // ── AA line covering the full vertical segment ────────────────────────────
    float line_aa = saturate(1.5 - max(abs_dist - LINE_WIDTH * 0.5, 0.0));
    color.rgb = lerp(color.rgb, sg.GetLineColor().rgb, line_aa * sg.GetLineColor().a);

    // ── Current-value dot at rightmost sample ────────────────────────────────
    if (count > 0)
    {
        float last_nval = saturate((sg.GetSamples()[count - 1] - vmin) / range);
        float marker_y  = (1.0 - last_nval) * fh;
        float marker_x  = fw - 0.5;
        float mdist     = length(float2(px - marker_x, py - marker_y));
        float malpha    = saturate(3.5 - mdist) * sg.GetLineColor().a;
        color.rgb = lerp(color.rgb, sg.GetLineColor().rgb * 1.4 + 0.1, malpha);
    }

    sg.GetOutput()[dtid] = float4(color.rgb, 1.0);
}
