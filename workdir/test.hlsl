
struct A
{
    RWTexture2D<float4> tex_array0;
    RWTexture2D<float4> tex_array1;
    RWTexture2D<float4> tex_array2;
};

A create()
{
    A a;
    a.tex_array0 = ResourceDescriptorHeap[0];
    a.tex_array1 = ResourceDescriptorHeap[1];
    a.tex_array2 = ResourceDescriptorHeap[2];

    return a;
}

static const A a = create();

[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    a.tex_array0[groupThreadID.xy] = 1;
}



struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc: TEXCOORD0;
};
/*
struct CB
{
uint id;
uint rw_id;
};

ConstantBuffer<CB> cb: register (b0);
SamplerState PointSampler : register (s0);

*/

float4 PS(quad_output i) : SV_TARGET0
{
    return 1;
}


