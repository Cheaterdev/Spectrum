

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




struct camera_info
{
	matrix view;
	matrix proj;
	matrix view_proj;
	matrix inv_view;
	matrix inv_proj;
	matrix inv_view_proj;
	float3 position;
	float3 direction;
};


cbuffer cbCamera : register(b0)
{
	camera_info camera;
};

cbuffer cbEffect : register(b1)
{
	float time;
};


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc, d, 1));
	return P.xyz / P.w;
}

float2 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return res.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
}

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture2D<float4> light_buffer : register(t4);
Texture2D<float4> prev_ssgi_buffer : register(t5);
Texture2D<float2> speed_tex : register(t6);

TextureCubeArray<float4> cube_probe : register(t7);
//Texture2D<float> ambient_texture : register(t6);

//RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);



float4 get_sky(float3 dir, float level)
{

	//return 0;
	level *= 64;
	//return tex_cube.Sample(LinearSampler, float4(dir,6));
	int l0 = level;
	int l1 = l0 + 1;


	return lerp(cube_probe.Sample(LinearSampler, float4(dir, l0)), cube_probe.Sample(LinearSampler, float4(dir, l1)), level - l0);


}




float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = saturate(dot(n, -v));
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float calc_vignette(float2 inTex)
{
	float2 T = saturate(inTex);
	// float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
	float vignette = 4 * (T.y * ((1 - T.y)));
	return vignette;
}
float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

static const float2 poisson[16] = {
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870),
	float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	float2(-0.81544232, -0.87912464),
	float2(-0.38277543, 0.27676845),
	float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188),
	float2(-0.24188840, 0.99706507),
	float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367),
	float2(0.14383161, -0.14100790)

};


float4 get_direction(float3 pos, float3 normal, float2 itc, float2 dir)
{

#define MAX_DIST 1
#define COUNT 128

	float2 direction = MAX_DIST*dir / COUNT;


	float dist = length(pos - camera.position);

	float4 res=0;


	for (int j =1; j < COUNT; j++)
	{





		//float2 delta_tc = poisson[16*rand*j]/16;// float2(rcos*poisson[j].x - rsin*poisson[j].y, rsin*poisson[j].x + rcos*poisson[j].y);
	

		float2 tc = itc + j*direction;// rand*j*poisson[j] / 120.0f;


		float level = 0;// 1 + 2 * float(j) / COUNT;// 3 * float(j) / 16;// length(poisson[j] * 2);// *float(j) / 16;
		float4 color =light_buffer.SampleLevel(LinearSampler, tc, level);
		float z = depth_buffer.SampleLevel(LinearSampler, tc, level).x;
		float3 t_pos = depth_to_wpos(z, float2(tc.xy), camera.inv_view_proj);

		float3 n = normalize(gbuffer[1].SampleLevel(LinearSampler, tc, level).xyz * 2 - 1);


		float len = length(pos - t_pos);

		float3 delta = (t_pos-pos)/ len;
		float s =pow(saturate(dot(normal, delta)), 2)/max(0.01,len);

		s = saturate(s);
		float w = 1/ (len*len / dist + 1);
		w*=saturate(dot(-n, delta) * 2);
		w*=saturate(dot(normal, delta)*2);
		
		color.xyz *= w/len;
		color.w = s;
		//color.w = saturate(color.w * 4);

		//color.xyz *= color.w;

		float sampleWeight = saturate(1 - res.w);
		res += color * sampleWeight;
	}

	float4 sky = get_sky(normal, 1);

	float sampleWeight = saturate(1 - res.w);
	res += sky * sampleWeight;

	//return 1;
	return float4(res.xyzw);

}
float4 PS(quad_output i) : SV_TARGET0
{
//	discard;
	return  0;// gbuffer[0].SampleLevel(PixelSampler, i.tc, 0);
	float2 dims;
gbuffer[0].GetDimensions(dims.x, dims.y);
//i.tc-=float2(0.5,0.5)/dims;

//float4 albedo = gbuffer[0].SampleLevel(PixelSampler, i.tc, 1);
float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).xyz * 2 - 1);
float4 specular = 1;// gbuffer[2].SampleLevel(PixelSampler, i.tc, 1);
float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0);
float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);

float sini = sin(time * 220 + float(i.tc.x));
float cosi = cos(time * 220 + float(i.tc.y));
float rand = rnd(float2(sini, cosi)) ;
float rand2 = 0.5+0.5*rnd(float2(cosi, sini));

float4 res = 0;

	float rcos = cos(6.14*rand);
	float rsin = sin(6.14*rand);
	res= get_direction(pos, normal, i.tc, rand2*float2(rcos, rsin));


//	

//	float2 delta = speed_tex.SampleLevel(PixelSampler, i.tc, 0).xy;

	float4 prev = prev_ssgi_buffer.SampleLevel(PixelSampler, i.tc, 0);

	return lerp(res, prev, 0);// float4(res.xyz, 1);
}



float4 PS_LAST(quad_output i) : SV_TARGET0
{

	//float4 back = light_buffer.SampleLevel(LinearSampler, i.tc, 0);
	float4 albedo = gbuffer[0].SampleLevel(PixelSampler, i.tc, 0);
	float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc,0).xyz * 2 - 1);
	float4 specular = gbuffer[2].SampleLevel(PixelSampler, i.tc, 0);



	float raw_z = depth_buffer.SampleLevel(LinearSampler, i.tc, 0);
//	if (raw_z >= 1) return result;
	float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);
	float3 v = normalize(pos - camera.position);
	float3 r = normalize(reflect(v, normal));

	float4 sky = get_sky(normal, 1);


	float  roughness = gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).w;
	float metallic = albedo.w;// specular.w;
							  //float fresnel = calc_fresnel(1- roughness, normal, v);

	float angle = roughness / 1.57f;



	float4 reflection = get_sky(r, angle);

	//return float4(reflection.xyz,1);
	return  float4(sky.xyz*albedo*(1- metallic)+ reflection*albedo*metallic,1);// float4(back.xyz, 1);
}


