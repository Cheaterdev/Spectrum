#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_VS
cbuffer cbParams : register(b0)
{
	float4 pos;
};

quad_output VS(uint index : SV_VERTEXID)
{
	static float2 Pos[] =
	{
		float2(pos.x, pos.w),
		float2(pos.x, pos.y),
		float2(pos.z, pos.w),
		float2(pos.z, pos.y)
		/*
		float2(-1, 1),
		float2(1, 1),
		float2(-1, -1),
		float2(1, -1),*/
	};
	static float2 Tex[] =
	{


		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),
	};
	quad_output Output;
	Output.pos = float4(Pos[index].x * 2 - 1, -(Pos[index].y * 2 - 1), 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
	Output.tc = Tex[index];
	return Output;
}
#endif

#ifdef BUILD_FUNC_PS
cbuffer cbParams : register(b0)
{
	float4 color;
	float4 size;
	float4 clip_size;
	float scale;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register (s0);

float get_arrow(float2 tc, float size)
{
	float c = 2.0 / size;
	float a = -4.0 / size;
	return smoothstep(a, a + c, tc.x + tc.y - 1)*smoothstep(a, a + c, -tc.x + tc.y) *smoothstep(a, a + c, 0.5 - tc.y);
}
float4 PS(quad_output i) : SV_TARGET0
{

	float2 screen_tc = i.pos.xy / i.pos.z;

	clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

	float corners = 8 * scale;

	float2 best_point = clamp(i.tc * size.xy, corners, size.xy - corners);
		float len = clamp(length(best_point - i.tc * size.xy) / (corners), 0, 1); // size.z;
	//float stage = i.tc.x > 1 - size.y / size.x;
	//return float4(stage.xxx, 1);
	float t2 = 0.1 / scale;
	float alpha = 1 - smoothstep(0.6 - t2, 0.6 + t2, len);

	//float alpha = 1 - smoothstep(0.60, 0.60+0.17/scale, len);

	float t = 0.1 / scale;
	float alpha2 = 1 - smoothstep(0.35-t, 0.35+t, len);
	float shadow = 1 - smoothstep(0, 1, len);


	float2 shadow_mask = min(i.tc * size.xy, corners.xx * 15 * scale) / corners.x / (15 * scale);

		float res_alpha = alpha - color.a*alpha2;
	
	
	return float4(color.xyz,res_alpha);
	

}
#endif