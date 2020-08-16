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

cbuffer cbLight : register(b1)
{
    camera_info light_cam;
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
Texture2D<float> depth_buffer : register(t3);
Texture2D<float> cam_buffer : register(t4);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];

float calc_fresnel(float k0, float3 n, float3 v)
{
    float ndv = dot(n, -v);
    return k0 + (1 - k0) * pow(1 - ndv, 5);
    return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}


float3 calc_color(float3 albedo, float3 normal, float4 specular, float3 r, float3 v, float3 light_dir, float3 light_color)
{
    float light = max(0, dot(light_dir, normal.xyz));
    float fresnel = calc_fresnel(specular.w, normal, v);
    float s = pow(max(0, dot(r, light_dir)), 8 * fresnel);
    float3  res_color = albedo * max(0, dot(light_dir, normal));
    res_color += fresnel * 5 * s * specular.xyz;
    return res_color;
}
[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    float2 dims;
    // uint dummy;
    gbuffer[0].GetDimensions(dims.x, dims.y);

    if (dims.x < tc.x) return;

    if (dims.y < tc.y) return;

    float2 shadow_dims;
    // uint dummy;
    cam_buffer.GetDimensions(shadow_dims.x, shadow_dims.y);
    float4 albedo = gbuffer[0][tc.xy];
    float4 normal = gbuffer[1][tc.xy] * 2 - 1;
    float4 specular = gbuffer[2][tc.xy];
    float3 pos = depth_to_wpos(depth_buffer[tc.xy], float2(tc.xy) / dims, camera.inv_view_proj);
    float3 light_dir = normalize(float3(0, 5, 1));
    float3 v = -normalize(camera.position - pos);
    float3 r = reflect(v, normal);
    float4 pos_l = mul(light_cam.view_proj, float4(pos, 1));
    pos_l /= pos_l.w;
    float3 cam_pos = depth_to_wpos_center(cam_buffer[(pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5)) * shadow_dims], float2(pos_l.xy), light_cam.inv_view_proj);
    float dist = length(light_cam.position - pos - 2 * normal);
    float dist2 = length(light_cam.position - cam_pos);
    float shadow = 1;//(dist2 >= dist);

    if (pos_l.z < 0 || pos_l.x > 1 || pos_l.x < -1 || pos_l.y > 1 || pos_l.y < -1)
        shadow = 1;

    float3 res_color = calc_color(albedo, normal, specular, r, v, light_dir, 1);
    result[tc.xy] = float4(res_color.xyz, albedo.w);
//   result[group_id.xy] = float4(cam_pos, albedo.w);
}




