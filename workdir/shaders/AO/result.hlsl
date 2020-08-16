
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


cbuffer cbCamera : register(b1)
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

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_tex : register(t3);
Texture2D<float> ao_tex : register(t5);
TextureCube<float4> cube_tex : register(t6);


SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);


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
float calc_fresnel(float k0, float3 n, float3 v)
{
    float ndv = saturate(dot(n, -v));
    return k0 + (1 - k0) * pow(1 - ndv, 5);
    return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}


float4 PS(quad_output i): SV_TARGET0
{
float4 albedo = gbuffer[0].Sample(PixelSampler,i.tc);
float ao = pow(ao_tex.Sample(PixelSampler,i.tc).x,2);
float3 normal = gbuffer[1].Sample(PixelSampler,i.tc)*2-1;
float specular = gbuffer[2].Sample(PixelSampler,i.tc).w;

float4 sky = cube_tex.SampleLevel(LinearSampler,normal,8);

float raw_z=depth_tex.Sample(PixelSampler,i.tc);
 float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
							   
 float3 v = -normalize(camera.position - pos);
    float fresnel = 1-calc_fresnel(specular, normal, v);
	
	
    return 0;//float4(fresnel*sky*albedo.xyz*ao,albedo.w);
}
