#define SIZE 16
#define FIX 8
#include "Common.hlsl"
#include "autogen/FrameInfo.h"
#include "autogen/VoxelInfo.h"
#include "autogen/VoxelScreen.h"
#include "autogen/VoxelUpscale.h"

static const Camera camera = GetFrameInfo().GetCamera();

static const Camera prevCamera = GetFrameInfo().GetPrevCamera();
static const VoxelInfo voxel_info = GetVoxelInfo();
static const GBuffer gbuffer = GetVoxelScreen().GetGbuffer();


static const float time = 0;// GetFrameInfo().GetTime();


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

static const Texture2D<float2> speed_tex = gbuffer.GetMotion();

static const Texture2D<float4> tex_downsampled = GetVoxelUpscale().GetTex_downsampled();


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
float4 color= voxels.SampleLevel(linearSampler,pos,level);
//color.rgb *= 1 + level / 2;
//color.w *= 2;
return color;
}
float get_alpha(float3 pos, float level)
{
return voxels.SampleLevel(linearSampler,pos,level).a;
}


float3 PrefilterEnvMap(float Roughness, float3 R)
{
	return  tex_cube.SampleLevel(linearSampler, R, Roughness*5).rgb;
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
	return     ReflectionColor *(Metallic*SpecularColor * EnvBRDF.x + EnvBRDF.y);
}



float3 get_sky(float3 dir, float level)
{
	return PrefilterEnvMap(level, dir);
}

float4 trace_refl(float4 start_color, float3 view,  float3 origin,float3 dir, float3 normal, float angle)
{
	float orig_angle = angle;
	angle = angle / 8;// pow(angle, 8);
	//return 0;
	float max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14)/1;
//
	//float angle_bad = saturate(abs(angle - max_angle)-0.8);
	//dir = normalize(dir+angle_bad*normal);

	max_angle = saturate((3.14 / 2 - acos(dot(normal, dir))) / 3.14) / 1;

	float best_angle = min(angle, max_angle);


	float angle_error = abs(best_angle - angle);
	//angle = min(angle, 0.34);
	
	//dir=normalize(lerp(dir,normal,angle_error));

	origin = saturate(((origin + 12*scaler*normal*angle_error- (voxel_min)) / voxel_size));
	angle = best_angle;
	float3 samplePos = 0;
    float4 accum = start_color;
    // the starting sample diameter
	float minDiameter = lerp(1.0 / 2048, 1.0 / 512, pow(angle,1/4));// *(1 + 4 * angle);
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

	float3 sky = get_sky( dir, orig_angle);
   float sampleWeight = saturate(max_accum - accum.w)/ max_accum;
   accum.xyz += sky *pow(sampleWeight, 1);// *angle_coeff;
//   accum.w = 1;
   //accum.xyz = sky;
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


float calc_vignette(float2 inTex)
{
	float2 T = saturate(inTex);
	// float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
	float vignette = 4 * (T.y * ((1 - T.y)));
	return vignette;
}

float4 get_ssr(float3 pos, float3 normal, float2 tc,float3 r, float angle, out float dist)
{
	dist = 1;

	float4 ssr = 0;// tex_ssr.SampleLevel(linearSampler, tc, 0);
	return ssr;// +float4(get_sky(r, angle), 1)*(1 - ssr.w);
	
}

[earlydepthstencil]
GI_RESULT PS(quad_output i)
{

	GI_RESULT result;
	
	result.screen =0;
	//result.gi = 0;
	//return result;
   float2 dims;
   gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);
   int2 tc = i.tc*dims;
   
	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz*2-1);

	float4 albedo = gbuffer.GetAlbedo()[tc];


	//return  float4(specular.xyz, 1);
    float3 index = ((pos - (voxel_min)) / voxel_size);
  
  float3 v = normalize(pos-camera.GetPosition());
  float3 r = normalize(reflect(v,normal));



 float  roughness = pow(gbuffer.GetNormals()[tc].w,1);
 float metallic = albedo.w;// specular.w;
  //float fresnel = calc_fresnel(1- roughness, normal, v);

 float angle = roughness;
 float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));

 float dist = 0;


 float4 reflection =  trace_refl(0, v, pos + scaler * normal / m, normalize(r), normal, angle);

 float3 screen = get_PBR(metallic * albedo, reflection, normal, v, roughness, metallic);
#ifdef REFLECTION
 result.screen.xyz = 0;

#else
 result.screen.xyz = screen;

#endif
 result.screen.w = 1;
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
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float raw_z = gbuffer.GetDepth()[tc.xy];
	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float4 albedo = gbuffer.GetAlbedo()[tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.GetPosition());
	float3 r = normalize(reflect(v, normal));



	float  roughness = pow(gbuffer.GetNormals()[tc].w, 1);
	float metallic = albedo.w;// specular.w;
							  //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
	float4 ssr = 0;



	float4 downsampled =  tex_downsampled.SampleLevel(linearSampler, i.tc, 0);



	float4 reflection = downsampled;// trace_refl(ssr, v, pos + scaler * normal / m, normalize(r), normal, angle);
//	reflection = float4(get_PBR(1, normal, v, 1),1);
									//ssr = ssr+get_sky(r,1)*(1 - ssr.w);


	float3 screen = get_PBR(metallic * albedo, reflection, normal, v, roughness, metallic);
#ifdef REFLECTION
	result.screen.xyz = reflection;// screen;//;// tex_color[tc] + float4(PBR(0, reflection, albedo, normal, v, 0.2, 0, metallic), 0);

#else
	result.screen.xyz =  0;//;// tex_color[tc] + float4(PBR(0, reflection, albedo, normal, v, 0.2, 0, metallic), 0);

#endif
	result.screen.w = 0;
	return result;

}



[earlydepthstencil]
GI_RESULT PS_low(quad_output i)
{

	GI_RESULT result;

	
	//result.gi = 0;
	//return result;
	float2 dims;
	gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);
	int2 tc = i.tc*dims;

	float raw_z = gbuffer.GetDepth()[tc.xy];
//	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float4 albedo = gbuffer.GetAlbedo()[tc];


	//return  float4(specular.xyz, 1);
	float3 index = ((pos - (voxel_min)) / voxel_size);

	float3 v = normalize(pos - camera.GetPosition());
	float3 r = normalize(reflect(v, normal));

//	float4 downsampled = tex_ray.SampleLevel(linearSampler, i.tc, 0);

	float  roughness = pow(gbuffer.GetNormals()[tc].w, 1);
	float metallic = albedo.w;// specular.w;
							  //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness;
	float m = 1 * max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
	float4 ssr = 0;

	//[branch]
	//if (angle < 0.1)

	
	float3 reflection = trace_refl(0, v, pos + scaler * normal / m, normalize(r), normal, angle);

	result.screen.xyz = 1;
	return result;
	
}
