struct t {

    float a;
    float2 b;
    float4 c;
    float d;
};

ConstantBuffer<t> data;
float4 VS(uint index : SV_VertexID) :SV_POSITION
{
 return data.c;
}

