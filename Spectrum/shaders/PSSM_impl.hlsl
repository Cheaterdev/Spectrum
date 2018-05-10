
#include "Common.hlsl"


Texture2DArray<float> light_buffer: register(t4);
StructuredBuffer<camera_info> light_cameras : register(t5);

cbuffer cbPSSM : register(b1)
{
	float scaler;
};


float get_shadow(float3 wpos)
{
	uint4 shadow_dims = uint4(512, 512, 512, 512);
	light_buffer.GetDimensions(shadow_dims.x, shadow_dims.y, shadow_dims.z);
	int level =0 ;
	camera_info light_cam = light_cameras[level];
	float4 pos_l = mul(light_cam.view_proj, float4(wpos , 1));
	//pos_l.z -= 0.0005 * pow(2, level);
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
	uint3 pos = uint3(light_tc * shadow_dims, level);
	float light_raw_z = light_buffer[pos.xyz];
	//  float shadow = light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z); //light_raw_z > pos_l.z - 0.0001
#ifdef SINGLE_SAMPLE
	float shadow = light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z);
#else
	float ShadowTexelSize = 1 / shadow_dims.x;
	const float Dilation = 2.0;
	float d1 = 1;//Dilation * ShadowTexelSize * 0.125;
	float d2 = 1;//Dilation * ShadowTexelSize * 0.875;
	float d3 = 1;//Dilation * ShadowTexelSize * 0.625;
	float d4 = 1;//Dilation * ShadowTexelSize * 0.375;
	float shadow = (
		2.0 * light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d2, d1)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d1, -d2)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d2, -d1)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d1, d2)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d4, d3)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d3, -d4)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d4, -d3)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d3, d4))
		) / 10.0;
#endif

	if (pos_l.z < 0 || pos_l.x > 1 || pos_l.x < -1 || pos_l.y > 1 || pos_l.y < -1)
		shadow = 1;

	return shadow;
}

float get_shadow(uint4 shadow_dims, pixel_info info, float level)
{
	camera_info light_cam = light_cameras[level];
	float4 pos_l = mul(light_cam.view_proj, float4(info.pos + info.normal / 2, 1));
	pos_l.z -= 0.005 * pow(2, level);
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
	uint3 pos = uint3(light_tc * shadow_dims, level);
	float light_raw_z = light_buffer[pos.xyz];
	//  float shadow = light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z); //light_raw_z > pos_l.z - 0.0001
#ifdef SINGLE_SAMPLE
	float shadow = light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z);
#else
	float ShadowTexelSize = 1 / shadow_dims.x;
	const float Dilation = 2.0;
	float d1 = 1;//Dilation * ShadowTexelSize * 0.125;
	float d2 = 1;//Dilation * ShadowTexelSize * 0.875;
	float d3 = 1;//Dilation * ShadowTexelSize * 0.625;
	float d4 = 1;//Dilation * ShadowTexelSize * 0.375;
	float shadow = (
		2.0 * light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d2, d1)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d1, -d2)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d2, -d1)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d1, d2)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d4, d3)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(-d3, -d4)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d4, -d3)) +
		light_buffer.SampleCmpLevelZero(cmp_sampler, float3(light_tc, level), pos_l.z, int2(d3, d4))
		) / 10.0;
#endif

	if (pos_l.z < 0 || pos_l.x > 1 || pos_l.x < -1 || pos_l.y > 1 || pos_l.y < -1)
		shadow = 1;

	return shadow;
}

float get_level(pixel_info info, int count)
{
	return clamp(log(info.view_z / scaler), 0, count - 1);
}

float get_shadow(pixel_info info)
{
	uint4 shadow_dims = uint4(512, 512, 512, 512);
	light_buffer.GetDimensions(shadow_dims.x, shadow_dims.y, shadow_dims.z);
	int level = get_level(info, shadow_dims.z);
	return get_shadow(shadow_dims, info, level);

}


float3 get_debug_color(pixel_info info)
{

	uint4 shadow_dims = uint4(512, 512, 512, 512);
	light_buffer.GetDimensions(shadow_dims.x, shadow_dims.y, shadow_dims.z);

	static const  float3 array[] = { float3(1, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1),
		float3(1, 1, 0),
		float3(0, 1, 1),
		float3(1, 0, 1)
	};

	return array[get_level(info, shadow_dims.z)];
}