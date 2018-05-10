#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
float4 pos : SV_POSITION;
float2 tc: TEXCOORD0;
float texture_offset : TEXCOORD1;
};

#ifdef BUILD_FUNC_VS
cbuffer cbParams : register(b0)
{
    float4 pos;
};

struct vertex_input
{
	float2 pos : POSITION;
	float2 tc : TEXCOORD;
};

StructuredBuffer<vertex_input> vb: register(t0, space1);
//StructuredBuffer<uint> texture_offset: register(t1, space1);


quad_output VS(uint index : SV_VERTEXID, uint instance : SV_INSTANCEID)
{

	vertex_input input = vb[16 * instance + index];
    quad_output Output;
    Output.pos = float4(input.pos, 0.99999, 1);
    Output.tc = input.tc;
	Output.texture_offset = instance;// texture_offset[instance];
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
SamplerState LinearSampler : register(s1);
SamplerState UISampler : register(s2);
Texture2D textures[] : register(t0, space1);


float4 PS(quad_output i) : SV_TARGET0
{
	return   textures[i.texture_offset].Sample(PointSampler, i.tc);
}
#endif

