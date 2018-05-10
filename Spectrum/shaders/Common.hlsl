#ifndef COMMON_HLSL
#define COMMON_HLSL
struct camera_info
{
	matrix view;
	matrix proj;
	matrix view_proj;
	matrix inv_view;
	matrix inv_proj;
	matrix inv_view_proj;
	float3 position;
	float unused;
	float3 direction;
	float unused2;
	float2 jitter; float2 unused3;
};


struct pixel_info
{
	float view_z;
	float3 pos;
	float3 normal;
	float3 albedo;
	float3 specular;
	float roughness;

	float3 view;
	float3 reflection;
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

float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = dot(n, -v);
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float3 calc_color(pixel_info info, float3 light_dir, float3 light_color)
{
	float fresnel = calc_fresnel(info.roughness, info.normal, info.view);
	float s = pow(saturate(dot(info.reflection, light_dir)), 128 * fresnel);
	float3 diffuse = info.albedo * max(0, dot(light_dir, info.normal)) * (1 - fresnel);
	float3 reflection = 0;// info.specular * fresnel * 50 * s;
	return light_color * (1 * diffuse + reflection);
}

#endif