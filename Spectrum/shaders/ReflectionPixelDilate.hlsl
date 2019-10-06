
#include "Common.hlsl"
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





cbuffer cbCamera : register(b0)
{
	camera_info camera;
	camera_info prev_camera;
};

cbuffer cbEffect : register(b1)
{
	float time;
};



float3 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return float3(res.xy * float2(0.5, -0.5) + float2(0.5, 0.5), res.z);
}

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer_old : register(t3);
Texture2D<float4> dist_buffer : register(t4);
Texture2D<float4> color_buffer : register(t5);
Texture2D<float4> color_buffer_prev : register(t6);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);
Texture2D<float2> speed_tex : register(t0, space2);
Texture2D<float4> depth_buffer: register(t7);
float4 PS(quad_output i) : SV_TARGET0
{
//	return 1;

	
	float2 dims;
dist_buffer.GetDimensions(dims.x, dims.y);

int2 tc = i.tc*dims;

float raw_z = depth_buffer[tc.xy];
if (raw_z == 1) return 0;

//float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0);
//float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);



pixel_info info;
info.albedo = 0;// gbuffer[0][tc.xy];
info.normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).rgb * 2 - 1);
info.metallic = gbuffer[0].SampleLevel(PixelSampler, i.tc, 0).w;
// info.specular = gbuffer[2][tc.xy].xyz;
info.roughness =  max(0.08, gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).w);// gbuffer[2][tc.xy].w;


info.pos = depth_to_wpos(raw_z, float2(i.tc.xy), camera.inv_view_proj);
info.view_z = camera.proj._34 * raw_z / (raw_z - camera.proj._33);
info.view = normalize(camera.position - info.pos);
info.reflection = reflect(-info.view, info.normal);

float  _BRDFBias = 0.0;
float NdotV = dot(info.normal, info.view);
float coneTangent = lerp(0, info.roughness * (1 - _BRDFBias), NdotV * sqrt(info.roughness));

float4 val = dist_buffer.SampleLevel(PixelSampler, i.tc + 0.5 / dims, 0);
//return float4(color_buffer.SampleLevel(PixelSampler, val.xy + 0.5 / dims, 0).xyz* (val.z > 0), val.z > 0);
//return val.zzzz>0;
float4 result=0;
float KSum = 0.5;
int samples_count = 0;
float zsum = 0;

float2 avg_tc = 0;
float avg_dist = 1;
#define R 0
for(int x=-R;x<=R;x++)
	for (int y = -R; y<= R; y++)
	{

	//	if (x == 0 && y == 0) continue;

		float p_raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0, int2(x, y));
		float3 p_pos = depth_to_wpos(p_raw_z, i.tc + float2(x, y) / dims, camera.inv_view_proj);



		float4 val = dist_buffer.SampleLevel(PixelSampler, i.tc+ 0.5 / dims, 0,int2(x,y));
	
		bool good =  val.z > 0;
		if (!good) continue;
		float PDF =   max(1e-5,val.w);
		float rp_raw_z = depth_buffer.SampleLevel(PixelSampler, val.xy,0);
		float3 rp_pos = depth_to_wpos(rp_raw_z, val.xy, camera.inv_view_proj);
	    float3 rp_normal = normalize(gbuffer[1].SampleLevel(PixelSampler, val.xy, 0).rgb * 2 - 1);

		float3 delta_pos = normalize(rp_pos- info.pos);


		float intersectionCircleRadius = coneTangent * length(int2(x, y)/dims);
		float mip =  clamp(log2(intersectionCircleRadius* max(dims.x, dims.y)), 0, 8);



		float3 color = saturate(dot(info.reflection, rp_normal)<0) * good*color_buffer.SampleLevel(LinearSampler, val.xy, 0);

		float w = good*(BRDF_UE4(info.view, delta_pos, info.normal, info.roughness));
		result.xyz += color * w;
			KSum += w;
			zsum += good;
			avg_tc += good * val.xy;
			avg_dist += val.z;
		samples_count++;
	//	result.xyz += 

	}


avg_tc /= KSum;
avg_dist /= zsum;

float total_w = saturate(zsum);

//avg_dist = saturate(avg_dist+total_w);
//avg_dist = val.z;

//return float4(avg_dist.xxx, 1);
float3 reflected_pos = info.pos - normalize(info.view) * avg_dist;// depth_to_wpos(avg_dist, i.tc, camera.inv_view_proj);


//	return float4(reflected_pos.yyy, 1);
float2 reflected_pos_prev =  project_tc(reflected_pos, prev_camera.view_proj);


float max_w= BRDF_UE4(info.view, info.reflection, info.normal, info.roughness);
//	float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).rgb * 2 - 1);
	//float4 specular = gbuffer[2].SampleLevel(PixelSampler, i.tc, 0);

float4 cur = float4(result.xyz / KSum, total_w);



//float2 delta = speed_tex.SampleLevel(PixelSampler, avg_tc, 0).xy;

float2 prev_tc =  reflected_pos_prev;// -delta * 0.5;
float4 prev_gi = color_buffer_prev.SampleLevel(LinearSampler, prev_tc, 0);
//float prev_z = tex_depth_prev.SampleLevel(LinearSampler, prev_tc, 0);



//float3 prev_pos = depth_to_wpos(prev_z, prev_tc, prev_camera.inv_view_proj);

float l = 0;// length(pos - prev_pos)* MOVE_SCALER;
return float4(lerp(cur, prev_gi, saturate(0.9 - l)).xyzw);


}


