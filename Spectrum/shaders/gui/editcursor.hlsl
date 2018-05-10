#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc: TEXCOORD0;
};

#if BUILD_FUNC == VS
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

#if BUILD_FUNC == PS
cbuffer cbParams : register(b0)
{

	float4 size;
	float4 clip_size;
	float stage;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register (s0);

float4 PS(quad_output i) : SV_TARGET0
{

	float2 screen_tc = i.pos.xy / i.pos.z;

	clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);
	//return float4(0,0,0,1);
	
	float shadow = 1-smoothstep(0.0,1-stage,abs(i.tc.x-0.5)*2);
	return float4(0, 0, 0, shadow-stage );

}
#endif