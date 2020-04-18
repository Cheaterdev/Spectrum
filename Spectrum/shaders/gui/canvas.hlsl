#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float2 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_PS


#include "../autogen/FlowGraph.h"

static const   float4 size = GetFlowGraph().GetSize();
static const float4 offset_scale = GetFlowGraph().GetOffset_size();

#define SHADOW_SIZE 1.0/50.0

float4 PS(quad_output i) : SV_TARGET0
{

    float2 screen_tc = i.pos.xy / i.pos.z;

//   clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);
    float scale = offset_scale.z;

    float vignette = 1;
    int2 pixel = i.tc * size.xy;

    //return float4(pixel.xyxy) / 50;
    vignette = min(vignette, pixel.x * SHADOW_SIZE);
    vignette = min(vignette, pixel.y * SHADOW_SIZE);
    vignette = min(vignette, (size.x - pixel.x) * SHADOW_SIZE);
    vignette = min(vignette, (size.y - pixel.y) * SHADOW_SIZE);
    vignette = 0.8 + 0.2 * vignette;
    float2 tc = float2(i.tc * size.xy - offset_scale.xy) * scale;
    float2 l = frac(tc / 25);
    float s = min(2, scale);

    float2 res = smoothstep(1 - 3 * scale / 25, 1, abs(l * 2 - 1));
    float2 center = smoothstep(1 - 0.3 * scale, 1 - 0.1 * scale, 1 - 0.33 * abs(tc));

    float r = max(res.x, res.y);
    float r2 = max(center.x, center.y) * saturate(s / scale);
    return float4(vignette * float(20 + r * 5 / saturate(scale) + r2 * 20).xxx, 200) / 255;

}
#endif