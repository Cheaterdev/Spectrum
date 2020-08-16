#pragma once
#include "Common.hlsl"

#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelBlur.h"

static const Camera camera = GetFrameInfo().GetCamera();
static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelScreen().GetGbuffer();

static const Texture2D<float4> tex_color = GetVoxelBlur().GetTex_color();

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
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);


//dims /= 2;
int2 tc = i.tc*dims;

float raw_z = gbuffer.GetDepth()[tc.xy];
//if (raw_z >= 1) return ;
float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

float dist = length(pos-camera.GetPosition())/100;

float4 res = tex_color.SampleLevel(pointClampSampler, i.tc, 0);

float w = 1;

#define R 1


[unroll] for(int x=-R;x<=R;x++)
[unroll] for (int y = -R; y <= R; y++)
{
	float2 t_tc = i.tc + 1*float2(x, y) / dims;

	float t_raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, i.tc, 1, int2(x, y));
	float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.GetInvViewProj());
	float3 t_normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, i.tc, 1, int2(x, y)).xyz * 2 - 1);


	float4 t_gi = tex_color.SampleLevel(pointClampSampler, i.tc, 0, int2(x,y));


	float cur_w = saturate(1 - length(t_pos - pos) / dist);
	cur_w *= pow(saturate(dot(t_normal, normal)),8);
	res += cur_w*t_gi;

	w += cur_w;



}


float4 cur_gi = res / w;
result.gi = float4(cur_gi.xyz,1);

return result;
}