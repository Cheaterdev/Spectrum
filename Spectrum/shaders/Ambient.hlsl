#define SIZE 16
#define FIX 8

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

cbuffer cbMip : register(b1)
{
    uint current_mip;
    uint2 dims;
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

Texture2D<float4> gbuffer[2] : register(t0);
Texture2D<float> depth_mips[3] : register(t2);

//Texture2D<float> depth_mips[3] : register(t5);

//Texture2D<float4> back : register(t7);



//RWTexture2D<float4> result: register(u0);



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




float4 PS(quad_output i): SV_Target0
{


    float raw_z = depth_mips[current_mip].SampleLevel(PixelSampler, i.tc, 0).x;
    float3 p = depth_to_wpos(raw_z, i.tc.xy, camera.inv_view_proj);
    float3 normal = gbuffer[1].SampleLevel(PixelSampler, i.tc.xy, current_mip) * 2 - 1;

    float ao_dist = distance(camera.position, p);
    float ao = 0;
    float counter = 0.0;

    for (int x = -3; x <= 3; x++)
        for (int y = -3; y <= 3; y++)
        {
            //   if (x == y&&x == 0)
            //     continue;
            float2 sample_tc = i.tc + float2(x, y) / dims;
            float3 sample_pos = depth_to_wpos(depth_mips[current_mip].SampleLevel(PixelSampler, i.tc, 0, int2(x, y)).x, sample_tc, camera.inv_view_proj);
            float dist = distance(sample_pos , p);
            float tc_weight = 8 - length(float2(x, y));
            float w = tc_weight * saturate(1 - 10 * dist / (ao_dist));
            ao += saturate(1.5 * dot(normal, (sample_pos - p) / dist) - 0.5) * w;
            counter += w;
        }

    return float4(1 - ao.xxx / (counter), 1);
}

static const int2 delta[4] =
{
    int2(0, 1),
    int2(1, 1),
    int2(1, 0),
    int2(0, 0)
};



Texture2D<float4> ao_mips[3] : register(t5);
TextureCube<float4> cube_probe : register(t8);


float upsample(int level, float3 pos, float3 normal, float2 tc)
{
    float2 low_dimensions;
    ao_mips[level].GetDimensions(low_dimensions.x, low_dimensions.y);
    float raw_z[4] = (float[4]) depth_mips[level].GatherRed(PixelSampler, tc);
    float low_res[4] = (float[4]) ao_mips[level].GatherRed(PixelSampler, tc);
    float3 normals[4];

    for (int j = 0; j < 4; j++)
        normals[j] = gbuffer[1].SampleLevel(PixelSampler, tc, level, delta[j]).xyz * 2 - 1;

    float dist_err = length(pos - camera.position) / 120;
    float combinedAO = 0;
    float totalWeight = 0.0;
    float2 fract = frac(tc * low_dimensions + 0.5);

    for (int i = 0; i < 4; ++i)
    {
        float3 p = depth_to_wpos(raw_z[i], tc.xy + delta[i] / low_dimensions, camera.inv_view_proj);
        float pos_weight = 1;
        ;
        saturate(1 - 0.01 * distance(p, pos) / dist_err);
        float norm_weight = 1 - saturate(dot(normal, (p - pos) / dist_err));
        // norm_weight *= pow(saturate(dot(normal, normals[i])), 1);
        // float norm_weight = pow(saturate(dot(normal, normals[i])), 3);
        float2 bilinear = (1 - abs(fract - delta[i]));
        float weight = norm_weight * pos_weight * bilinear.x * bilinear.y + 0.001;
        totalWeight += weight;
        combinedAO += low_res[i] * weight;
    }

    combinedAO /= totalWeight;
    return (combinedAO);
}


struct PS_RESULT
{
float ao : SV_TARGET0;
float4 ambient : SV_TARGET1;
};
float calc_fresnel(float k0, float3 n, float3 v)
{
    float ndv = saturate(dot(n, -v));
    return k0 + (1 - k0) * pow(1 - ndv, 5);
    return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}


PS_RESULT PS_LAST(quad_output i)
{
    float2 dims;
    gbuffer[0].GetDimensions(dims.x, dims.y);
    float raw_z = depth_mips[0].SampleLevel(PixelSampler, i.tc, 0).x;
    float3 p = depth_to_wpos(raw_z, i.tc.xy, camera.inv_view_proj);
    float4 raw_n = gbuffer[1].SampleLevel(PixelSampler, i.tc.xy, 0);
    float3 normal = raw_n.xyz * 2 - 1;
    float mip1 = upsample(1, p, normal, i.tc);
    float mip2 = upsample(2, p, normal, i.tc);
    float3 albedo = gbuffer[0].SampleLevel(PixelSampler, i.tc.xy, 0);
    float3 v = -normalize(camera.position - p);
    float fresnel = calc_fresnel(raw_n.w, normal, v);
    float3 cube_ambient = albedo.xyz * cube_probe.SampleLevel(LinearSampler, normal, 8);
    PS_RESULT result;
    result.ao = float4(pow(mip1 * mip2.xxx, 2), 1);
    result.ambient = 0*float4((1 - fresnel) * result.ao * cube_ambient, 1);
    return result;
    //;
}