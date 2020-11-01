struct vertex_output
{
float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

#if BUILD_FUNC == VS
cbuffer cbCamera : register(b0)
{
    matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_view_proj;
    float3 position;
    float3 direction;
};

struct instance_data
{
    matrix node_global_matrix;
    float4 vmin;
    float4 vmax;
    float4 color;
};
/*
cbuffer cbNode : register(b1)
{
    instance_data instances[64]; 
};*/


StructuredBuffer<instance_data> instances : register(t0);
//static const float3 data.vmin = float3(0, 0, 0);
//static const	float3 data.vmax = float3(5, 5, 5);
float3 get_position(uint v_index, uint i_index)
{
    instance_data data = instances[i_index];

    switch (v_index)
    {
    case 0:
        return  float3(data.vmin.x, data.vmin.y, data.vmin.z);

    case 1:
        return  float3(data.vmin.x, data.vmin.y, data.vmax.z);

    case 2:
        return  	float3(data.vmin.x, data.vmin.y, data.vmin.z);

    case 3:
        return  	float3(data.vmax.x, data.vmin.y, data.vmin.z);

    case 4:
        return  	float3(data.vmax.x, data.vmin.y, data.vmax.z);

    case 5:
        return  	float3(data.vmax.x, data.vmin.y, data.vmin.z);

    case 6:
        return  	float3(data.vmax.x, data.vmin.y, data.vmax.z);

    case 7:
        return  	float3(data.vmin.x, data.vmin.y, data.vmax.z);

    case 8:
        return  	float3(data.vmin.x, data.vmax.y, data.vmin.z);

    case 9:
        return  	float3(data.vmin.x, data.vmax.y, data.vmax.z);

    case 10:
        return  	float3(data.vmin.x, data.vmax.y, data.vmin.z);

    case 11:
        return  	float3(data.vmax.x, data.vmax.y, data.vmin.z);

    case 12:
        return  	float3(data.vmax.x, data.vmax.y, data.vmax.z);

    case 13:
        return  	float3(data.vmax.x, data.vmax.y, data.vmin.z);

    case 14:
        return  	float3(data.vmax.x, data.vmax.y, data.vmax.z);

    case 15:
        return  	float3(data.vmin.x, data.vmax.y, data.vmax.z);

    case 16:
        return  	float3(data.vmin.x, data.vmin.y, data.vmin.z);

    case 17:
        return  	float3(data.vmin.x, data.vmax.y, data.vmin.z);

    case 18:
        return  	float3(data.vmin.x, data.vmin.y, data.vmax.z);

    case 19:
        return  	float3(data.vmin.x, data.vmax.y, data.vmax.z);

    case 20:
        return  	float3(data.vmax.x, data.vmin.y, data.vmin.z);

    case 21:
        return  	float3(data.vmax.x, data.vmax.y, data.vmin.z);

    case 22:
        return  	float3(data.vmax.x, data.vmin.y, data.vmax.z);

    case 23:
        return  	float3(data.vmax.x, data.vmax.y, data.vmax.z);

	default:

		return float3(0, 0, 0);
    }

    return float3(0, 0, 0);
}

vertex_output VS(uint v: SV_VertexID, uint i : SV_InstanceID)
{
    vertex_output o;
    o.pos = 0;
    matrix mat_mvp = mul(view_proj, instances[i].node_global_matrix);
    o.pos = mul(mat_mvp, float4(get_position(v, i), 1));
    o.color = instances[i].color;
    return o;
}
#endif

#if BUILD_FUNC == PS
float4 PS(vertex_output i) : SV_TARGET0
{
    return i.color*float4(1,1,1,0.2);
}
#endif