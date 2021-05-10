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

static const Texture2D<float4> tex_color = GetVoxelBlur().GetNoisy_output();
static const Texture2D<float4> tex_frames = GetVoxelBlur().GetPrev_result();

static const RWTexture2D<float4> tex_result = GetVoxelBlur().GetScreen_result();
static const RWTexture2D<float4> tex_gi_result = GetVoxelBlur().GetGi_result();

struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc : TEXCOORD0;
};


float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, 0.5 + 0.5 * NoV), 0);
}


float3 get_PBR(float3 SpecularColor, float3 ReflectionColor, float3 N, float3 V, float Roughness, float Metallic)
{
	V *= -1;
	float NoV = dot(N, V);
	//return NoV<0;
//	float3 R = 2 * dot(V, N) * N - V;
//	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
	float2 EnvBRDF = IntegrateBRDF(Roughness, Metallic, NoV);
	return     ReflectionColor * (Metallic * SpecularColor * EnvBRDF.x + EnvBRDF.y);
}



[numthreads(8, 8, 1)]
void  PS(uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{

	uint2 index = GetVoxelBlur().GetTiling().get_pixel_pos(dispatchID);




	float2 dims;
	tex_color.GetDimensions(dims.x, dims.y);

	float2 itc = float2(index + 0.5) / dims;
	//dims /= 2;
	int2 tc = index;
	float4 albedo = gbuffer.GetAlbedo()[tc];

	float  roughness = pow(gbuffer.GetNormals()[tc].w, 1);
	float metallic = albedo.w;// specular.w;

	float raw_z = gbuffer.GetDepth()[tc.xy];
	float3 pos = depth_to_wpos(raw_z, itc, camera.GetInvViewProj());
	float3 v = normalize(pos - camera.GetPosition());

	float4 res = 0;
	float w = 0;
	float framesNorm = tex_frames.SampleLevel(pointClampSampler, itc, 0).x;
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

#ifdef ENABLE_BLUR

	float dist = length(pos - camera.GetPosition()) / 100;

	int frames = 8 * framesNorm;


	float blurRadiusScale = 1.0 / (1.0 + frames) - 1.0 / (1 + 8);
#define R 3
	if (blurRadiusScale > 0)
	{
		[unroll] for (int x = -R; x <= R; x++)
			[unroll] for (int y = -R; y <= R; y++)
		{
			float2 offset = 2 * float2(x, y) * (blurRadiusScale);

			float2 t_tc = itc + offset / dims;

			float t_raw_z = gbuffer.GetDepth().SampleLevel(pointBorderSampler, t_tc, 0);
			float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.GetInvViewProj());
			float3 t_normal = normalize(gbuffer.GetNormals().SampleLevel(pointBorderSampler, t_tc, 0).xyz * 2 - 1);


			float4 t_gi = tex_color.SampleLevel(pointClampSampler, t_tc, 0);


			float cur_w = saturate(1 - length(t_pos - pos) / dist);
			cur_w *= pow(saturate(dot(t_normal, normal)), 2);
			//cur_w = 1;
			res += cur_w * t_gi;

			w += cur_w;



		}
	}
	else
#endif
	{		
		res = tex_color.SampleLevel(pointClampSampler, itc, 0);
		w = 1;
	}


float4 cur_gi = res/w;

//tex_gi_result[index] = float4(cur_gi.xyz, framesNorm);
//tex_result[index] += float4(albedo.xyz * cur_gi.xyz, 1);
tex_gi_result[index] =  float4(cur_gi.xyz, framesNorm);



#ifdef REFLECTION
float3 color = get_PBR(albedo, cur_gi.xyz, normal, v, roughness, metallic);
#else
float NoV = dot(normal, v);
float3 color = albedo*IntegrateBRDF(roughness, metallic, -NoV).x* cur_gi.xyz;
#endif


tex_result[index] += float4(color, 1);

}