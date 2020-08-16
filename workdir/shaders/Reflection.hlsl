#define SIZE 8

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

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture2D<float4> light_buffer : register(t4);
TextureCube<float4> cube_probe : register(t5);
Texture2D<float> ambient_texture : register(t6);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);

float calc_fresnel(float k0, float3 n, float3 v)
{
    float ndv = saturate(dot(n, -v));
    return k0 + (1 - k0) * pow(1 - ndv, 5);
    return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float calc_vignette(float2 inTex)
{
    float2 T = saturate(inTex);
    float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
    return vignette;
}
float rnd(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

//groupshared float3 positions[SIZE][SIZE];
//groupshared float3 reflects[SIZE][SIZE];
//groupshared float roughness[SIZE][SIZE];
groupshared float result_dist[SIZE / 2][ SIZE / 2];

[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    int thread_index = thread_id.x + thread_id.y * SIZE;
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    const uint2 res_tc = (group_id.xy * SIZE + thread_id.xy);
    float2 dims;
    gbuffer[0].GetDimensions(dims.x, dims.y);
    //  if (dims.x < tc.x) return;
    // if (dims.y < tc.y) return;
    float2 ftc = tc / dims;
    float4 back = light_buffer[tc];
    float4 albedo = gbuffer[0][tc];
    float3 normal = gbuffer[1][tc].xyz * 2 - 1;
    float4 specular = gbuffer[2][tc];
    //  if (length(specular.xyz) < 0.01) { result[tc.xy] = back; return; }
    float raw_z = depth_buffer[tc];
    float3 pos = depth_to_wpos(raw_z, ftc,
                               camera.inv_view_proj);
    float3 v = -normalize(camera.position - pos);
    float3 r = reflect(v, normal);
    float3 refl_pos = pos;
    float sini = sin(float(tc.x));
    float cosi = cos(float(tc.y));
    float rand = 1;// rnd(float2(sini, cosi)) * 0.2 + 0.8;
    float errorer = distance(camera.position, pos) / 1;
    float3 dist = rand * distance(camera.position, pos) / 10;
    float fresnel = calc_fresnel(specular.w, normal, v);
    //  positions[thread_id.x][thread_id.y] = pos;
    //  reflects[thread_id.x][thread_id.y] = r;
    // roughness[thread_id.x][thread_id.y] = specular.w;
    // GroupMemoryBarrierWithGroupSync();
    float raw_z_reflected = 0;
    float2 reflect_tc = tc;
    float level = 0;/*
#define CLIPPER 2

    if (thread_id.x % CLIPPER == 0 && thread_id.y % CLIPPER == 0)
    {
        reflect_tc = project_tc(pos + dist * r, camera.view_proj);

        for (int i = 0; i < 4; i++)
        {
            raw_z_reflected = depth_buffer.SampleLevel(PixelSampler, reflect_tc, clamp(level / 2, 2, 5)).x;
            refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);
            dist = distance(pos, refl_pos);
            level = dist * (1 - fresnel) / 4;
            reflect_tc = project_tc(pos + dist * r, camera.view_proj);
        }

        result_dist[thread_id.x / CLIPPER][thread_id.y / CLIPPER] = dist;
    }

    GroupMemoryBarrierWithGroupSync();

    if (!(thread_id.x % CLIPPER == 0 && thread_id.y % CLIPPER == 0))
    {
        reflect_tc = project_tc(pos +  result_dist[thread_id.x / CLIPPER][thread_id.y / CLIPPER] * r, camera.view_proj);

        for (int i = 0; i < 2; i++)
        {
            raw_z_reflected = depth_buffer.SampleLevel(PixelSampler, reflect_tc, clamp(level / 2, 1, 5)).x;
            refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);
            dist = distance(pos, refl_pos);
            level = dist * (1 - fresnel) / 4;
            reflect_tc = project_tc(pos + dist * r, camera.view_proj);
        }
    }
	*/

    for (int i = 0; i < 4; i++)
    {
        reflect_tc = project_tc(pos + dist * r, camera.view_proj);
        raw_z_reflected = depth_buffer.SampleLevel(PixelSampler, reflect_tc, clamp(level / 2, 1, 5)).x;
        refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);
        dist = distance(pos, refl_pos);
        level = dist * (1 - fresnel) / 4;
    }

    float3 delta_position = pos - refl_pos;
    float reflection_distance = distance(pos + dist * r, refl_pos);
    dist = distance(pos, refl_pos);
    float vignette = calc_vignette(reflect_tc);
    float goodness = vignette
                     * (raw_z_reflected < 0.9999)
                     * (raw_z_reflected > raw_z)
                     * (dot(r, v) > 0)
                     * (reflection_distance < errorer);
    //float3 cube_color = fresnel * ao * (specular.xyz) * cube_probe.SampleLevel(LinearSampler, r, (1 - fresnel) * 8) * pow(1 - goodness, 8);
    //  vignette *= saturate(dot(r, v)*3);
    float bad = pow(saturate((dot(r, -delta_position) / dist)), 128);
    bad *= saturate(dot(r, v) * 5);
    bad *= saturate(1 - 2 * reflection_distance / errorer);
    bad *= (raw_z_reflected < 0.9999);
    level = lerp(level, 8, saturate(1 - 1 * bad));
    float4 res = 0;
    res += back;
    float3 reflect_color = 0;
    float s = 0;
    // for (int i = -0; i <= 0; i++)
    //     for (int j = -0; j <= 0; j++)
    {
        s++;
        reflect_color += light_buffer.SampleLevel(LinearSampler, reflect_tc, level).xyz;
    }
    //res += float4(cube_color / 1, 1);
    res += float4(specular.xyz * fresnel * reflect_color / s * pow(0.95, level), 1);
    //+ /*1*float4(cube_color / 10, 1)+*/float4(specular.xyz * vignette * fresnel * light_buffer.SampleLevel(LinearSampler, reflect_tc, level).xyz, 1);
    // float4 res =float4( goodness *  light_buffer.SampleLevel(LinearSampler, reflect_tc, 0).xyz, 1);
    // res = float4(reflect_color / s, 1);
    result[res_tc.xy] = float4(res.xyz, 1);
    //  result[tc.xy] = float4(ao.xxx, 1);
//   result[tc.xy] = float4(bad.xxx, 1);
}




