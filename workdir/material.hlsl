struct camera_info
{
    matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_view_proj;
    float3 position;
    float3 direction;
};

cbuffer cbCamera : register(b0)
{
    camera_info camera;
};

struct vertex_output
{
float4 pos : SV_POSITION;
float3 wpos : POSITION;
float3 normal : NORMAL;
float3 binormal : BINORMAL;
float3 tangent : TANGENT;
float2 tc : TEXCOORD;
};
Texture2D<float4> tex_albedo : register(t0);
Texture2D<float4> tex_specular : register(t1);
Texture2D<float4> tex_normal : register(t2);

SamplerState point_sampler : register(s0);
#if BUILD_FUNC == PS
float4 PS(vertex_output i) : SV_TARGET0
{

    float2 xy;
    tex_normal.GetDimensions(xy.x, xy.y);
//	if (xy.x<1)return 1;
    float3 bump = xy.x > 0 ? normalize(tex_normal.Sample(point_sampler, i.tc).xyz * 2.0 - 1.0) : float3(0, 0, 1);
    float3 normal = normalize(bump.x * i.tangent + bump.y * i.binormal + bump.z * i.normal);
    float4 albedo = tex_albedo.Sample(point_sampler, i.tc);
    float4 specular = tex_specular.Sample(point_sampler, i.tc) + 0.2;
    float3 v = -normalize(camera.position - i.wpos);
    float3 r = reflect(v, normal);
    //	return float4(bump, 1);
    float3 light_dir = normalize(float3(0, 5, 1));
    float s = pow(max(0, dot(r, light_dir)), 32 * specular.w);
    float4 color = albedo;
    color.xyz *= max(0, dot(light_dir, normal)) * 0.7 + 0.3 * specular.x;
    color.xyz += 2 * s * specular.xyz * (1 + albedo);
    color.w = 1;
    return float4(pow(color.xyz, 1.0f / 2.2), 1);
}
#endif

#if BUILD_FUNC == PS_Color

cbuffer cbMaterial : register(b1)
{
    float4 albedo;
};
float4 PS_Color(vertex_output i) : SV_TARGET0
{
    float3 normal = i.normal;
    //float4 albedo = tex_albedo.SampleLevel(point_sampler, i.tc, 0);
    float4 specular = 1;// tex_specular.SampleLevel(point_sampler, i.tc, 0);

    float3 v = -normalize(camera.position - i.wpos);
    float3 r = reflect(v, normal);
    float f = pow(max(0, dot(-v, normal)), 3) * 0.5 + 0.5;
    float3 light_dir = normalize(float3(0, 1, 1));
    float s = pow(max(0, dot(r, light_dir)), 32 * specular.w);
    float4 color = albedo * albedo;
    color.xyz *= max(0, dot(light_dir, normal)) * 0.8 + 0.2;


    float spec = 2 * s * specular.xyz * (albedo);
    color.xyz *= max(0, dot(-v, normal)) * 0.8 + 0.2;
    //color.xyz += ;

    color = lerp(color, spec, 1 - f);
    color.w = 1;
    return float4(pow(color.xyz, 1.0f / 2), 1);
}
#endif