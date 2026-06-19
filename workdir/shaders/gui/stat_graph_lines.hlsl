#include "../autogen/StatGraphLine.h"

static const float LINE_HALF_WIDTH = 1.5; // pixels

// ── Vertex shader ──────────────────────────────────────────────────────────
struct VS_OUT
{
    float4 pos : SV_Position;
};

VS_OUT VS(uint vid : SV_VertexID)
{
    ConstantBuffer<StatGraphLine> sg = GetStatGraphLine();
    uint count = sg.GetCount();
    float fw = (float)sg.GetWidth();
    float fh = (float)sg.GetHeight();

    float u     = (count > 1) ? (float)vid / (float)(count - 1) : 0.5;
    float val   = sg.GetSamples()[vid];
    float range = max(sg.GetVmax() - sg.GetVmin(), 0.001);
    float nval  = saturate((val - sg.GetVmin()) / range);

    // Pixel space → snap to integer pixel grid → NDC
    float px = floor(u * fw) + 0.5;
    float py = floor((1.0 - nval) * fh) + 0.5;

    VS_OUT o;
    o.pos = float4(px / fw * 2.0 - 1.0, -(py / fh * 2.0 - 1.0), 0.0, 1.0);
    return o;
}

// ── Geometry shader ─────────────────────────────────────────────────────────
struct GS_OUT
{
    float4 pos : SV_Position;
    // tc: x=along segment, y=cross (fill: 0=line edge 1=bottom / line: 0=edge 1=edge),
    //     z=0 fill, z=1 line
    float3 tc  : TEXCOORD0;
};

[maxvertexcount(12)]
void GS(line VS_OUT input[2], inout TriangleStream<GS_OUT> stream)
{
    ConstantBuffer<StatGraphLine> sg = GetStatGraphLine();
    float inv_x = 2.0 / (float)sg.GetWidth();
    float inv_y = 2.0 / (float)sg.GetHeight();

    // Perpendicular normal in pixel space → NDC offset
    float2 dir_px = float2(
        (input[1].pos.x - input[0].pos.x) / inv_x,
        (input[1].pos.y - input[0].pos.y) / inv_y);
    float len = length(dir_px);
    if (len < 0.0001) return;
    float2 n_px = float2(-dir_px.y, dir_px.x) / len;
    float4 hw   = LINE_HALF_WIDTH * float4(n_px.x * inv_x, n_px.y * inv_y, 0, 0);

    float4 bot0 = float4(input[0].pos.x, -1.0, 0, 1);
    float4 bot1 = float4(input[1].pos.x, -1.0, 0, 1);

    GS_OUT o;

    // ── Fill trapezoid (from line down to bottom) ──────────────────────────
    // tc.y: 0 at line position, 1 at bottom   tc.z = 0
    o.pos = input[0].pos; o.tc = float3(0, 0, 0); stream.Append(o);
    o.pos = input[1].pos; o.tc = float3(1, 0, 0); stream.Append(o);
    o.pos = bot0;         o.tc = float3(0, 1, 0); stream.Append(o);
    stream.RestartStrip();

    o.pos = input[1].pos; o.tc = float3(1, 0, 0); stream.Append(o);
    o.pos = bot1;         o.tc = float3(1, 1, 0); stream.Append(o);
    o.pos = bot0;         o.tc = float3(0, 1, 0); stream.Append(o);
    stream.RestartStrip();

    // ── Line quad (thick AA line) ──────────────────────────────────────────
    // tc.y: 0 and 1 are edges, 0.5 is center   tc.z = 1
    o.pos = input[0].pos - hw; o.tc = float3(0, 0, 1); stream.Append(o);
    o.pos = input[1].pos - hw; o.tc = float3(1, 0, 1); stream.Append(o);
    o.pos = input[0].pos + hw; o.tc = float3(0, 1, 1); stream.Append(o);
    stream.RestartStrip();

    o.pos = input[1].pos - hw; o.tc = float3(1, 0, 1); stream.Append(o);
    o.pos = input[1].pos + hw; o.tc = float3(1, 1, 1); stream.Append(o);
    o.pos = input[0].pos + hw; o.tc = float3(0, 1, 1); stream.Append(o);
    stream.RestartStrip();
}

// ── Pixel shader ────────────────────────────────────────────────────────────
float4 PS(GS_OUT input) : SV_Target
{
    ConstantBuffer<StatGraphLine> sg = GetStatGraphLine();

    if (input.tc.z < 0.5)
    {
        // Fill: full opacity at top of texture, zero at bottom
        float4 c = sg.GetFillColor();
        c.rgb *= 1.3;
        float fill_t = input.pos.y / (float)sg.GetHeight();
        c.a *= 1.4 * lerp(0.0, 1.0, 1.0 - fill_t);

        return c;
    }
    else
    {
        // Line: AA soft falloff at edges
        float4 c = sg.GetLineColor();
        float edge = abs(input.tc.y * 2.0 - 1.0);
        c.a *= 1.0 - smoothstep(0.2, 1.0, edge);
      
        return c;
    }
}
