#define SIZE 16
#define FIX 8
#include "2D_screen_simple.h"

#include "Common.hlsl"
#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelUpscale.h"

static const Camera camera = GetFrameInfo().GetCamera();

static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelScreen().GetGbuffer();


static const float time =  GetFrameInfo().GetTime();


//#define NONE
//#define SCREEN
//#define INDIRECT
//#define REFLECTION

#define SCALER 1

static const float3 voxel_min = voxel_info.GetMin().xyz;
static const float3 voxel_size= voxel_info.GetSize().xyz;

static const float scaler = voxel_size / 256;
static const Texture3D<float4> voxels = GetVoxelScreen().GetVoxels();
static const TextureCube<float4> tex_cube = GetVoxelScreen().GetTex_cube();
static const Texture2D<float2> speed_tex = GetVoxelScreen().GetGbuffer().GetMotion();


//static const Texture2D<float4> tex_color = GetVoxelScreen().GetTex_color();
static const Texture2D<float4> tex_gi_prev = GetVoxelUpscale().GetTex_gi_prev();
static const Texture2D<float> tex_depth_prev = GetVoxelUpscale().GetTex_depth_prev();
static const Texture2D<float4> tex_downsampled = GetVoxelUpscale().GetTex_downsampled();



//#define voxel_min float3(-150,-150,-150)
//#define voxel_size float3(300,300,300)

float4 get_voxel(float3 pos, float level)
{
	float4 color =  voxels.SampleLevel(linearSampler, pos, level);
//color.rgb *= 1 + level / 2;

return color;
}
float get_alpha(float3 pos, float level)
{
return voxels.SampleLevel(linearSampler,pos,level).a;
}

float get_start_dist(float3 origin,float3 dir, float minDiameter, float angle)
{
	 float3 samplePos = 0;
	 
float dist = minDiameter*1;
float prev_dist=dist;
   samplePos = origin + dir * dist;
	float minVoxelDiameterInv = 1.0 / minDiameter;

	float cur_angle = 1;
   while(cur_angle>angle&&dist<1&& all(samplePos <= 1) && all(samplePos >= 0))
   {
	  float sampleDiameter = max(minDiameter, cur_angle * dist);
	  float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
  
	  samplePos = origin + dir * dist;
	
	 float sampleValue = get_alpha(samplePos, sampleLOD);

   if(sampleValue>0)
   {
	cur_angle/=2;
	
   }else
   {
	cur_angle*=1.1;
	prev_dist = dist;
	dist += sampleDiameter;
   }
   
   }
  
  
	return minDiameter;
}


float3 PrefilterEnvMap(float Roughness, float3 R)
{
	return  tex_cube.SampleLevel(linearSampler, R, 0*Roughness * 5).rgb;
}




float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return GetFrameInfo().IntegrateBRDF(Roughness, Metallic, NoV);
}





float3 get_sky(float3 dir, float level)
{
	return PrefilterEnvMap(1, dir);
}



float4 trace_screen(float3 origin, float3 dir, float level)
{
	origin = saturate(((origin - (voxel_min)) / voxel_size));
	dir = normalize(dir);
	float3 samplePos = 0;
	float4 accum = 0;
	// the starting sample diameter
	float minDiameter = 1.0 / 512;// *(1 + 2 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;
	// push out the starting point to avoid self-intersection
	float startDist = 0;// get_start_dist(origin, dir, minDiameter, angle);

	float maxDist = 1;
	float dist = startDist;

	while (dist <= maxDist && accum.w < 0.9 && all(samplePos <= 1) && all(samplePos >= 0))
	{
	//	float sampleDiameter = minDiameter + level;//max(minDiameter, angle * dist);
		float sampleDiameter = minDiameter + level;// *dist;

		float sampleLOD = level;// log2(sampleDiameter * minVoxelDiameterInv);
		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);

		/*if (sampleValue.w > 0)
		{
			if (level == 0)
				return sampleValue / sampleValue.w;
			else
				level =saturate(level-0.1);
		}
		else
		{
			level = saturate(level + 0.1);
			dist += minDiameter*sampleDiameter;// pow(2, level);
		}*/
		float sampleWeight = saturate(1 - accum.w);
		accum += sampleValue * sampleWeight;
		dist += minDiameter*sampleDiameter;// pow(2, level);
	}


	return accum;
}

float4 trace(float4 start_color, float3 view, float3 origin,float3 dir,float3 normal, float angle)
{
	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14) ;

	float angle_coeff = saturate(max_angle/(angle+0.01));
	//angle = min(angle, max_angle);
	origin = saturate(((origin - (voxel_min)) / voxel_size));
 
	float3 samplePos = 0;
	float4 accum = start_color;
	// the starting sample diameter
	float minDiameter = 1.0 / 256;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;
  
	float maxDist = 1;
	float dist = minDiameter;

	float max_accum =0.9;
	while (dist <= maxDist && accum.w <max_accum && all(samplePos <= 1) && all(samplePos >= 0))
	{
	   float sampleDiameter = minDiameter+ angle * dist;
	
	   float sampleLOD = 2*log2(sampleDiameter * minVoxelDiameterInv);
		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);//* float4(1,1,1,1 + sampleLOD/4);
	//	sampleValue.w *= 0.8;// (1 + 1 * angle);
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
	float sampleWeight = saturate(max_accum - accum.w*SCALER) / max_accum;
	accum.xyz+= sky* pow(sampleWeight,1);
		
   return accum;// / saturate(accum.w);
}



float3 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res.xyz /= res.w;
	return float3(res.xy * float2(0.5, -0.5) + float2(0.5, 0.5), res.z);
}
float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float4 get_ssgi(float3 pos, float3 normal, float3 dir,float dist, float2 orig_tc)
{

	return 0;
	/*
#define MAX_DIST 0.00001
#define COUNT 4

	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

	float4 res = 0;
	for (int j = 0; j < COUNT; j++)
	{

		float2 tc = project_tc(pos + dist*dir*(j)/COUNT, camera.GetViewProj());

		if (all(abs(tc - orig_tc) < 1.0f / dims)) continue;

		float level = 0;// 2 * float(j) / COUNT;// 1 + 2 * float(j) / COUNT;// 3 * float(j) / 16;// length(poisson[j] * 2);// *float(j) / 16;
		float4 color = tex_color.SampleLevel(pointClampSampler , tc, 0);
		float z = gbuffer.GetDepth().SampleLevel(pointClampSampler , tc, level).x;
		float3 t_pos = depth_to_wpos(z, float2(tc.xy), camera.GetInvViewProj());

		float3 n = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler , tc, level).xyz * 2 - 1);
		float len = length(pos - t_pos);

		float3 delta = (t_pos - pos) / len;
		float s =  dist*pow(saturate(dot(normal, delta)),3) / (len);

		s *= 4;
		s = saturate(s);
		float w = dist / (len*len + dist);
		w *= saturate(dot(-n, delta) * 4);
		w *= saturate(dot(normal, delta) * 4);

		color.xyz *= w / (len / dist + 1);
		color.w = s;
	
		float sampleWeight = saturate(1 - res.w);
		res += color * sampleWeight;

		//res += w*s*color;
	}

//return 0;
	return res;*/

}
float4 get_direction(float3 pos,float v, float3 orig_pos, float3 normal,  float3 dir, float k,float a,float dist, float2 orig_tc)
{
	
	

	//return 5*get_voxel(origin,2);
	//float4 ssgi =  get_ssgi(orig_pos, normal, dir, dist, orig_tc);
//ssgi += 1 * 1 * pow(1-ssgi.w, 2);

//return ssgi;

float4 gi = trace(0,v, pos, dir, normal, a);
	return gi;
//	return ssgi+gi*(1 - ssgi.w);// // *pow(saturate(dot(normal, dir)), 4);
}

float4 getGI(float2 tc,float3 orig_pos, float3 Pos, float3 Normal, float3 V,float r, float roughness, float metallic)
{
	float a = 0.1;

	float4 Color = 0;
	float k = scaler;

	float sini = sin(time * 220 + float(tc.x));
	float cosi = cos(time * 220 + float(tc.y));
	float rand = rnd(float2(sini, cosi));

	float rcos = cos(6.14*rand);
	float rsin = sin(6.14*rand);
	float rand2 = rnd(float2(cosi, sini));

	float t = lerp(0.5,1, rand2);

	float3 right =rsin*t*normalize(cross(Normal, float3(0, 1, 0.1)));
	float3 tangent =rcos*t*normalize(cross(right, Normal));

	float3 dir = normalize(Normal + right + tangent);
	float2 EnvBRDF = IntegrateBRDF(roughness, metallic, dot(Normal,dir));

	Color= EnvBRDF.x*get_direction(Pos+ 1*Normal,V, orig_pos, Normal,normalize(Normal + right + tangent), k, a, length(Pos - camera.GetPosition())/20,tc);

	return 1*Color;

}
#include "PBR.hlsl"


#define MOVE_SCALER 0.1

struct GI_RESULT
{
	float4 screen: SV_Target0;
	float4 gi: SV_Target1;
};

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
   int2 tc = i.tc*dims;
   
	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1)
		return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz*2-1);

	float4 albedo = gbuffer.GetAlbedo()[tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);
  
  float3 v = normalize(pos-camera.GetPosition());
  float3 r = normalize(reflect(v,normal));
#ifdef SCREEN
  result.screen= trace_screen(camera.GetPosition(), v, 0);

 return result;
#endif
  //return voxels.SampleLevel(linearSampler,index,0);
	float m = 1*max(max(abs(normal.x),abs(normal.y)),abs(normal.z));
 

	
	float w = 0.1;
	float4 res =  tex_downsampled.SampleLevel(pointClampSampler, i.tc, 0)* w;

#define R 1

	for (int x = -R; x <= R; x++)
		for (int y = -R; y <= R; y++)
		{
			float2 t_tc = i.tc + float2(x, y) / dims;


			int2 offset = float2(x, y);

			float t_raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler , i.tc, 1, offset);
			float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.GetInvViewProj());
			float3 t_normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler , i.tc, 1, offset).xyz * 2 - 1);


			float4 t_gi =  tex_downsampled.SampleLevel(pointClampSampler, i.tc, 0, offset);

		
			float cur_w = saturate(1 - length(t_pos - pos));// 1.0 / (8 * length(t_pos - pos) + 0.1);
			cur_w *= pow(saturate(dot(t_normal, normal)), 16);
		//	cur_w = saturate(cur_w - 0.1);
			
			res += cur_w*t_gi;

			w += cur_w;



		}
	float4 gi =  res / w;
	
	

	//float4 gi = max(0, getGI(i.tc, pos, pos + scaler * normal / m, normal, v, r, gbuffer.GetNormals()[tc].w, albedo.w));





  float2 delta = speed_tex.SampleLevel(pointClampSampler , i.tc, 0).xy;

  float2 prev_tc = i.tc - delta;
  float4 prev_gi = tex_gi_prev.SampleLevel(linearClampSampler, prev_tc, 0);
  float prev_z = tex_depth_prev.SampleLevel(linearClampSampler, prev_tc, 0);



  float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prevCamera.GetInvViewProj());

  float l = length(pos - prev_pos)* MOVE_SCALER;
  gi =  lerp(gi, prev_gi, saturate(0.95 - l));
  

  result.gi = gi;


#ifdef INDIRECT
  result.screen = gi;
  return result;
#endif
  //return gi;
// return gi;
  //specular.w = pow(specular.w, 1);

 float  roughness = pow(gbuffer.GetNormals()[tc].w,1);
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

	float4 PS_Low(quad_output i) : SV_Target0
	{
	
#ifdef NONE
		return 0;
#endif
	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

	int2 tc = i.tc*dims;

	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, i.tc, 1);
	if (raw_z >= 1) return 0;

	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, i.tc, 1).xyz * 2 - 1);
	float4 albedo = gbuffer.GetAlbedo().SampleLevel(pointClampSampler, i.tc, 0);


	float3 v = normalize(pos -camera.GetPosition());
	float3 r = normalize(reflect(v,normal));

	float m = 1 * max(max(abs(normal.x),abs(normal.y)),abs(normal.z));
	float4 gi = max(0, getGI(i.tc, pos, pos + scaler * normal / m, normal, v, r, gbuffer.GetNormals()[tc].w, albedo.w));

	return gi;
	}



[earlydepthstencil]
GI_RESULT PS_Resize(quad_output i) : SV_Target0
	{
	
#ifdef NONE
		return 0;
#endif


	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

	int2 tc = i.tc*dims;

	float4 albedo = gbuffer.GetAlbedo()[tc];

	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1) {
		GI_RESULT result;

	result.screen =0;
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
	result.gi =1;
	return  result;
#endif
	float angle = roughness ;
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
	float prev_z =  tex_depth_prev.SampleLevel(pointClampSampler, prev_tc, 0);
	float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prevCamera.GetInvViewProj());


	float dist = length(pos - camera.GetPosition()) * MOVE_SCALER;
	float l =  length(pos - prev_pos) / dist;

	gi = lerp(gi, prev_gi, saturate(0.95 - pow(l,0.5)));

#ifdef INDIRECT
	GI_RESULT result;

	result.screen = gi;
	result.gi = gi;
	return  result;
#endif


	GI_RESULT res;
	res.screen = albedo * gi;// *tex_color[tc] + albedo * gi;// float4(PBR(gi, 0, albedo, normal, v, 0.2, roughness, metallic), 1);
	res.gi =  gi;

	res.screen.w = 1;
	return  res;

	}