#define SIZE 18

#include "Common.hlsl"


cbuffer cbCamera : register(b0)
{
    camera_info camera;
};


Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);


RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerComparisonState cmp_sampler: register(s0);
SamplerState LinearSamplerClamp : register(s2);
Texture3D<float4> brdf : register(t6);
#include "PSSM_impl.hlsl"

#include "PBR.hlsl"

[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    float2 dims;
    uint dummy;
    gbuffer[0].GetDimensions(dims.x, dims.y);

    if (dims.x < tc.x) return;

    if (dims.y < tc.y) return;

  
    pixel_info info;
    info.albedo = gbuffer[0][tc.xy];
    info.normal = normalize(gbuffer[1][tc.xy].xyz * 2 - 1);
	info.metallic = gbuffer[0][tc.xy].w;
   // info.specular = gbuffer[2][tc.xy].xyz;
	info.roughness = gbuffer[1][tc.xy].w;// gbuffer[2][tc.xy].w;
    float raw_z = depth_buffer[tc.xy];

    info.pos = depth_to_wpos(raw_z, float2(tc.xy) / dims, camera.inv_view_proj);
    info.view_z = camera.proj._34 * raw_z / (raw_z - camera.proj._33);
    info.view = -normalize(camera.position - info.pos);
    info.reflection = reflect(info.view, info.normal);
 
	uint4 shadow_dims = uint4(512, 512, 512, 512);
	light_buffer.GetDimensions(shadow_dims.x, shadow_dims.y, shadow_dims.z);
	float shadow = get_shadow(shadow_dims,info,0);
  
	
	if (raw_z < 1)
	//	result[tc.xy] = float4(PBR(direct, reflection, info.albedo, info.normal, info.view, 0.2, info.roughness, gbuffer[0][tc.xy].w), 1);
	result[tc.xy] = float4(get_debug_color(info), 1);
	else
		result[tc.xy] = 1;
}





#define SCALE 1


#include "Rect.hlsl"
SamplerState pixel_sampler: register(s1);


cbuffer cbPSSM : register(b1)
{
	int level;
	float time;
};

float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float4 PS(quad_output i) : SV_Target0
{
	
	pixel_info info;
info.albedo = gbuffer[0].SampleLevel(pixel_sampler,i.tc,0);
info.normal = normalize(gbuffer[1].SampleLevel(pixel_sampler, i.tc, 0).xyz * 2 - 1);
float raw_z = depth_buffer.SampleLevel(pixel_sampler, i.tc, 0);
if (raw_z >= 1) return 0;

info.pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);


//info.roughness = gbuffer[1][tc.xy].w + 0.1;// gbuffer[2][tc.xy].w;
//int level = 1;

camera_info light_cam = light_cameras[level];
float4 pos_l = mul(light_cam.view_proj, float4(info.pos +info.normal*0.1, 1));
float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
//uint3 pos = uint3(light_tc * shadow_dims, level);

 
float tc_scaler = 0;// 0.5 / pow(2, level);
if (any(light_tc.xy < tc_scaler) || any(light_tc.xy > 1 - tc_scaler))
discard;


float sini = sin(time * 220 + float(i.tc.x));
float cosi = cos(time * 220 + float(i.tc.y));
float rand = rnd(float2(sini, cosi));

float rcos = cos(6.14*rand);
float rsin = sin(6.14*rand);


static const float2 poisson[17] = {
	float2(0, 0),
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870),
	float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	float2(-0.81544232, -0.87912464),
	float2(-0.38277543, 0.27676845),
	float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188),
	float2(-0.24188840, 0.99706507),
	float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367),
	float2(0.14383161, -0.14100790)

};



float result = 0;
float blur_scaler = 0.0;


for (int i = 1; i < 2; i++)
{
	float2 cur_tc = light_tc + float2(rsin,rcos)*poisson[i] * tc_scaler;
	float light_raw_z = light_buffer.SampleLevel(pixel_sampler, float3(cur_tc, level), 0) ;
	float3 pos = depth_to_wpos(light_raw_z, cur_tc, light_cam.inv_view_proj);


	//
	//float3 delta = pos - info.pos;
	//float3 proj = normalize(delta)*dot(normalize(delta), light_cam.direction * 1000);
	//float cur_rad = sqrt(length(light_cam.direction)*length(light_cam.direction)*1000 - length(proj)*length(proj));
	//	if(l > Out.L) continue;

	float cur_rad =(dot(normalize(pos-info.pos), -light_cam.direction)-0.999)/0.001;
	//if (i == 0 && cur_rad < 0.1) break;

	blur_scaler = max(blur_scaler, cur_rad*length(poisson[i]));

	//blur_scaler = min(blur_scaler,dot(light_cam.direction,normalize(pos-info.pos)));
}
//return blur_scaler;

for (int i = 1; i < 2; i++)
{

	float2 cur_tc = light_tc + float2(rsin, rcos)*blur_scaler*poisson[i] * tc_scaler;
	float light_raw_z = light_buffer.SampleLevel(pixel_sampler, float3(cur_tc, level), 0) + (blur_scaler+1)*0.00003;

	result+= light_raw_z > pos_l.z;
}
/*
float sum = 0.01;
for (int i = 0; i < 16; i++)
{

	float2 cur_tc = light_tc + blur_scaler*float2(rsin, rcos)*poisson[i] * tc_scaler;
	float light_raw_z = light_buffer.SampleLevel(pixel_sampler, float3(cur_tc, level), 0);// +(blur_scaler + 1)*0.0001;

	float3 pos = depth_to_wpos(light_raw_z, cur_tc, light_cam.inv_view_proj);
	float cur_rad = 1;// (dot(normalize(pos - info.pos), -light_cam.direction) - 0.99) / 0.01;

	result += cur_rad *(light_raw_z > pos_l.z);
	sum += cur_rad;
}*/
//return  float(level) / 6;
return result/1;// light_raw_z > pos_l.z;
}


Texture2D<float> light_mask: register(t4);



float3 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res.xyz /= res.w;
	return float3(res.xy * float2(0.5, -0.5) + float2(0.5, 0.5),res.z);
}

float get_sss(float z, float3 pos, float2 tc, float3 n)
{
	float2 dims;
	depth_buffer.GetDimensions(dims.x, dims.y);
	float3 r = normalize(-light_cameras[0].direction);
	float level = 0;
	float res = 1;
	float dist = 0.0;

	float step = distance(camera.position, pos) / 1000;
	float errorer = step*2;

//	float errorrer = 0.02/SCALE;

	for (int i = 0; i < 8; i++)
	{
		dist += step;

		float3	reflect_tc = project_tc(pos + dist * r, camera.view_proj);

		if (any(reflect_tc.xy < 0 || reflect_tc.xy>1)) return res;

		float	raw_z = depth_buffer.SampleLevel(pixel_sampler, reflect_tc, level).x;

		float3 p1 = depth_to_wpos(raw_z, float2(reflect_tc.xy), camera.inv_view_proj);
		float3 p2 = pos + dist * r;


		float l1 = length(p1 - camera.position);
		float l2 = length(p2 - camera.position);


		if ((l1 < l2-errorer) )
			res = min(res, abs(l2-l1));
	}

	return res;
}

float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return brdf.SampleLevel(LinearSamplerClamp, float3(Roughness, Metallic, NoV), 0);
}

float4 PS_RESULT(quad_output i) : SV_Target0
{

	pixel_info info;

float4 packed_0 = gbuffer[0].SampleLevel(pixel_sampler, i.tc, 0);

//return packed_0;
info.albedo = packed_0.rgb;
info.metallic = packed_0.w;

info.normal = normalize(gbuffer[1].SampleLevel(pixel_sampler, i.tc, 0).xyz * 2 - 1);
float raw_z = depth_buffer.SampleLevel(pixel_sampler, i.tc, 0);
if (raw_z >= 1) return 0;
info.pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);


//info.specular = gbuffer[2].SampleLevel(pixel_sampler, i.tc, 0);
info.roughness = max(0.04,gbuffer[1].SampleLevel(pixel_sampler, i.tc, 0).w);// gbuffer[2][tc.xy].w;

info.view_z = camera.proj._34 * raw_z / (raw_z - camera.proj._33);
info.view = normalize(camera.position - info.pos);
info.reflection = reflect(info.view, info.normal);

float sss = get_sss(info.view_z,info.pos, i.tc, info.normal);// *saturate(dot(-light_cameras[0].direction, info.normal));
float shadow =  light_mask.SampleLevel(pixel_sampler, i.tc, 0);
float3 res_color = calc_color(info, -light_cameras[0].direction, shadow);
//sss = saturate(sss * 2 - 1);
//shadow = light_cameras[0].direction.x;

//shadow = min(shadow, sss);

//return shadow;
//return min(shadow,sss);
float3 light_dir = normalize(-light_cameras[0].direction);

//float3 direct = shadow*max(0, dot(light_dir, info.normal));
//float3 reflection = 0;// shadow*pow(saturate(dot(info.reflection, light_dir)), 2 * info.roughness);
//return float4(get_debug_color(info),0);

//float3 F;


float3 Fk=0;

float NV = dot(info.normal, info.view);
float NL = dot(info.normal, light_dir);
float3 h = normalize(info.view + light_dir);

float HV = dot(h, info.view);
float2 EnvBRDF = 1;

EnvBRDF = IntegrateBRDF(info.roughness, saturate(info.metallic), NL);
/*
if(NL>0)
EnvBRDF *= IntegrateBRDF(info.roughness, NL).x;
else
EnvBRDF = 0;*/

//float3 F = FresnelSchlick(saturate(info.metallic), saturate(NV));

//return (EnvBRDF.xxxx);
float3 refl = CookTorrance_GGX_sample(light_dir, info,Fk);

return float4(shadow*(EnvBRDF.x*info.albedo), 0);

//return  float4(PBR(direct, reflection, info.albedo, info.normal, info.view, 0.2, info.roughness, packed_0.w), 1);
}