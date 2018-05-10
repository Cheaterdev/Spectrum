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
Texture3D<float4> voxels : register(t0,space1);

SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);

TextureCubeArray<float4> tex_cube : register(t0,space2);
Texture2D<float2> speed_tex : register(t2, space2);
Texture2D<float4> tex_color : register(t3, space2);
//Texture2D<float4> tex_gi_prev : register(t4, space2);
Texture2D<float4> tex_ray : register(t4, space2);

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

float4 get_sky(float3 dir, float level)
{

	//return 0;
	level *= 16;
	//return tex_cube.Sample(LinearSampler, float4(dir,6));
	int l0 = level;
	int l1 = l0 + 1;


	return lerp(tex_cube.Sample(LinearSampler, float4(dir, l0)), tex_cube.Sample(LinearSampler, float4(dir, l1)), level - l0);


}
float4 trace_refl(float4 start_color, float3 origin,float3 dir, float3 normal, float angle)
{
	//return 0;
	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14)/1;

	float angle_bad = saturate(abs(angle - max_angle)-0.5);
	//dir = normalize(dir+angle_bad*normal);

	max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14) / 1;

	float best_angle = min(angle, max_angle);


	float angle_error = abs(best_angle - angle);
	angle = min(angle, 0.34);
	
	//dir=normalize(lerp(dir,normal,angle_error));

	origin = saturate(((origin + 12*scaler*normal*angle_error- (voxel_min)) / voxel_size));

	float3 samplePos = 0;
    float4 accum = start_color;
    // the starting sample diameter
	float minDiameter = 1.0 / 512;// *(1 + 4 * angle);
	float minVoxelDiameterInv = 1.0 / minDiameter;
	float maxDist = 1;
	float dist = minDiameter;// +angle_error * minDiameter;// +angle*minDiameter / 2;// +16 * angle_bad*minDiameter;
	float max_accum = 0.95;// -angle*0.8;

    while (dist <= maxDist && accum.w <max_accum && all(samplePos <= 1) && all(samplePos >= 0))
    {
	  
		float sampleDiameter = minDiameter + angle * dist;

		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		samplePos = origin + dir * dist;
		float4 sampleValue = get_voxel(samplePos, sampleLOD);//* float4(1,1,1,1 + sampleLOD/4);
//		sampleValue.rgb *= saturate(dist*100);									 //	sampleValue.w*=(1+1*angle);

	//	sampleValue.w = saturate(sampleValue.w+ saturate(1-dist*200));
		float sampleWeight = saturate(1 - accum.w);
		accum += sampleValue * sampleWeight;
		dist += sampleDiameter;

    }
	
	//float angle_coeff = saturate(max_angle / (angle + 0.01));

//	accum.xyz *= angle_coeff;

	float4 sky = get_sky(dir, angle);
   float sampleWeight = saturate(max_accum - accum.w)/ max_accum;
   accum += sky * pow(sampleWeight, 1);// *angle_coeff;
		
		return accum;// / accum.w;
}

#include "PBR.hlsl"


struct GI_RESULT
{
	float4 screen: SV_Target0;
	//float4 gi: SV_Target1;
};

float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float2 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return res.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
}

float calc_vignette(float2 inTex)
{
	float2 T = saturate(inTex);
	// float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
	float vignette = 4 * (T.y * ((1 - T.y)));
	return vignette;
}
float4 get_ssr2(float3 pos, float3 normal, float2 tc,float3 r, float angle, out float dist)
{
	float3 refl_pos = pos;
	float sini = sin(time * 220 + float(tc.x));
	float cosi = cos(time * 220 + float(tc.y));
	float rand = rnd(float2(sini, cosi));
	float rand2 = rnd(float2(cosi, sini));

	float rcos = cos(6.14*rand);
	float rsin = sin(6.14*rand);

	float3 right = normalize(cross(r, float3(0, 1, 0.1)));
	float3 tangent = normalize(cross(right, r));


	r += 0.1*rand2*angle*(tangent*rsin + right*rcos)/4;

	r = normalize(r);
	float errorer = distance(camera.position, pos) / 1;

	float orig_dist = distance(camera.position, pos);
	dist = orig_dist/10 ; //lerp(10,20,1-fresnel);

	float raw_z_reflected = 0;
	float2 reflect_tc = tc;
	float	level = 0;// clamp(angle * 32 * dist / orig_dist, 0, 8);

	for (int i = 0; i < 8; i++)
	{
		reflect_tc = project_tc(pos + dist * r, camera.view_proj);
		raw_z_reflected = depth_buffer.SampleLevel(LinearSampler, reflect_tc, 0).x;
		refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);
		dist = distance(pos, refl_pos);



		//level = clamp(angle * 32 * dist  / orig_dist, 0, 8);
		/*if(raw_z_reflected<raw_z)
		{/
		dist*=3;
		level*=2;
		}	//level=12;*/
	}

//	reflect_tc = project_tc(pos + dist * r, camera.view_proj);
	float3 delta_position = pos - refl_pos;
	float reflection_distance = distance(pos + dist * r, refl_pos);
	dist = distance(pos, refl_pos);
	float vignette = calc_vignette(reflect_tc);
	float bad = 1;// pow(saturate((dot(r, -delta_position) / dist)), 1);

	//bad*= pow(saturate((dot(r, -delta_position) / dist)), 1);
	bad *=saturate((dot(normal, -delta_position ) ))>0;
	

	bad *=dist > 0.1/ errorer;// saturate(1 - dot(v, r));
	bad *= (raw_z_reflected < 1);
	// level +=  8 * saturate(1 - 1 * bad);


	float4 reflect_color = tex_color.SampleLevel(LinearSampler, reflect_tc,min(18*dist*angle, 8));
	float4 res = 0;

	//return reflect_color;
	//float lerper = 1 - reflect_color.w * pow(0.9, (1 - bad) * 32);

	float3 direct = vignette * reflect_color.xyz *(bad);// pow(0.9, (1 - bad) * 32);


	dist*=bad;
	return float4(direct, vignette *bad);// *saturate(1 - dist*angle / orig_dist);
}

float4 get_ssr(float3 pos, float3 normal, float2 tc, float3 r, float angle, out float dist)
{

	float4 ray = tex_ray.SampleLevel(LinearSampler, tc, 0);


	float2 rtc = project_tc(pos + ray.x * r, camera.view_proj);

	float4 albedo = tex_color.SampleLevel(LinearSampler, rtc, clamp(ray.x*angle*2,0, 8));

	float vignette = calc_vignette(rtc);


	albedo.w = vignette*(1-angle);
	dist = 1;
	return ray.w*albedo;
}
[earlydepthstencil]
GI_RESULT PS(quad_output i)
{

	GI_RESULT result;
	
	result.screen =0;
	//result.gi = 0;
	//return result;
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



 float  roughness = pow(gbuffer[1][tc].w,1);
 float metallic = albedo.w;// specular.w;
  //float fresnel = calc_fresnel(1- roughness, normal, v);

 float angle = roughness;
 float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
 float4 ssr =0;




 float4 downsampled = tex_ray.SampleLevel(LinearSampler, i.tc, 0);



 float4 reflection = trace_refl(ssr, pos + scaler*normal / m, normalize(r), normal, angle);

 //ssr = ssr+get_sky(r,1)*(1 - ssr.w);
 result.screen = tex_color[tc] +  float4(PBR(0, reflection, albedo, normal, v, 0.2, 0, metallic), 0);
 return result;

	}


[earlydepthstencil]
GI_RESULT PS_resize(quad_output i)
{

	GI_RESULT result;

	result.screen = 0;
	//result.gi = 0;
	//return result;
	float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float raw_z = depth_buffer[tc.xy];
	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 normal = normalize(gbuffer[1][tc].xyz * 2 - 1);

	float4 albedo = gbuffer[0][tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.position);
	float3 r = normalize(reflect(v, normal));



	float  roughness = pow(gbuffer[1][tc].w, 1);
	float metallic = albedo.w;// specular.w;
							  //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
	float4 ssr = 0;




	float4 downsampled = tex_ray.SampleLevel(LinearSampler, i.tc, 0);



	float4 reflection = downsampled;// downsampled;// trace_refl(ssr, pos + scaler*normal / m, normalize(r), normal, angle);

									//ssr = ssr+get_sky(r,1)*(1 - ssr.w);
	result.screen = tex_color[tc] + float4(PBR(0, reflection, albedo, normal, v, 0.2, 0, metallic), 0);
	return result;

}



[earlydepthstencil]
GI_RESULT PS_low(quad_output i)
{

	GI_RESULT result;

	result.screen = 0;
	//result.gi = 0;
	//return result;
	float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float raw_z = depth_buffer[tc.xy];
	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 normal = normalize(gbuffer[1][tc].xyz * 2 - 1);

	float4 albedo = gbuffer[0][tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.position);
	float3 r = normalize(reflect(v, normal));

//	float4 downsampled = tex_ray.SampleLevel(LinearSampler, i.tc, 0);

	float  roughness = pow(gbuffer[1][tc].w, 1);
	float metallic = albedo.w;// specular.w;
							  //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
	float4 ssr = 0;

	//[branch]
	//if (angle < 0.1)

	float dist = 0;
	ssr = get_ssr(pos, normal, i.tc, r, angle, dist);

	//ssr*= saturate(4 - dist/5)*saturate(1- roughness);


	//ssr.w = 1;
	float4 reflection = trace_refl(ssr, pos + scaler*normal / m, normalize(r), normal, angle);

	//ssr = ssr+get_sky(r,1)*(1 - ssr.w);
	result.screen = reflection;;/// tex_color[tc] + float4(PBR(0, reflection, albedo, normal, v, 0.2, 0, metallic), 0);
	return result;

}
