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
    float4 color1;
    float4 color2;
	float4 color3;
	float4 color4;
    float4 size;
    float4 clip_size;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register (s0);

float get_arrow(float2 tc, float size)
{
	float c = 2.0 / size;
	float a = -4.0 / size;
	return smoothstep(a,a+ c, tc.x + tc.y - 1)*smoothstep(a,a+ c, -tc.x + tc.y) *smoothstep(a, a+c, 0.5-tc.y );
}
float4 PS(quad_output i) : SV_TARGET0
{

    float2 screen_tc = i.pos.xy / i.pos.z;

    clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

    float corners = 5;

    float2 best_point = clamp(i.tc * size.xy, corners, size.xy - corners);
    float len = clamp(length(best_point - i.tc * size.xy) / (corners), 0, 1); // size.z;
    //float stage = i.tc.x > 1 - size.y / size.x;
    //return float4(stage.xxx, 1);
    float alpha = 1 - smoothstep(0.60, 0.77, len);
    float shadow = 1 - smoothstep(0, 1, len);

  
    float2 shadow_mask = min(i.tc * size.xy, corners.xx * 15) / corners.x / 15;



    float4 ret_color;

	float divider = 1 - size.y / size.x;
	if (i.tc.x < divider)
	{
		float3 result = lerp(color3.xyz, color4.xyz, i.tc.y);
			result = lerp(0, result, alpha);
		result*= 0.5;// *dot(result, 1.0 / 3.0);

		float shadow2 = 1 - (1 - saturate(-(i.tc.x - (divider))*size.x / 2))*(0.5 - 0.25*color1.a);
//		shadow2 = shadow2*0.3 + 0.7;

		//return float4(shadow2.xxx,1);
		ret_color = float4(shadow2*result, alpha + shadow);
    }
    else
	{
		float3 result = lerp(color1.xyz, color2.xyz, i.tc.y);
		result = lerp(0, result, alpha);
		float shadow2 = 1 - (1 - min(shadow_mask.x, shadow_mask.y)) * color1.a * smoothstep(0.3, 0.9, len);
		//return float4(shadow2.xxx, 1);
        ret_color = float4(shadow2.x * result, alpha + (1 - color1.a) * shadow);
		float2 cor_tc;
		cor_tc.y = lerp(1 - i.tc.y, i.tc.y, color1.a);
		cor_tc.x = (i.tc.x - divider) / (1 - divider-3/size.x);

		float arrow = get_arrow(cor_tc, size.y);
		ret_color = lerp(ret_color, lerp(arrow, 0.8*arrow, color1.a), arrow);
    }

	//float delim = smoothstep(0.5 / size.x, 2.0 / size.x, abs(i.tc.x - (1 - size.y / size.x)));
	//return float4(delim.xxx,1);
//	ret_color.xyz = lerp(0.3, ret_color.xyz, delim);

	return  ret_color;

}
#endif