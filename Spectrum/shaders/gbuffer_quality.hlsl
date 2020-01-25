

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




struct camera_info
{
    matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_proj;
    matrix inv_view_proj;
    float3 position;
    float3 direction;
};


cbuffer cbCamera : register(b0)
{
    camera_info camera;
};


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
    return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc, d, 1));
    return P.xyz / P.w;
}

float2 project_tc(float3 pos, matrix mat)
{
    float4 res = mul(mat, float4(pos, 1));
    res /= res.w;
    return res.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
}

Texture2D<float3> normals_buffer : register(t0);
Texture2D<float> depth_buffer : register(t1);
Texture2D<float2> quality_tex : register(t3);

SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);


static const int2 delta2[4] =
{
	int2(0, 1),
	int2(1, 1),
	int2(1, 0),
	int2(0, 0),

};


float4 PS(quad_output i):SV_Target0
{
    float2 dims;
normals_buffer.GetDimensions(dims.x, dims.y);

float3 low_normals = normalize(normals_buffer.SampleLevel(PixelSampler, i.tc, 1).xyz * 2 - 1);
float low_depth = depth_buffer.SampleLevel(PixelSampler, i.tc, 1);

float3 p2 = depth_to_wpos(low_depth, i.tc.xy, camera.inv_view_proj);


float result_color = 1;
float result_reflection = 1;

float2 full_tc = i.tc -float2(0.5, 0.5) / dims;

float3 v = normalize(p2 - camera.position);
float3 r = normalize(reflect(v, low_normals));




#define R 1

[unroll] for (int x = -R; x <= R; x++)
[unroll] for (int y = -R; y <= R; y++)
{
	//float2 t_tc = i.tc + 2 * float2(x, y) / dims;
	int2 deltas = int2(x, y);

	float3 full_normals = normalize(normals_buffer.SampleLevel(PixelSampler, full_tc, 0, deltas).xyz * 2 - 1);
	float full_depth = depth_buffer.SampleLevel(PixelSampler, full_tc, 0, deltas);



	float3 p1 = depth_to_wpos(full_depth, i.tc.xy, camera.inv_view_proj);
	float3 delta = p1 - p2;


	float3 _v = normalize(p1 - camera.position);
	float3 _r = normalize(reflect(_v, full_normals));



	float my_result = 1;
	float pos_w = length(delta) <  (length(p2 - camera.position)*0.02);
float normal_w =  pow(saturate(dot(full_normals, low_normals)), 32);
float refl_w = pow(saturate(dot(r, _r)), 256);

	

result_color =min(result_color, pos_w*normal_w);
result_reflection = min(result_reflection, pos_w*refl_w);


}
	return float4(result_color, result_reflection,0,0);

}



void PS_STENCIL(quad_output i)
{
	clip(0.05-quality_tex.SampleLevel(PixelSampler, i.tc, 0).x);
}

void PS_STENCIL_REFL(quad_output i)
{
	//discard;
	clip(0.05 - quality_tex.SampleLevel(PixelSampler, i.tc, 0).y);
}
