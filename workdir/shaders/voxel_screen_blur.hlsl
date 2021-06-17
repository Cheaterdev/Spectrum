#pragma once
#include "Common.hlsl"


#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelBlur.h"

#include "autogen/TilingPostprocess.h"

static const Camera camera = GetFrameInfo().GetCamera();
static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelScreen().GetGbuffer();

static const Texture2D<float4> tex_color = GetVoxelBlur().GetNoisy_output();
static const Texture2D<float4> tex_frames = GetVoxelBlur().GetPrev_result();

static const RWTexture2D<float4> tex_result = GetVoxelBlur().GetScreen_result();
static const RWTexture2D<float4> tex_gi_result = GetVoxelBlur().GetGi_result();


static const float FLT_EPS = 0.00000001f;

float Luminance(float3 color)
{
	return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float2 CalculateMotion(float3 pos)
{
	float2 cur_tc = project_tc(pos, camera.GetViewProj());
	float2 prev_tc = project_tc(pos, prevCamera.GetViewProj());
	return cur_tc - prev_tc;
}
static const int2 offset[9] =
{
	int2(0, 0),
	int2(0, 1),
	int2(1, -1),
	int2(-1, -1),
	int2(-1, 0),
	int2(0, -1),
	int2(1, 0),
	int2(-1, 1),
	int2(1, 1)
};
float4 clip_aabb(float3 aabb_min, float3 aabb_max, float4 p, float4 q)
{
	float3 p_clip = 0.5 * (aabb_max + aabb_min);
	float3 e_clip = 0.5 * (aabb_max - aabb_min) + FLT_EPS;

	float4 v_clip = q - float4(p_clip, p.w);
	float3 v_unit = v_clip.xyz / e_clip;
	float3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return float4(p_clip, p.w) + v_clip / ma_unit;
	else
		return q; // point inside aabb
}

static const float TResponseMin = 0.9;
static const float TResponseMax = 1.0;

float4 temporal_reprojection(float2 uv, float vs_dist, float2 dims)
{
	float4 current = tex_color.SampleLevel(pointClampSampler, uv, 0);
	//Store1TPixel(groupIndex, current);
	//GroupMemoryBarrierWithGroupSync();

	//bool border = groupThreadId.x == 0 | groupThreadId.y == 0 | groupThreadId.x == KERNEL_SIZE + 1 || groupThreadId.y == KERNEL_SIZE + 1;
//	if (border) return;

	float2 unityVelocity = gbuffer.GetMotion().SampleLevel(pointClampSampler, uv, 0).xy;
	float3 hit = GetVoxelBlur().GetHit_and_pdf().SampleLevel(pointBorderSampler, uv, 0).xyz;

//	float3 virtu
//	float depth = GetDepth(uv, 0);
//	float unityPrevDepth = GetDepth(uv + unityVelocity, 0);
//	float hitDepth = GetDepth(hit.xy, 0);
	/*
	float2 reflectionCameraVelocity = CalculateMotion(hitDepth, uv);
	float2 hitCameraVelocity = CalculateMotion(hit);
	float2 cameraVelocity = CalculateMotion(depth, uv);
	float2 hitUnityVelocity = GetVelocity(hit.xy);

	float2 velocityDiff = cameraVelocity - unityVelocity;

	float2 hitVelocityDiff = hitCameraVelocity - hitUnityVelocity;
	float objectVelocityMask = saturate(dot(velocityDiff, velocityDiff) * ScreenSize.x * 100);
	float hitObjectVelocityMask = saturate(dot(hitVelocityDiff, hitVelocityDiff) * ScreenSize.x * 100);
	float2 objectVelocity = unityVelocity * objectVelocityMask;
	float2 hitObjectVelocity = hitUnityVelocity * hitObjectVelocityMask;

	float2 reflectVelocity = lerp(lerp(reflectionCameraVelocity, hitObjectVelocity, hitObjectVelocityMask), objectVelocity, objectVelocityMask);
	float2 velocity = reflectVelocity;
	float2 prevUV = uv - velocity;
	*/

	float2 velocity = CalculateMotion(hit);
	float2 prevUV = uv - velocity;
	float4 previous = tex_frames.SampleLevel(pointClampSampler, prevUV, 0);

	half4 currentMin = 100;
	half4 currentMax = 0;
	half4 currentAvarage = 0;

	[unroll]
	for (int i = 0; i < 9; i++)
	{
		float2 tuv = uv + offset[i]/dims;
		float4 val = tex_color.SampleLevel(pointClampSampler, tuv, 0);
		currentMin = min(currentMin, val);
		currentMax = max(currentMax, val);
		currentAvarage += val;
	}

	currentAvarage /= 9.0;

	previous = clip_aabb(currentMin.xyz, currentMax.xyz, clamp(currentAvarage, currentMin, currentMax), previous);

	float lum0 = Luminance(current.rgb);
	float lum1 = Luminance(previous.rgb);

	float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
	float unbiased_weight = 1.0 - unbiased_diff;
	float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
	float k_feedback = lerp(TResponseMin, TResponseMax, unbiased_weight_sqr);

	return  lerp(current, previous, k_feedback);
}


struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc : TEXCOORD0;
};


float4 get_voxel(float3 pos, float level)
{
	float4 color = GetVoxelScreen().GetVoxels().SampleLevel(linearClampSampler, pos, level);

	//color.xyz /= color.w + 0.0001;
	return color;
}



float4 trace(float4 start_color, float start_dist, float3 origin, float3 dir, float angle, float3 normal, out float dist)
{
	float3 voxel_min = GetVoxelInfo().GetMin().xyz;
	float3 voxel_size = GetVoxelInfo().GetSize().xyz;
	float3 oneVoxelSize = 0.5 / (voxel_info.GetVoxel_tiles_count() * voxel_info.GetVoxels_per_tile());

	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);

	float angle_coeff = saturate(max_angle / (angle + 0.01));
	angle = min(angle, max_angle);
	float3 startOrigin = saturate(((origin + start_dist * dir - (voxel_min)) / voxel_size));
	origin = saturate(((origin - (voxel_min)) / voxel_size));

//	return get_voxel(origin,0);
	float3 samplePos = 0;
	float4 accum = start_color;
	// the starting sample diameter
	float minDiameter = oneVoxelSize.z;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;

	float maxDist = 1;
	dist = length(startOrigin - origin);

	float max_accum = 1;
	while (dist <= maxDist && accum.w < max_accum && all(samplePos <= 1) && all(samplePos >= 0))
	{
		float sampleDiameter = minDiameter + angle * dist;

		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);//* float4(1,1,1,1 + sampleLOD/4);
		//sampleValue.w *= 333;


		float sampleWeight = saturate(1 - accum.w);
		accum += sampleValue * sampleWeight;
		dist += sampleDiameter;

		//if (accum.w >= 0.9)
		//	accum /= accum.w;

	}

	if (accum.w < 0.1) dist = 1;
	//accum.xyz *= pow(dist,0.7);
	//accum.xyz *= angle_coeff;
	//accum *= 1.0 / 0.9;

	float3 sky = GetFrameInfo().GetSky().SampleLevel(linearSampler, normalize(dir), 6*angle);
	float sampleWeight = saturate(max_accum - accum.w) / max_accum;
	accum.xyz += sky * pow(sampleWeight, 1);


	dist *= length(voxel_size);
	//	accum.xyz = sky;

	return accum;// / saturate(accum.w);
}

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
	//return;
	uint2 index = GetTilingPostprocess().GetTiling().get_pixel_pos(dispatchID);

	float2 dims;
	tex_color.GetDimensions(dims.x, dims.y);

	float2 itc = float2(index + 0.5) / dims;
	//dims /= 2;
	int2 tc = index;
	float4 albedo = gbuffer.GetAlbedo()[tc];

	float  roughness = pow(max(MIN_ROUGHNESS, gbuffer.GetNormals()[tc].w), 2);
	float metallic = albedo.w;// specular.w;

	float raw_z = gbuffer.GetDepth()[tc.xy];
	float3 pos = depth_to_wpos(raw_z, itc, camera.GetInvViewProj());
	float3 v = normalize(pos - camera.GetPosition());

	float4 res = 0;
	float w = 0;
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);
	float framesNorm = tex_frames.SampleLevel(pointClampSampler, itc, 0).x;
#ifndef ENABLE_BLUR

	float dist = length(pos - camera.GetPosition()) / 100;

	int frames = 16 * framesNorm;
	
	float blurRadiusScale =  1.0 / (1.0 + frames) - 1.0 / (1 + 8);
	

#define R 1
	if (blurRadiusScale > 0)
	{
		[unroll] for (int x = -R; x <= R; x++)
			[unroll] for (int y = -R; y <= R; y++)
		{
			float2 offset = 1 * float2(x, y) * (blurRadiusScale);

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
		res =  tex_color.SampleLevel(pointClampSampler, itc, 0);
		w = 1;
	}


float4 cur_gi = res/w;

//tex_gi_result[index] = float4(cur_gi.xyz, framesNorm);
//tex_result[index] += float4(albedo.xyz * cur_gi.xyz, 1);
//tex_gi_result[index] =  float4(cur_gi.xyz, framesNorm);



#ifdef REFLECTION

float3 oneVoxelSize = voxel_info.GetSize() / (voxel_info.GetVoxel_tiles_count() * voxel_info.GetVoxels_per_tile());

float3 rayDir = normalize(reflect(v, normal));
float max_ray = length(oneVoxelSize) / (tan(roughness) + 0.001);

float3 refl_pos = pos + GetVoxelBlur().GetHit_and_pdf().SampleLevel(pointBorderSampler, itc, 0).xyz * cur_gi.w;
float4 virt_pos = float4(pos + v * cur_gi.w, 1);

//virt_pos = mul(virt_pos, camera.GetInvViewProj());
//virt_pos = mul(virt_pos, camera.GetViewProj());

float2 prev_tc = project_tc(virt_pos.xyz/ virt_pos.w, prevCamera.GetViewProj());

//float2 delta = gbuffer.GetMotion().SampleLevel(pointClampSampler, itc, 0).xy;
	//float2 prev_tc = itc - delta;
	//float4 prev_gi = temporal_reprojection(itc, 0, dims); //tex_frames.SampleLevel(pointClampSampler, prev_tc, 0);


	// 
	// 
//	color = cur_gi;

	//color = dot(normal,GetVoxelBlur().GetHit_and_pdf().SampleLevel(pointBorderSampler, itc, 0).xyz)><0;
/*
cur_gi.w = cur_gi.w < max_ray;
cur_gi.xyz *= cur_gi.w;
*/


/*
float4 voxel_color = trace(0, 0.0, pos + (rayDir * max_ray + oneVoxelSize * (normal)), rayDir, roughness* roughness, normal, max_ray);
if (cur_gi.w == 0)
cur_gi = voxel_color;// lerp(cur_gi, voxel_color, saturate(cur_gi.w / max_ray));
*/



	float3 color = temporal_reprojection(itc, 0, dims);
	// color = lerp(color, prev_gi, 0.9);
	tex_gi_result[index] = float4(color,0);


//cur_gi = lerp(cur_gi/(cur_gi.w+0.0001), voxel_color, 1-saturate(cur_gi.w));


//

//float3 dir = GetRandomDir(itc, rayDir, roughness, GetFrameInfo().GetTime().y);

 color = get_PBR(albedo, color.xyz, normal, v, roughness, metallic);
//tex_gi_result[index] = float4(color, 1);


#else
tex_gi_result[index] = float4(cur_gi.xyz, framesNorm);

float NoV = dot(normal, v);
float3 color = albedo*IntegrateBRDF(roughness, metallic, -NoV).x* cur_gi.xyz;
#endif


tex_result[index] += float4(color, 1);

}