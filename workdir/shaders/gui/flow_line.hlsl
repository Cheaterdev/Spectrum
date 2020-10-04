

struct VS_OUTPUT
{
float4 cpoint : SV_POSITION;
float4 color : COLOR;
};


struct HS_CONSTANT_OUTPUT
{
float edges[2] : SV_TessFactor;
};

struct HS_OUTPUT
{
float3 cpoint : CPOINT;
float4 color : COLOR;
};


struct DS_OUTPUT
{
float4 position : SV_Position;
float3 normal : NORMAL;
float4 color : COLOR;
};

struct GS_OUTPUT
{
float4 position : SV_Position;
float4 tc: TEXCOORD0;
float4 color : COLOR;
};

////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BUILD_FUNC_VS
#include "../autogen/LineRender.h"

VS_OUTPUT VS(uint id: SV_VertexID)
{
    VSLine input =  GetLineRender().GetVb()[id];
   
    VS_OUTPUT o;
    o.cpoint = float4(input.GetPos().x * 2 - 1, -(input.GetPos().y * 2 - 1), 0, 1);
    o.color = input.GetColor();
    return o;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BUILD_FUNC_HS
HS_CONSTANT_OUTPUT HSConst()
{
    HS_CONSTANT_OUTPUT output;
    output.edges[0] = 1.0f; // Detail factor (see below for explanation)
    output.edges[1] = 40.0f; // Density factor
    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HS_OUTPUT HS(InputPatch<VS_OUTPUT, 4> ip, uint id : SV_OutputControlPointID)
{
    HS_OUTPUT output;
    output.cpoint = ip[id].cpoint;
    output.color = ip[id].color;
    return output;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BUILD_FUNC_DS

[domain("isoline")]
DS_OUTPUT DS(HS_CONSTANT_OUTPUT input, OutputPatch<HS_OUTPUT, 4> op, float2 uv : SV_DomainLocation)
{
    DS_OUTPUT output;
    float t = uv.x;
    float2 pos = pow(1.0f - t, 3.0f) * op[0].cpoint + 3.0f * pow(1.0f - t, 2.0f) * t * op[1].cpoint + 3.0f * (1.0f - t) * pow(t, 2.0f) * op[2].cpoint + pow(t, 3.0f) * op[3].cpoint;
    output.position = float4(pos, 1, 1.0f);
    float t2 = uv.x;
    
    
    if(t>0.5) t2-=1.0 / 4000;
    if (t <=0.5) t2 += 1.0 / 4000;

    float2 delta1, delta2, pos2, pos3;
    pos2 = pow(1.0f - t2, 3.0f) * op[0].cpoint + 3.0f * pow(1.0f - t2, 2.0f) * t2 * op[1].cpoint + 3.0f * (1.0f - t2) * pow(t2, 2.0f) * op[2].cpoint + pow(t2, 3.0f) * op[3].cpoint;
 
    delta1 = normalize(pos - pos2);
    if (t <= 0.5)delta1 = -delta1;

    //	delta2=-normalize(pos-pos3);
    float2 delta = delta1;//normalize(delta1+delta2);
    output.normal = float3(delta.y, -delta.x, 0);
    output.color = pow(1.0f - t, 3.0f) * op[0].color + 3.0f * pow(1.0f - t, 2.0f) * t * op[1].color + 3.0f * (1.0f - t) * pow(t, 2.0f) * op[2].color + pow(t, 3.0f) * op[3].color;
    return output;
}
#endif


#include "../autogen/FlowGraph.h"

static const float4 size = GetFlowGraph().GetSize();
static const float4 offset_scale = GetFlowGraph().GetOffset_size();
static const float2 inv_pixel_size = GetFlowGraph().GetInv_pixel();




////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BUILD_FUNC_GS



[maxvertexcount(6)]
void GS(line DS_OUTPUT input[2], inout TriangleStream<GS_OUTPUT> TriStream)
{
    GS_OUTPUT o;
    float4 n1 = 5 * float4(inv_pixel_size * input[0].normal.xy, 0, 0);
    float4 n2 = 5 * float4(inv_pixel_size * input[1].normal.xy, 0, 0);
    o.position = input[0].position - n1; o.tc = float4(0, 0, 0, 0); o.color = input[0].color;  TriStream.Append(o);
   o.position = input[1].position + n2; o.tc = float4(1, 1, 0, 0); o.color = input[1].color;   TriStream.Append(o);
  o.position = input[0].position + n1; o.tc = float4(0, 1, 0, 0); o.color = input[0].color;   TriStream.Append(o);
   

    TriStream.RestartStrip();
    o.position = input[0].position - n1; o.tc = float4(0, 0, 0, 0); o.color = input[0].color;   TriStream.Append(o);
    o.position = input[1].position - n2; o.tc = float4(1, 0, 0, 0); o.color = input[1].color;   TriStream.Append(o);
    o.position = input[1].position + n2; o.tc = float4(1, 1, 0, 0); o.color = input[1].color;  TriStream.Append(o);
    TriStream.RestartStrip();
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BUILD_FUNC_PS


float4 PS(GS_OUTPUT i): SV_TARGET0
{
    float2 screen_tc = i.position.xy;
    //return float4(screen_tc/200, 0, 1);
//   clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

    float a = i.tc.y;

    //	float s = 1.4 - offset_scale.z;
    float s = min(2, offset_scale.z);
    a = smoothstep(1 / s - 0.8 * s, 1 / s, abs(a * 2 - 1));
    float4 r = i.color;
    r.a *= 1 - a;

    r.a *= saturate(s / offset_scale.z);
    return r;
}
#endif