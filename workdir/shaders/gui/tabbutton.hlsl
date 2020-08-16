#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float2 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_VS
cbuffer cbParams : register(b0)
{
    float4 pos;
};

quad_output VS(uint index : SV_VERTEXID)
{
    static float2 Pos[] =
    {
        float2(pos.x, pos.w),
        float2(pos.x, pos.y),
        float2(pos.z, pos.w),
        float2(pos.z, pos.y)
        /*
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1),*/
    };
    static float2 Tex[] =
    {


        float2(0, 1),
        float2(0, 0),
        float2(1, 1),
        float2(1, 0),
    };
    quad_output Output;
    Output.pos = float4(Pos[index].x * 2 - 1, -(Pos[index].y * 2 - 1), 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
    Output.tc = Tex[index];
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS
cbuffer cbParams : register(b0)
{
    float4 color1;
    float4 color2;
    float4 size;
    float4 clip_size;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register(s0);

float4 PS(quad_output i) : SV_TARGET0
{

    float2 screen_tc = i.pos.xy / i.pos.z;

    // clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

    float corners = 4;

    float2 best_point = clamp(i.tc * size.xy, corners, size.xy - corners);
    best_point.y = max(i.tc.y * size.y, best_point.y);
    float len = clamp(length(best_point - i.tc * size.xy) / (corners), 0, 1); // size.z;

    float alpha = 1 - smoothstep(0.60, 0.77, len);
    float shadow = 1 - smoothstep(0, 1, len);

    float3 result = lerp(color1.xyz, color2.xyz, i.tc.y);
    result = lerp(0, result, alpha);

    float2 shadow_mask = min(i.tc * size.xy, corners.xx * 5) / corners.x / 5;

    return float4(result, alpha + (1 - color1.a) * shadow);

}
#endif