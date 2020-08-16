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
    float4 size;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register(s0);

float4 PS(quad_output i) : SV_TARGET0
{
    //  return size;
    float2 screen_tc = i.pos.xy / i.pos.z;

    // clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

    float round = 6;
    float corners = 12;


    float2 cornered = corners;
    float corners_round = corners + round;

    float2 best_point = clamp(i.tc * size.xy, cornered.xx, size.xy - cornered.xx);

    float len = clamp(length(best_point - i.tc * (size.xy)) / round, 0, 1); // size.z;


    float2 corrected_tc = len < 1 ? (best_point - float2(corners_round.xx)) / (size.xy - 2 * corners_round.xx) : 0;
    //return float4(len.xxx, 1);
    float alpha = 1 - smoothstep(round / 20, round / 20 + 0.1, len);
    float shadow = 1 - smoothstep(0, 1, len);

    float3 result = 0.2;


    float4 window_color = float4(result, alpha);
    float4 shadow_color = float4(0, 0, 0, shadow.x / 3);

    return shadow_color * (1 - window_color.a) + window_color * window_color.a;


}
#endif