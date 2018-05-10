#define M_PI 3.141592653589793238462643383279f
struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_VS

quad_output VS(uint index : SV_VERTEXID)
{
	static float2 Pos[] =
	{
		/*float2(pos.x, pos.w),
		float2(pos.x, pos.y),
		float2(pos.z, pos.w),
		float2(pos.z, pos.y)*/
		
		float2(-1, 1),
		float2(-1, -1),
		float2(1, 1),
		float2(1, -1)
	};
	static float2 Tex[] =
	{


		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),
	};
	quad_output Output;
	Output.pos = float4(Pos[index].x , -Pos[index].y, 0.99999, 1); //float4(Input.Pos.xy,0.3,1);
	Output.tc = Tex[index];
	return Output;
}
#endif

#ifdef BUILD_FUNC_PS
Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register (s0);


	float4 PS(quad_output input) : SV_TARGET0
	{
	
	float orig = tex.Sample(PointSampler, input.tc);
	
	
float2 res = 0;
for(int i=-1;i<=1;i++)
for(int j=-1;j<=1;j++)
{
float w = 1-length(float2(i,j))/length(float2(2,2));
res+=float2(tex.Sample(PointSampler, input.tc ,int2(i,j)).x*w,w);

}


		return float4(2*saturate((res.x/res.y-orig*0.99).xxx)*float3(1,0.5,0),1);

	}

#endif