#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float2 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_VS

quad_output VS(uint index : SV_VERTEXID)
{
    static float2 Pos[] =
    {       
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1),
    };
    static float2 Tex[] =
    {


        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
    };
    quad_output Output;
    Output.pos = float4(Pos[index].x, Pos[index].y, 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
    Output.tc = Tex[index];
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS

Texture2D<float> tex : register(t0,space2);

SamplerState PointSampler : register(s0);
float PS(quad_output i) : SV_Depth
{

    return tex.SampleLevel(PointSampler, i.tc,0);
}
#endif