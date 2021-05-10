struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc: TEXCOORD0;
};

struct CB
{
uint id;
uint rw_id;
};

ConstantBuffer<CB> cb: register (b0);
SamplerState PointSampler : register (s0);

struct A
{
	Texture2D<float4> getTexture() { return ResourceDescriptorHeap[cb.id]; }
	RWTexture2D<float4> getRWTexture() { return ResourceDescriptorHeap[cb.rw_id]; }

};


A createA()
{

A a;

return a;
}

static const A a = createA();


A getA()
{
return a;
}



float4 PS(quad_output i) : SV_TARGET0
{
    return a.getTexture().Sample(PointSampler, i.tc);
}




[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
   getA().getRWTexture()[groupThreadID.xy] =   getA().getTexture().Sample(PointSampler, groupThreadID.xy);
}