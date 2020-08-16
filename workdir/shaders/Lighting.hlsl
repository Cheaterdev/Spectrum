#define SIZE 16
#define THREAD_COUNT (SIZE*SIZE)
#define MAX_LIGHTS_POWER 10
#define MAX_LIGHTS (1<<MAX_LIGHTS_POWER)
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

struct Light
{
    float4 color;
    float4 pos;

};

cbuffer buff: register(b0)
{
    camera_info camera;
};

cbuffer buff2: register(b1)
{
    int totalLights;
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
StructuredBuffer<Light> light_buffer: register(t5);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);


float calc_vignette(float2 inTex)
{
    float2 T = saturate(inTex);
    float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
    return vignette;
}

groupshared uint depth_min;
groupshared uint depth_max;

groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;


float calc_fresnel(float k0, float3 n, float3 v)
{
    float ndv = dot(n, -v);
    return k0 + (1 - k0) * pow(1 - ndv, 5);
    return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}


float3 calc_color(float3 albedo, float3 normal, float4 specular, float3 r, float3 v, float3 light_dir, float3 light_color, float attentuation, float l)
{
    float light = max(0, dot(light_dir, normal.xyz));
    float fresnel = calc_fresnel(specular.w, normal, v);
    float s = pow(max(0, dot(r, light_dir)), 64 * fresnel);
    float3  res_color = attentuation * albedo * max(0, dot(light_dir, normal));
  //  res_color += l * fresnel * 1 * s * specular.xyz;
    return 1 * light_color;
}
 

[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    const uint groupIndex = thread_id.y * SIZE + thread_id.x;
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    float2 dims;
    gbuffer[0].GetDimensions(dims.x, dims.y);
    bool good = all(uint2(dims) > tc);
    uint listIndex = 0;

    //  [unroll]for (int i = 0; i < 2; i++)
    //    InterlockedAdd(sTileNumLights, 1, listIndex);

    //   if (dims.x < tc.x) return;
    //  if (dims.y < tc.y) return;
    if (groupIndex == 0)
    {
        sTileNumLights = 0;
        depth_min = 0x7F7FFFFF;      // Max float
        depth_max = 0;
    }

    GroupMemoryBarrierWithGroupSync();
    float raw_z = depth_buffer[tc.xy];
    float3 pos = depth_to_wpos(raw_z, float2(tc.xy) / dims, camera.inv_view_proj);
    float view_z = camera.proj._34 * raw_z / (raw_z - camera.proj._33);

    // GroupMemoryBarrierWithGroupSync();

    if (good)
    {
        InterlockedMin(depth_min, asuint(view_z));
        InterlockedMax(depth_max, asuint(view_z));
    }

    GroupMemoryBarrierWithGroupSync();
    float minTileZ = asfloat(depth_min);
    float maxTileZ = asfloat(depth_max);
    float2 tileScale = float2(dims.xy) * rcp(float(2 * SIZE));
    float2 tileBias = tileScale - float2(group_id.xy);
    float4 c1 = float4(camera.proj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -camera.proj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);
    // Derive frustum planes
    float4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, maxTileZ);
    // Normalize frustum planes (near/far already normalized)

    [unroll] for (uint i = 0; i < 4; ++i)
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));

    uint lightIndex = 0;
    // Cull lights for this tile

    for (lightIndex = groupIndex; lightIndex < totalLights; lightIndex += THREAD_COUNT)
    {
        Light light = light_buffer[lightIndex];
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        float4 view_pos = mul(camera.view, float4(light.pos.xyz, 1));
        view_pos /= view_pos.w;

        [unroll] for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustumPlanes[i], view_pos);
            inFrustum = inFrustum && (d >= -light.pos.w);
        }

        [branch] if (inFrustum)
        {
            // Append light to list
            // Compaction might be better if we expect a lot of lights
            uint listIndex = 0;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    float4 albedo = gbuffer[0][tc.xy];
    float4 normal = gbuffer[1][tc.xy] * 2 - 1;
    float4 specular = gbuffer[2][tc.xy];
    float3 v = -normalize(camera.position - pos);
    float3 r = reflect(v, normal);
    float3 res_color = 0;
    uint light_count = sTileNumLights;

    for (uint tileLightIndex = 0; tileLightIndex < light_count; ++tileLightIndex)
    {
        Light light = light_buffer[sTileLightIndices[tileLightIndex]];
        float3 light_dir = normalize(light.pos.xyz - pos);
        float ndl = max(0, dot(light_dir, normal.xyz));
        float light_dist = distance(light.pos.xyz, pos);

        if (light_dist > light.pos.w) continue;

        float l = 1 - pow(light_dist / light.pos.w, 2);
        float denom = 1 / (l * light.color.w) + 1;
        float attenuation = 1 / (denom * denom);
        res_color += calc_color(albedo, normal, specular, r, v, light_dir, light.color , attenuation, l);
    }

    result[tc.xy] = float4(res_color, 10);
    //    GroupMemoryBarrierWithGroupSync(); 
    return ;
    /*
     float2 reflect_tc = tc;
     float3 refl_pos = pos;
     float3 dist = distance(camera.position, pos) / 2;

     for (int i = 0; i < 4; i++)
     {
         reflect_tc = project_tc(pos + dist * r, camera.view_proj);
         refl_pos = depth_to_wpos(depth_buffer.SampleLevel(LinearSampler, reflect_tc, 0).x, float2(reflect_tc.xy), camera.inv_view_proj);
         dist = distance(pos, refl_pos);
     }

     float goodness = calc_vignette(reflect_tc) *
                      (dot(refl_pos - pos, r) > 0.9) *
                      (distance(pos + dist * r, refl_pos) < 2)
                      ;
     float fresnel = calc_fresnel(specular.w, normal, v);
     result[tc.xy] = albedo;//float4(specular.xyz * fresnel * goodness * gbuffer[0].SampleLevel(LinearSampler, reflect_tc, 0).xyz, 1);
     //result[tc.xy] = float4(fresnel * goodness * gbuffer[0].SampleLevel(LinearSampler, reflect_tc, 0).xyz, 1);*/
}




