#define SIZE 16
#define FIX 8
#include "2D_screen_simple.h"

#include "Common.hlsl"
#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelScreen.h"

#ifndef BUILD_FUNC_CS
#include "autogen/VoxelUpscale.h"

static const Texture2D<float4> tex_gi_prev = GetVoxelUpscale().GetTex_gi_prev();
static const Texture2D<float> tex_depth_prev = GetVoxelUpscale().GetTex_depth_prev();
static const Texture2D<float4> tex_downsampled = GetVoxelUpscale().GetTex_downsampled();


#endif
static const Camera camera = GetFrameInfo().GetCamera();

static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelScreen().GetGbuffer();


static const float time = GetFrameInfo().GetTime();


//#define NONE
//#define SCREEN
//#define INDIRECT
//#define REFLECTION

#define SCALER 1

static const float3 voxel_min = voxel_info.GetMin().xyz;
static const float3 voxel_size = voxel_info.GetSize().xyz;

static const float scaler = voxel_size / 256;
static const Texture3D<float4> voxels = GetVoxelScreen().GetVoxels();
static const TextureCube<float4> tex_cube = GetVoxelScreen().GetTex_cube();
static const Texture2D<float2> speed_tex = GetVoxelScreen().GetGbuffer().GetMotion();



float4 get_voxel(float3 pos, float level)
{
	float4 color = voxels.SampleLevel(linearSampler, pos, level);


	//color.rgb *= 1 + level / 2;

		//color.w = saturate(color.w* (level+3));
	return color;
}
float get_alpha(float3 pos, float level)
{
	return voxels.SampleLevel(linearSampler, pos, level).a;
}


float3 PrefilterEnvMap(float Roughness, float3 R)
{
	return  tex_cube.SampleLevel(linearSampler, R, Roughness * 3).rgb;
}




float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().IntegrateBRDF(Roughness, Metallic, NoV);
}


float3 get_sky(float3 dir, float level)
{
	return PrefilterEnvMap(1, dir);
}


float4 trace(float4 start_color, float3 view, float3 origin, float3 dir, float3 normal, float angle)
{
	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14);

	float angle_coeff = saturate(max_angle / (angle + 0.01));
	//angle = min(angle, max_angle);
	origin = saturate(((origin - (voxel_min)) / voxel_size));

	float3 samplePos = 0;
	float4 accum = start_color;
	// the starting sample diameter
	float minDiameter = 1.0 / 512;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;

	float maxDist = 1;
	float dist = minDiameter;

	float max_accum = 0.9;
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
	//accum.xyz *= pow(dist,0.7);
	//accum.xyz *= angle_coeff;
	//accum *= 1.0 / 0.9;

	float3 sky = get_sky(normal, angle);
	float sampleWeight = saturate(max_accum - accum.w * SCALER) / max_accum;
	accum.xyz += sky * pow(sampleWeight, 1);

	return accum;// / saturate(accum.w);
}




float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float4 get_direction(float3 pos, float v, float3 orig_pos, float3 normal, float3 dir, float k, float a, float dist, float2 orig_tc)
{
	float4 gi = trace(0, v, pos, dir, normal, a);
	return gi;
}

float4 getGI(float2 tc, float3 orig_pos, float3 Pos, float3 Normal, float3 V, float r, float roughness, float metallic)
{
	float a = 0.4;

	float4 Color = 0;
	float k = scaler;

	float sini = sin(time * 220 + float(tc.x));
	float cosi = cos(time * 220 + float(tc.y));
	float rand = rnd(float2(sini, cosi));

	float rcos = cos(6.14 * rand);
	float rsin = sin(6.14 * rand);
	float rand2 = rnd(float2(cosi, sini));

	float t = rand2;// lerp(0.15, 1, rand2);

	float3 right = rsin * t * normalize(cross(Normal, float3(0, 1, 0.1)));
	float3 tangent = rcos * t * normalize(cross(right, Normal));

	float3 dir = normalize(Normal + right + tangent);
	float2 EnvBRDF = IntegrateBRDF(roughness, metallic, dot(Normal, dir));

	Color = EnvBRDF.x * get_direction(Pos, V, orig_pos, Normal, normalize(Normal + right + tangent), k, a, length(Pos - camera.GetPosition()) / 20, tc);

	return 1 * Color;

}
#include "PBR.hlsl"


#define MOVE_SCALER 1

struct GI_RESULT
{
	float4 screen: SV_Target0;
	float4 gi: SV_Target1;
};
#ifdef BUILD_FUNC_PS
[earlydepthstencil]
GI_RESULT PS(quad_output i)
{

	GI_RESULT result;

	result.screen = 0;
	result.gi = 0;
#ifdef NONE
	return result;
#endif
	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);
	int2 tc = i.tc * dims;

	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1)
		return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float4 albedo = gbuffer.GetAlbedo()[tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.GetPosition());
	float3 r = normalize(reflect(v, normal));
#ifdef SCREEN
	result.screen = trace_screen(camera.GetPosition(), v, 0);

	return result;
#endif
	//return voxels.SampleLevel(linearSampler,index,0);
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));



	float w = 0.1;
	float4 res = tex_downsampled.SampleLevel(pointClampSampler, i.tc, 0) * w;

#define R 1

	for (int x = -R; x <= R; x++)
		for (int y = -R; y <= R; y++)
		{
			float2 t_tc = i.tc + float2(x, y) / dims;


			int2 offset = float2(x, y);

			float t_raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, i.tc, 1, offset);
			float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.GetInvViewProj());
			float3 t_normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, i.tc, 1, offset).xyz * 2 - 1);


			float4 t_gi = tex_downsampled.SampleLevel(pointClampSampler, i.tc, 0, offset);


			float cur_w = saturate(1 - length(t_pos - pos));// 1.0 / (8 * length(t_pos - pos) + 0.1);
			cur_w *= pow(saturate(dot(t_normal, normal)), 16);
			//	cur_w = saturate(cur_w - 0.1);

			res += cur_w * t_gi;

			w += cur_w;



		}
	float4 gi = res / w;



	//float4 gi = max(0, getGI(i.tc, pos, pos + scaler * normal / m, normal, v, r, gbuffer.GetNormals()[tc].w, albedo.w));





	float2 delta = speed_tex.SampleLevel(pointClampSampler, i.tc, 0).xy;

	float2 prev_tc = i.tc - delta;
	float4 prev_gi = tex_gi_prev.SampleLevel(linearClampSampler, prev_tc, 0);
	float prev_z = tex_depth_prev.SampleLevel(linearClampSampler, prev_tc, 0);



	float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prevCamera.GetInvViewProj());

	float l = length(pos - prev_pos) * MOVE_SCALER;
	gi = lerp(gi, prev_gi, saturate(0.95 - l));


	result.gi = gi;


#ifdef INDIRECT
	result.screen = gi;
	return result;
#endif
	//return gi;
  // return gi;
	//specular.w = pow(specular.w, 1);

	float  roughness = pow(gbuffer.GetNormals()[tc].w, 1);
	float metallic = albedo.w;// specular.w;
	 //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness;

	float4 reflection = 0;// trace_refl(pos + scaler*normal / m, normalize(r), normal, angle);
	//return metallic*reflection;

#ifdef REFLECTION
	result.screen = reflection;
	return result;
#endif


	result.screen = albedo * gi;// *tex_color[tc] + albedo * gi;// float4(PBR(gi, reflection, albedo, normal, v, 0.2, roughness, metallic), 1);

	result.screen.w = 1;
	return result;
	//	
}

#endif
struct NoiseOutput
{

	float4 gi : SV_Target0;
	float numFrames : SV_Target1;
};

static const Texture2D<float> tex_frames_prev = GetVoxelScreen().GetPrev_frames();
static const Texture2D<float> tex_depth_prev2 = GetVoxelScreen().GetPrev_depth();

static const Texture2D<float4> tex_gi_prev2 = GetVoxelScreen().GetPrev_gi();

struct Bilinear { float2 origin; float2 weights; };
Bilinear GetBilinearFilter(float2 uv, float2 texSize)
{
	Bilinear result;
	result.origin = floor(uv * texSize - 0.5);
	result.weights = frac(uv * texSize - 0.5);
	return result;
}
float4 GetBilinearCustomWeights(Bilinear f, float4 customWeights)
{
	float4 weights;
	weights.x = (1.0 - f.weights.x) * (1.0 - f.weights.y);
	weights.y = f.weights.x * (1.0 - f.weights.y);
	weights.z = (1.0 - f.weights.x) * f.weights.y;
	weights.w = f.weights.x * f.weights.y;
	return weights * customWeights;
}
float4 ApplyBilinearCustomWeights(float4 s00, float4 s10, float4 s01, float4 s11, float4 w, bool normalize = true)
{
	float4 r = s00 * w.x + s10 * w.y + s01 * w.z + s11 * w.w;
	return r * (normalize ? rcp(dot(w, 1.0)) : 1.0);
}


float get_occlusion(float prev_z, float2 tc, float3 pos, float vl)
{
	float3 prev_pos = depth_to_wpos(prev_z, tc, prevCamera.GetInvViewProj());

	float l = 10*length(pos - prev_pos)/ vl;

	return 0.00005 + (all(tc > 0 && tc < 1) * saturate(1-l));
	//return all(tc > 0 && tc < 1) && l < 0.1 ? 1 : 0.005;
}

static const int2 offset[4] =
{
	int2(0, 0),
	int2(0, 1),
	int2(1, 0),
	int2(1, 1)
};
#define FRAMES 8

struct upscale_result
{
	float4 history;
	float frames;
};

upscale_result get_history(float3 pos, float2 tc, float2 prev_tc, float2 dims, float l)
{
	Bilinear bilinearFilterAtPrevPos = GetBilinearFilter((prev_tc), dims);
	float2 gatherUv = (float2(bilinearFilterAtPrevPos.origin) +0.5) / dims;

	float4 viewZprev = tex_depth_prev2.GatherRed(pointBorderSampler, gatherUv).wzxy;
	// Compute disocclusion basing on plane distance

	float4 occlusion;


	occlusion.x = get_occlusion(viewZprev.x, gatherUv + float2(offset[0]) / dims, pos, l);
	occlusion.y = get_occlusion(viewZprev.y, gatherUv + float2(offset[1]) / dims, pos, l);
	occlusion.z = get_occlusion(viewZprev.z, gatherUv + float2(offset[2]) / dims, pos, l);
	occlusion.w = get_occlusion(viewZprev.w, gatherUv + float2(offset[3]) / dims, pos, l);

	// Sample history
	float4 weights = GetBilinearCustomWeights(bilinearFilterAtPrevPos, occlusion);

	float4 s00 = tex_gi_prev2.SampleLevel(pointBorderSampler, gatherUv + float2(offset[0]) / dims, 0);
	float4 s01 = tex_gi_prev2.SampleLevel(pointBorderSampler, gatherUv + float2(offset[1]) / dims, 0);
	float4 s10 = tex_gi_prev2.SampleLevel(pointBorderSampler, gatherUv + float2(offset[2]) / dims, 0);
	float4 s11 = tex_gi_prev2.SampleLevel(pointBorderSampler, gatherUv + float2(offset[3]) / dims, 0);

	float4 accumSpeedPrev = float4(s00.w, s01.w,s10.w,s11.w);
	 
	// ... read s00, s10, s01, s11 using point filtering
	float4 history =  ApplyBilinearCustomWeights(s00, s10, s01, s11, weights, true);

	accumSpeedPrev = min(FRAMES * accumSpeedPrev + 1.0, FRAMES);
	//	int newFrames = occlusion ? 0 : clamp(prevFrames + 1, 0, 8);

	float accumSpeed = ApplyBilinearCustomWeights(accumSpeedPrev.x, accumSpeedPrev.y, accumSpeedPrev.z, accumSpeedPrev.w, weights, false);

	upscale_result result;

	result.history = float4(history.xyz, accumSpeed);
	result.frames = accumSpeed;
	return result;
}


#ifdef BUILD_FUNC_CS

#include "autogen/VoxelOutput.h"
static const RWTexture2D<float4> tex_noise = GetVoxelOutput().GetNoise();
static const RWTexture2D<float> tex_frames = GetVoxelOutput().GetFrames();

[numthreads(8, 8, 1)]
void  CS(uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex)
{
	//NoiseOutput output = { 0,0,0,0,1};

	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

	
	int2 tc = dispatchID.xy;

	float2 itc = float2(tc + 0.5) / dims;
	
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, itc, 0);
	if (raw_z >= 1)
	{
		tex_noise[dispatchID.xy] = 0;
		tex_frames[dispatchID.xy] = 1;
	}

	float3 pos = depth_to_wpos(raw_z, itc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, itc, 0).xyz * 2 - 1);
	float4 albedo = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, itc, 0);


	float3 v = normalize(pos - camera.GetPosition());
	float3 r = normalize(reflect(v, normal));

	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
	float4 gi = max(0, getGI(itc, pos, pos + scaler * normal / m, normal, v, r, gbuffer.GetNormals()[tc].w, albedo.w));


	float2 delta = speed_tex.SampleLevel(pointClampSampler, itc, 0).xy;
	float2 prev_tc = itc - delta;

	float l = length(pos - camera.GetPosition());
	
	upscale_result reprojected = get_history(pos, tc, prev_tc, dims, l);

	float speed = 1.0 / (1.0 + reprojected.frames);

	gi = lerp(reprojected.history, gi, speed);

	tex_noise[dispatchID.xy] = float4(gi.xyz, raw_z);// accumSpeedPrev / 8;// (accumSpeed / 8) == 1;
	tex_frames[dispatchID.xy] = float(reprojected.frames) / FRAMES;
	//return output;
}

#endif


#ifdef BUILD_FUNC_PS_Resize
[earlydepthstencil]
GI_RESULT PS_Resize(quad_output i) : SV_Target0
{

#ifdef NONE
		return 0;
#endif


	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

	int2 tc = i.tc * dims;

	float4 albedo = gbuffer.GetAlbedo()[tc];

	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1) {
		GI_RESULT result;

	result.screen = 0;
		result.gi = 1;
		return  result;
	}
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float  roughness = pow(gbuffer.GetNormals()[tc].w, 1);
	float metallic = albedo.w;// specular.w;

//	return roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));

	float3 v = normalize(pos - camera.GetPosition());

	float3 r = normalize(reflect(v, normal));

#ifdef SCREEN

	GI_RESULT result;

	result.screen = trace_screen(camera.GetPosition(), v, 0);
	result.gi = 1;
	return  result;
#endif
	float angle = roughness;
	//angle = saturate(0.01+dot(normal, -v));

#ifdef REFLECTION
	GI_RESULT result;

	result.screen = 0;
	result.gi = 0;
	return  result;
#endif

	float4 gi = tex_downsampled.SampleLevel(pointClampSampler, i.tc, 0);
	float2 delta = speed_tex.SampleLevel(pointClampSampler , i.tc, 0).xy;

	float2 prev_tc = i.tc - delta;

	float4 prev_gi = tex_gi_prev.SampleLevel(linearClampSampler, prev_tc, 0);
	float prev_z = tex_depth_prev.SampleLevel(pointClampSampler, prev_tc, 0);
	float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prevCamera.GetInvViewProj());


	float dist = length(pos - camera.GetPosition()) * MOVE_SCALER;
	float l = length(pos - prev_pos) / dist;

	gi = lerp(gi, prev_gi, saturate(0.95 - pow(l,0.5)));

#ifdef INDIRECT
	GI_RESULT result;

	result.screen = gi;
	result.gi = gi;
	return  result;
#endif


	GI_RESULT res;
	res.screen = albedo * gi;// *tex_color[tc] + albedo * gi;// float4(PBR(gi, 0, albedo, normal, v, 0.2, roughness, metallic), 1);
	res.gi = gi;

	res.screen.w = 1;
	return  res;

}

#endif