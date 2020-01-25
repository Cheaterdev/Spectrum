#pragma once
#include "Common.hlsl"

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture3D<float4> voxels : register(t0, space1);

SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);

TextureCubeArray<float4> tex_cube : register(t0, space2);
Texture2D<float4> speed_tex : register(t2, space2);
Texture2D<float4> tex_gi : register(t3, space2);
Texture2D<float4> tex_gi_prev : register(t4, space2);
Texture2D<float> tex_depth_prev : register(t5, space2);

cbuffer cbCamera : register(b0)
{
	camera_info camera;
	camera_info prev_camera;
};

struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc : TEXCOORD0;
};


struct GI_RESULT
{
	//float4 screen: SV_Target0;
	float4 gi: SV_Target0;
};


GI_RESULT PS(quad_output i) 
{


	GI_RESULT result;

	float2 dims;
gbuffer[0].GetDimensions(dims.x, dims.y);


//dims /= 2;
int2 tc = i.tc*dims;

float raw_z = depth_buffer[tc.xy];
//if (raw_z >= 1) return ;
float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
float3 normal = normalize(gbuffer[1][tc].xyz * 2 - 1);



float2 delta = speed_tex.SampleLevel(PixelSampler, i.tc, 0).xy;

float2 prev_tc = i.tc -delta;
float4 prev_gi = tex_gi_prev.SampleLevel(LinearSampler, prev_tc, 0);
float prev_z = tex_depth_prev.SampleLevel(LinearSampler, prev_tc, 0);



float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prev_camera.inv_view_proj);



float dist = length(pos-camera.position)/100;
float l = length(pos - prev_pos)/ dist;



//float4 prev_gi= tex_gi_prev.SampleLevel(PixelSampler, i.tc, 0);
float4 res = 0;

float w = 0;

#define R 2

[unroll] for(int x=-R;x<=R;x++)
[unroll] for (int y = -R; y <= R; y++)
{
	float2 t_tc = i.tc + 2*float2(x, y) / dims;

	float t_raw_z = depth_buffer[t_tc.xy*dims];
	float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.inv_view_proj);
	float3 t_normal = normalize(gbuffer[1][t_tc*dims].xyz * 2 - 1);


	float4 t_gi = tex_gi.SampleLevel(PixelSampler, i.tc, 0,int2(x,y));


	float cur_w =  saturate(1 - 1*length(t_pos - pos));
	cur_w *= pow(saturate(dot(t_normal, normal)),32);
	res += cur_w*t_gi;

	w += cur_w;



}
float4 cur_gi = res / w;
//cur_gi = lerp(cur_gi, prev_gi, saturate(0.95-l));



/*result.screen =*/ result.gi = cur_gi;
//result.screen = l;

return result;
}