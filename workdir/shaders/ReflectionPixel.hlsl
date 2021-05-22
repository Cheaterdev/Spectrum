
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
Texture2D<float4> light_buffer: register(t5);

Texture2D<float4> depth_buffer: register(t7);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);



float calc_vignette(float2 inTex)
{
	float2 T = saturate(inTex);
	// float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
	float vignette = 4 * (T.y * ((1 - T.y)));
	return vignette;
}

float2 cell(float2 ray, float2 cell_count, uint camera) {
	return floor(ray.xy * cell_count);
}

float2 cell_count(float2 input_texture2_size, float level) {
	return input_texture2_size / (level == 0.0 ? 1.0 : exp2(level));
}

float3 intersect_cell_boundary(float3 pos, float3 dir, float2 cell_id, float2 cell_count, float2 cross_step, float2 cross_offset, uint camera) {
	float2 cell_size = 1.0 / cell_count;
	float2 planes = cell_id / cell_count + cell_size * cross_step;

	float2 solutions = (planes - pos) / dir.xy;
	float3 intersection_pos = pos + dir * min(solutions.x, solutions.y);

	intersection_pos.xy += (solutions.x < solutions.y) ? float2(cross_offset.x, 0.0) : float2(0.0, cross_offset.y);

	return intersection_pos;
}

bool crossed_cell_boundary(float2 cell_id_one, float2 cell_id_two) {
	return (int)cell_id_one.x != (int)cell_id_two.x || (int)cell_id_one.y != (int)cell_id_two.y;
}

float minimum_depth_plane(float2 ray, float level, float2 cell_count, uint _camera) {

	float raw_z = depth_buffer.Load(int3(ray.xy * cell_count, level)).r;
	return raw_z;
	float3 pos = depth_to_wpos(raw_z, ray.xy, camera.inv_proj);

	return pos.z;
}
#define HIZ_START_LEVEL 0
#define HIZ_STOP_LEVEL 0
#define MAX_ITERATIONS 32
#define HIZ_MAX_LEVEL 8
#define depth_threshold 0.1
float3 hi_z_trace(float3 p, float3 v, out uint iterations) {
	float level = HIZ_START_LEVEL;
	float3 v_z = v / v.z;
	float2 dims;
	uint camera = 0;
	depth_buffer.GetDimensions(dims.x, dims.y);

	float2 hi_z_size = cell_count(dims, level);
	float3 ray = p;

	float2 cross_step = float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
	float2 cross_offset = cross_step * 0.001;
	cross_step = saturate(cross_step);

	float2 ray_cell = cell(ray.xy, hi_z_size.xy, camera);
	ray = intersect_cell_boundary(ray, v, ray_cell, hi_z_size, cross_step, cross_offset, camera);

	iterations = 0;
	while (level >= HIZ_STOP_LEVEL && iterations < MAX_ITERATIONS&&all(ray<1) && all(ray >0)) {
		// get the cell number of the current ray
		float2 current_cell_count = cell_count(dims, level);
		float2 old_cell_id = cell(ray.xy, current_cell_count, camera);

		// get the minimum depth plane in which the current ray resides
		float min_z = minimum_depth_plane(ray.xy, level, current_cell_count, camera);

		// intersect only if ray depth is below the minimum depth plane
		float3 tmp_ray = ray;
		if (v.z > 0) {
			float min_minus_ray = min_z - ray.z;
			tmp_ray = min_minus_ray > 0 ? ray + v_z * min_minus_ray : tmp_ray;
			float2 new_cell_id = cell(tmp_ray.xy, current_cell_count, camera);
			if (crossed_cell_boundary(old_cell_id, new_cell_id)) {
				tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset, camera);
				level = min(HIZ_MAX_LEVEL, level + 2.0f);
			}
			else {
				if (level == HIZ_START_LEVEL && min_minus_ray > depth_threshold) {
					tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset, camera);
					level = HIZ_START_LEVEL + 1;
				}
			}
		}
		else if (ray.z < min_z) {
			tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset, camera);
			level = min(HIZ_MAX_LEVEL, level + 2.0f);
		}

		ray.xyz = tmp_ray.xyz;
		--level;

		++iterations;
	}
	return ray;
}

float4 PS(quad_output i) : SV_TARGET0
{
	float2 dims;
depth_buffer.GetDimensions(dims.x, dims.y);
	float roughness = gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).w;
	float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc,0);
	if (raw_z == 1) return 0;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_proj);
//	float3 wpos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);

	float3 normal = mul((float3x3)camera.view, normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).xyz * 2 - 1));

	float3 V = -normalize(pos);
	//return float4(V, 1);

	float sini = sin(time * 220 + float(i.tc.x));
	float cosi = cos(time * 220 + float(i.tc.y));
	float rand = rnd(float2(sini, cosi));
	float rand2 = rnd(float2(cosi, sini));

	float rcos = cos(6.14*rand);
	float rsin = sin(6.14*rand);

	float2 Xi = hammersley2d(rand * 1000, 1000);
	float PDF;
	float3 H = ImportanceSampleGGXPdf(Xi, roughness, normal, PDF);
	float3 L = 2 * dot(V, H) * H - V;
	float3 r = L;// reflect(v, normal);

	if (dot(r, normal) < 0)
		r = normalize(r+normal);
//	pos -= V * 0.3;
//	float3 r = normalize(reflect(-V, normal));
//	return float4(pos.zzz/30,1);

	//return float4(r, 1);

	float3 posr = pos +normalize(r);
	float3 pss = float3(i.tc, raw_z);// pos.z);
	float4 pcs = mul(camera.proj,float4(posr,1));
	float3 pss_ = pcs.xyz / pcs.w;
	pss_.xy = pss_.xy * float2(0.5, -0.5) + float2(0.5, 0.5); 
	
	//pss_.z = posr.z;
	uint iters = 0;
	float3 ftc = hi_z_trace(pss, (pss_- pss), iters);



	{
		float raw_z2 = depth_buffer.SampleLevel(PixelSampler, ftc, 0);
		 if (raw_z2 == 1) return 0;
		float3 rpos = depth_to_wpos(raw_z2, ftc, camera.inv_proj);

		if (any(ftc.x > 1)) return 0;
		if (any(ftc.x <0)) return 0;
		float3 delta = (rpos - pos);

		float dist = length(delta);
		bool good = (dot(delta / dist, normalize(r)) > 0.9);// *(saturate(1 - dist / 5));
	//	if (!good) return 0;
		return float4(ftc.xy, good? dist :0, PDF);
	
	}
	//float2 ftc = project_tc(ray, camera.proj);

}
	//return float4(minimum_depth_plane(i.tc,3, cell_count(dims, 3),3).xxx,1);




float4 PS_LAST(quad_output i) : SV_TARGET0
{

	float4 dist = dist_buffer.SampleLevel(LinearSampler, i.tc, 0);


	float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).rgb * 2 - 1);
	float4 specular = gbuffer[2].SampleLevel(PixelSampler, i.tc, 0);


	float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0);
	//	if (raw_z == 1) return 0;
		float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);



		float3 v = -normalize(camera.position - pos);
		float len = length(camera.position - pos);

		float3 r = reflect(v, normal);



		float2 tc = project_tc(pos + dist.x * r, camera.view_proj);

		float vignette = calc_vignette(tc);

		float4 albedo = light_buffer.SampleLevel(LinearSampler,tc,clamp(dist.x,0,8));


		float w = 1;// saturate(1 - dist.x / 8);

		//return float4(dist.xxx/18, 1);
		return  float4(normal, 1);// w*vignette*albedo.xyz*dist.w, 0.1);
}


