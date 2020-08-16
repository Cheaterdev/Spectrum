#define SIZE 16
#define FIX 8

struct camera_info
{
    matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_proj;
    matrix inv_view_proj;
    float3 position;
    float3 direction;
};


cbuffer cbCamera : register(b0)
{
    camera_info camera;
};

cbuffer cbMip : register(b1)
{
    float global_time;
};


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
    return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc, d, 1));
    return P.xyz / P.w;
}

Texture2D<float4> color_tex[2] : register(t0);
Texture2D<float4> speed_tex : register(t0,space1);

SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);


struct quad_output
{
float4 pos : SV_POSITION;
float2 tc : TEXCOORD0;
};

quad_output VS(uint index : SV_VERTEXID)
{
    static float2 Pos[] =
    {
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1)
    };
    static float2 Tex[] =
    {

        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
    };
    quad_output Output;
    Output.pos = float4(Pos[index], 0.3, 1);
    Output.tc = Tex[index];
    return Output;
}




float4 PS(quad_output i): SV_Target0
{


    float2 dims;
    color_tex[0].GetDimensions(dims.x, dims.y);

    float2 delta       = i.tc - 0.5;
    float vignette  = 1 - dot(delta, delta);

    float2 speed =  speed_tex.Sample(PixelSampler, i.tc).xy;

	//return float4(speed * 1, 0, 1);
#define COUNT 1
    float4 color = 0;
 

//	for(int j=0;j<COUNT;j++)
	color = color_tex[0].Sample(LinearSampler, i.tc);
   // color += color_tex[1].Sample(PixelSampler, i.tc);
 //color /= 2;
 // color.xyz*=vignette;wqq
 //rcolor = float4(speed, 0, 1);

    float x = abs(i.tc.x * i.tc.y * (global_time * 1000));
    x = fmod(x, 13) * fmod(x, 123);
    float dx = fmod(x, 0.01);
    float Y = 1 - saturate(1 * dot(2 * color, (1.0f / 3).xxx)); 
    float Noise = 1 + Y * (dx - 0.005) * 64;

    color = saturate(pow(vignette, 4) * (color * Noise));


    color = pow(color, 1.0 / 2.2);

    return float4(color.xyz, 1);
}
