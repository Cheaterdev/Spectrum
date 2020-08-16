#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float2 tc: TEXCOORD0;
};

#if BUILD_FUNC == VS
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

#if BUILD_FUNC == PS
cbuffer cbParams : register(b0)
{
    float4 color1;
    float4 color2;
    float4 color_back;
    float4 size;
    float4 clip_size;
    float4 properties;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register(s0);

float4 PS(quad_output i) : SV_TARGET0
{
    float2 screen_tc = i.pos.xy / i.pos.z;

    clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

    float round = 5;
    float corners = properties.x;
    float header_size = properties.y + round * 1;
    float corners_round = corners + round;

    float2 best_point = clamp(i.tc * size.xy, corners_round.xx, size.xy - corners_round.xx);
    float len = clamp(length(best_point - i.tc * (size.xy)) / corners, 0, 1); // size.z;


    float2 corrected_tc = len < 1 ? (best_point - float2(corners_round.xx)) / (size.xy - 2 * corners_round.xx) : 0;
    //	return float4(len.xxx, 1);
    float t = 0.03;

    float alpha = 1 - smoothstep(0.2 - t, 0.2 + t, len);
    t = 0.1;
    float alpha2 = 1 - smoothstep(0.5 - t, 0.5, len);

    float shadow = 1 - smoothstep(0, 1, len);

    float stage = corrected_tc.y / (header_size - round) * (size.y - corners_round * 2);

    float3 result = stage < 1 ? lerp(color1.xyz, color2.xyz, stage) : color_back;

    float shadow2 = stage < 1 ? 1 : (0.7 + 0.3 * saturate((stage - 1) * 5));
    //	shadow2 = saturate(shadow2*shadow2 + 0.3*abs(corrected_tc.x-0.5)*2);

    float4 window_color = float4(shadow2.xxx * result, alpha);
    window_color.xyz = lerp(window_color.xyz * 0.7, window_color.xyz, alpha2);

    float4 shadow_color = float4(0, 0, 0, shadow.x / 3);

    return shadow_color * (1 - window_color.a) + window_color * window_color.a; // lerp(float4(0, 0, 0, shadow.x / 4), float4(shadow2.xxx*result, 1), alpha);

}
#endif