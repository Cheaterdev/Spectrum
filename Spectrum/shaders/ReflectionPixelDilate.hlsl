

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

float3 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return float3(res.xy * float2(0.5, -0.5) + float2(0.5, 0.5), res.z);
}

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture2D<float4> dist_buffer : register(t4);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);


float4 PS(quad_output i) : SV_TARGET0
{


	float2 dims;
dist_buffer.GetDimensions(dims.x, dims.y);

float4 cur_val = dist_buffer.SampleLevel(PixelSampler, i.tc, 0);
float3 my_pos = depth_to_wpos(cur_val.z, float2(i.tc), camera.inv_view_proj);

float result = cur_val.x;
float max_value = cur_val.y;
float goodness = cur_val.w;

#define R 6

float counter = 1;
for(int x=-R;x<=R;x++)
	for (int y = -R; y<= R; y++)
	{

		if (x == 0 && y == 0) continue;
		float4 val = dist_buffer.SampleLevel(PixelSampler, i.tc, 0,int2(x,y));
		float3 pos = depth_to_wpos(val.z, float2(i.tc+float2(x,y)/dims), camera.inv_view_proj);

		
		if (val.y > cur_val.z&&length(pos-my_pos)<0.3)
		//if(result>val.x&& val.w>goodness)
		{

		//goodness= max(goodness, val.w);
	//	counter++;
	//	result = min(result, val.x);


		}
		
		//if()


	//	else
		//	result = min(result, val.x);

	}
	/*
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

	float4 albedo = gbuffer[0].SampleLevel(PixelSampler, tc, 0);

	return float4(dist.xxx / 4,1);// float4(/*normal.w*albedo*vignette*albedo*dist.w);*/


	return float4(result.xxx, goodness) ;
}


