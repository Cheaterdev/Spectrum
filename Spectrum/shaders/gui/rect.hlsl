#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
};

#ifdef BUILD_FUNC_VS
cbuffer cbParams : register(b0)
{
    float4 pos;
};

quad_output VS(float2 pos: SV_POSITION, uint index : SV_VERTEXID)
{
    quad_output Output;
    Output.pos = float4(pos, 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS_COLOR
cbuffer cbParams : register(b0)
{
    float4 color;
};


SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);
SamplerState UISampler : register(s2);

float4 PS_COLOR(quad_output i) : SV_TARGET0
{
    return color;
}
#endif

