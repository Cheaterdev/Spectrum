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
        
      /*  float2(-1, 1),
       
        float2(-1, -1), float2(1, 1),
        float2(1, -1)*/
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

//#ifdef BUILD_FUNC_PS


Texture2D<float> residency_tex:  register(t0);
Texture2D<float4> tex: register(t1);
SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);
RWByteAddressBuffer visibility_tex : register(u0);

float4 tile_sample(Texture2D<float4> tex, SamplerState s, float2 tc, Texture2D<float> residency, RWByteAddressBuffer visibility )
{
    float calculated_level = tex.CalculateLevelOfDetail(s, tc);
    float2 dims;
    residency.GetDimensions(dims.x, dims.y);


    float4 res_data = residency.Gather(PointSampler, tc) * 255;
    float min_level = max(res_data.x, res_data.y);
    min_level = max(min_level, res_data.z);
    min_level = max(min_level, res_data.w);

    int offset = 4 * (floor(tc.x * dims.x) + floor(tc.y * dims.y + 0.5) * dims.x);
    visibility.InterlockedMin(offset, int(calculated_level));

    return tex.SampleLevel(s, tc, max(calculated_level, min_level));
}

float4 PS(quad_output i) : SV_TARGET0
{
    return tile_sample(tex, LinearSampler, i.tc, residency_tex, visibility_tex);
}
//#endif