

Texture2D<float4> full_tex:register(t0);

SamplerState LinearSampler: register(s0);

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

float4 PS(quad_output i):SV_TARGET0
{
  return float4(pow(full_tex.SampleLevel(LinearSampler, i.tc,0).rgb,1.0/2.2),1);
}
