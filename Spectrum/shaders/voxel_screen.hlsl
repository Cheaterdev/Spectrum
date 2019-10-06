#define SIZE 16
#define FIX 8

#include "Common.hlsl"

//#define NONE
//#define SCREEN
//#define INDIRECT
//#define REFLECTION

#define SCALER 1
cbuffer cbCamera : register(b0)
{
	camera_info camera;
	camera_info prev_camera;
};
  //	#define voxel_min float3(-50,-50,-50)
	//#define voxel_size float3(100,100,100)
cbuffer voxel_info : register(b1)
{
   //	float3 voxel_min:packoffset(c0);
	//float3 voxel_size:packoffset(c3);
	
	float voxel_min_x;
	float voxel_min_y;
	float voxel_min_z;
	float voxel_size_x;
	float voxel_size_y;
	float voxel_size_z;

	float time;
	
};
static const float3 voxel_min=float3(voxel_min_x,voxel_min_y,voxel_min_z);
static const float3 voxel_size=float3(voxel_size_x,voxel_size_y,voxel_size_z);

static const float scaler = voxel_size / 256;

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture3D<float4> brdf : register(t4);
Texture3D<float4> voxels : register(t0,space1);

SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);
SamplerState LinearSamplerClamp: register(s2);

TextureCube<float4> tex_cube : register(t0,space2);
Texture2D<float2> speed_tex : register(t2, space2);
Texture2D<float4> tex_color : register(t3, space2);
Texture2D<float4> tex_gi_prev : register(t4, space2);
Texture2D<float> tex_depth_prev : register(t5, space2);

struct quad_output
{
float4 pos : SV_POSITION;
float2 tc : TEXCOORD0;
};

quad_output VS(uint index : SV_VERTEXID)
{
	static float2 Pos[] =
	{
		float2(-1, 1),
		float2(1, 1),
		float2(-1, -1),
		float2(1, -1)
	};
	static float2 Tex[] =
	{

		float2(0, 0),
		float2(1, 0),
		float2(0, 1),
		float2(1, 1),
	}; 
	quad_output Output;
	Output.pos = float4(Pos[index], 0.3, 1);
	Output.tc = Tex[index];
	return Output;
}


//#define voxel_min float3(-150,-150,-150)
//#define voxel_size float3(300,300,300)

float4 get_voxel(float3 pos, float level)
{
float4 color= voxels.SampleLevel(LinearSampler,pos,level);
//color.rgb *= 1 + level / 2;

return color;
}
float get_alpha(float3 pos, float level)
{
return voxels.SampleLevel(LinearSampler,pos,level).a;
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
	return  tex_cube.SampleLevel(LinearSampler, R, Roughness * 5).rgb;
}




float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	return brdf.SampleLevel(LinearSamplerClamp, float3(Roughness, Metallic, 0.5 + 0.5*NoV), 0);
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
	float minDiameter = 1.0 / 1024;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;
  
	float maxDist = 1;
	float dist = minDiameter;

	float max_accum =0.9;
	while (dist <= maxDist && accum.w <max_accum && all(samplePos <= 1) && all(samplePos >= 0))
	{
	   float sampleDiameter = minDiameter+ angle * dist;
	
	   float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
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
	accum.xyz+= 1*sky * pow(sampleWeight,2);
		
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

#define MAX_DIST 0.00001
#define COUNT 4




	//float2 my_tc = project_tc(pos, camera.view_proj);
//	float2 new_tc = project_tc(pos + dir, camera.view_proj);


	//float2 rdir = normalize(new_tc - my_tc);
	//float2 direction = MAX_DIST*rdir / COUNT;


	//float sini = sin(float(my_tc.x));
//	float cosi = cos( float(my_tc.y));
	
	//float rand2 = 0.5 + 0.5*rnd(float2(cosi, sini));
	float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);

	float4 res = 0;
	for (int j = 0; j < COUNT; j++)
	{





		//float2 delta_tc = poisson[16*rand*j]/16;// float2(rcos*poisson[j].x - rsin*poisson[j].y, rsin*poisson[j].x + rcos*poisson[j].y);



	//	float2 tc = my_tc + j*direction;// rand*j*poisson[j] / 120.0f;

		float2 tc = project_tc(pos + dist*dir*(j)/COUNT, camera.view_proj);

		if (all(abs(tc - orig_tc) < 1.0f / dims)) continue;

		float level = 0;// 2 * float(j) / COUNT;// 1 + 2 * float(j) / COUNT;// 3 * float(j) / 16;// length(poisson[j] * 2);// *float(j) / 16;
		float4 color = tex_color.SampleLevel(PixelSampler, tc, 0);
		float z = depth_buffer.SampleLevel(PixelSampler, tc, level).x;
		float3 t_pos = depth_to_wpos(z, float2(tc.xy), camera.inv_view_proj);

		float3 n = normalize(gbuffer[1].SampleLevel(PixelSampler, tc, level).xyz * 2 - 1);
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
		//color.w = saturate(color.w *114);

		//color.xyz *= color.w;

		float sampleWeight = saturate(1 - res.w);
		res += color * sampleWeight;

		//res += w*s*color;
	}

//return 0;
	return res;

}
float4 get_direction(float3 pos,float v, float3 orig_pos, float3 normal,  float3 dir, float k,float a,float dist, float2 orig_tc)
{
	
	float4 ssgi =  get_ssgi(orig_pos, normal, dir, dist, orig_tc);
//ssgi += 1 * 1 * pow(1-ssgi.w, 2);

//return ssgi;

float4 gi = trace(ssgi,v, pos, dir, normal, a);
	return gi;
//	return ssgi+gi*(1 - ssgi.w);// // *pow(saturate(dot(normal, dir)), 4);
}

float4 getGI(float2 tc,float3 orig_pos, float3 Pos, float3 Normal, float3 V,float r, float roughness, float metallic)
{
	float a = 0.4;

	float4 Color = 0;
	float k = scaler;

	float sini = sin(time * 220 + float(tc.x));
	float cosi = cos(time * 220 + float(tc.y));
	float rand = rnd(float2(sini, cosi));

	float rcos = cos(6.14*rand);
	float rsin = sin(6.14*rand);
	float rand2 = rnd(float2(cosi, sini));

	float t = rand2 *3;

	float3 right =rsin*t*normalize(cross(Normal, float3(0, 1, 0.1)));
	float3 tangent =rcos*t*normalize(cross(right, Normal));

	//float3 right = t*normalize(cross(Normal, float3(0, 1, 0.1)));
	//float3 tangent = t*normalize(cross(right, Normal));

	float3 dir = normalize(Normal + right + tangent);
	float2 EnvBRDF = IntegrateBRDF(roughness, metallic, dot(Normal,dir));

	Color= 3*EnvBRDF.x*get_direction(Pos+ 2*Normal,V, orig_pos, Normal,normalize(Normal + right + tangent), k, a, length(Pos - camera.position)/20,tc);// trace(Pos + k*normalize(Normal + right), normalize(Normal + right), a);

	/*Color += get_direction(Pos, orig_pos, Normal, normalize(Normal + right), k,a);// trace(Pos + k*normalize(Normal + right), normalize(Normal + right), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal - right), k, a);//trace(Pos + k*normalize(Normal - right), normalize(Normal - right), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal + tangent), k, a);//trace(Pos + k*normalize(Normal + tangent), normalize(Normal + tangent), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal- tangent), k, a);//trace(Pos + k*normalize(Normal - tangent), normalize(Normal - tangent), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal + tangent - right), k, a);//trace(Pos + k*normalize(Normal + tangent - right), normalize(Normal + tangent - right), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal + tangent + right), k, a);//trace(Pos + k*normalize(Normal + tangent + right), normalize(Normal + tangent + right), a);
	Color += get_direction(Pos, orig_pos, Normal, normalize(Normal - tangent - right), k, a);//trace(Pos + k*normalize(Normal - tangent - right), normalize(Normal - tangent - right), a);
   Color += get_direction(Pos, orig_pos, Normal, normalize(Normal - tangent + right), k, a);//trace(Pos + k*normalize(Normal - tangent + right), normalize(Normal - tangent + right), a);
   Color /= 4;*/
   // Color =1- trace_ao(Pos, Normal,2);
	return 1*Color;//*pow(1-Color.a,0.01);

}
#include "PBR.hlsl"


#define MOVE_SCALER 1

struct GI_RESULT
{
	float4 screen: SV_Target0;
	float4 gi: SV_Target1;
};
Texture2D<float4> tex_downsampled : register(t1, space2);

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
   gbuffer[0].GetDimensions(dims.x, dims.y);
   int2 tc = i.tc*dims;
   
	float raw_z = depth_buffer[tc.xy];
	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 normal = normalize(gbuffer[1][tc].xyz*2-1);

	float4 albedo = gbuffer[0][tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);
  
  float3 v = normalize(pos-camera.position);
  float3 r = normalize(reflect(v,normal));
#ifdef SCREEN
  result.screen= trace_screen(camera.position, v, 0);

 return result;
#endif
  //return voxels.SampleLevel(LinearSampler,index,0);
	float m = 1*max(max(abs(normal.x),abs(normal.y)),abs(normal.z));
 

	
	float w = 0.01;
	float4 res = tex_downsampled.SampleLevel(PixelSampler, i.tc, 0)*w;

#define R 1

	for (int x = -R; x <= R; x++)
		for (int y = -R; y <= R; y++)
		{
			float2 t_tc = i.tc + float2(x, y) / dims;

			float t_raw_z = depth_buffer.SampleLevel(PixelSampler, t_tc, 1);
			float3 t_pos = depth_to_wpos(t_raw_z, t_tc, camera.inv_view_proj);
			float3 t_normal = normalize(gbuffer[1].SampleLevel(PixelSampler, t_tc, 1).xyz * 2 - 1);


			float4 t_gi = tex_downsampled.SampleLevel(PixelSampler, t_tc, 0);

		
			float cur_w = saturate(1 - length(t_pos - pos)/2);// 1.0 / (8 * length(t_pos - pos) + 0.1);
			cur_w *= pow(saturate(dot(t_normal, normal)), 4);
		//	cur_w = saturate(cur_w - 0.1);
			
			res += cur_w*t_gi;

			w += cur_w;



		}
	float4 gi =  res / w;

	







  float2 delta = speed_tex.SampleLevel(PixelSampler, i.tc, 0).xy;

  float2 prev_tc = i.tc - delta;
  float4 prev_gi = tex_gi_prev.SampleLevel(LinearSampler, prev_tc, 0);
  float prev_z = tex_depth_prev.SampleLevel(LinearSampler, prev_tc, 0);



  float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prev_camera.inv_view_proj);

  float l = length(pos - prev_pos)*MOVE_SCALER;
  gi = lerp(gi, prev_gi, saturate(0.95 - l));


  result.gi = gi;
#ifdef INDIRECT
  result.screen = gi;
  return result;
#endif
  //return gi;
// return gi;
  //specular.w = pow(specular.w, 1);

 float  roughness = pow(gbuffer[1][tc].w,1);
 float metallic = albedo.w;// specular.w;
  //float fresnel = calc_fresnel(1- roughness, normal, v);

 float angle = roughness;

 float4 reflection = 0;// trace_refl(pos + scaler*normal / m, normalize(r), normal, angle);
 //return metallic*reflection;

#ifdef REFLECTION
 result.screen = reflection;
 return result;
#endif


 result.screen = 1 * tex_color[tc] + albedo*gi;// float4(PBR(gi, reflection, albedo, normal, v, 0.2, roughness, metallic), 1);
 return result;
//	
	}

	float4 PS_Low(quad_output i) : SV_Target0
	{
#ifdef NONE
		return 0;
#endif
		float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float raw_z = depth_buffer[tc.xy];
	if (raw_z >= 1) return 0;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 normal = normalize(gbuffer[1][tc].xyz * 2 - 1);

	float4 albedo = gbuffer[0][tc];

	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.position);
	float3 r = normalize(reflect(v,normal));


	//return voxels.SampleLevel(LinearSampler,index,0);
	float m = 1 * max(max(abs(normal.x),abs(normal.y)),abs(normal.z));
	float4 gi = max(0,getGI(i.tc, pos, pos + scaler*normal / m,normal,v,r, gbuffer[1][tc].w, albedo.w));

//	return 1;
	return  gi;
	
	}



[earlydepthstencil]
GI_RESULT PS_Resize(quad_output i) : SV_Target0
	{
#ifdef NONE
		return 0;
#endif


		float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float4 albedo = gbuffer[0][tc];

	float raw_z = depth_buffer[tc.xy];
	if (raw_z >= 1) {
		GI_RESULT result;

	result.screen =0;
		result.gi = 0;
		return  result;
	}
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 normal = normalize(gbuffer[1][tc].xyz * 2 - 1);

	float  roughness = pow(gbuffer[1][tc].w, 1);
	float metallic = albedo.w;// specular.w;

//	return roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));

	float3 v = normalize(pos - camera.position);

	float3 r = normalize(reflect(v, normal));

#ifdef SCREEN

	GI_RESULT result;

	result.screen = trace_screen(camera.position, v, 0);
	result.gi = 0;
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

	
	float4 gi = tex_downsampled.SampleLevel(LinearSampler, i.tc, 0);






	float2 delta = speed_tex.SampleLevel(PixelSampler, i.tc, 0).xy;

	float2 prev_tc = i.tc - delta;
	float4 prev_gi = tex_gi_prev.SampleLevel(LinearSampler, prev_tc, 0);
	float prev_z = tex_depth_prev.SampleLevel(LinearSampler, prev_tc, 0);



	float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prev_camera.inv_view_proj);



	float dist = length(pos - camera.position) / 100;
	float l = length(pos - prev_pos) / dist;

	gi =  lerp(gi, prev_gi, saturate(0.95 - l));

#ifdef INDIRECT
	GI_RESULT result;

	result.screen = gi;
	result.gi = gi;
	return  result;
#endif
//	return gi;


	GI_RESULT res;
	res.screen = 1 * tex_color[tc] + albedo*gi;// float4(PBR(gi, 0, albedo, normal, v, 0.2, roughness, metallic), 1);
	res.gi =  gi;
	return  res;

	}