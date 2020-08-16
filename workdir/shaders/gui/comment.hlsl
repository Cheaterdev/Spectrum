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
	float4 color1;
	float4 color2;
	float4 color_back;
	float4 sh_color;
	float4 size;
	float4 clip_size;
	float4 properties;
};

Texture2D<float4> tex : register(t0);

SamplerState PointSampler : register (s0);

float4 PS(quad_output i) : SV_TARGET0
{
	float2 screen_tc = i.pos.xy / i.pos.z;

	clip(float4(screen_tc.xy, -screen_tc.xy) - clip_size);

	float scale = properties.z / 10;
	float round = properties.z;
	float corners = properties.x;
	float header_size = properties.y;// +round * 1;
	float corners_round = corners + round;

	float2 best_point = clamp(i.tc * size.xy, corners_round.xx, size.xy - corners_round.xx);
		float len = clamp(length(best_point - i.tc * (size.xy)) / corners, 0, 1); // size.z;


	float2 corrected_tc = len < 1 ? (best_point - float2(corners_round.xx)) / (size.xy - 2 * corners_round.xx) : 0;
		//	return float4(len.xxx, 1);

		float t = 0.05 / scale;
	float alpha = 1 - smoothstep(0.5 - t, 0.5 + t, len);
	float orig_a = alpha;
	t = (sh_color.w * 0.3); // / scale;
	float c = 0.5;// -0.4*sh_color.w;
	t = 0.1;
	float alpha2 = 1 - smoothstep(t / 2 - t / 2 / scale, t / 2 + t / 2 / scale, abs(len - c));

	c = 0.44;// -0.4*sh_color.w;
	t = 0.0135;
	float alpha_c = 1 - 0.5*smoothstep(c - t, c + t, len);

	//	return float4(alpha2.xxx, 1);
	//float alpha = 1 - smoothstep(round / (20 * scale), round / (20 * scale) + 0.03*scale, len);
	float shadow = 1 - smoothstep(0, 1, len);

	float stage = corrected_tc.y / (header_size - round) * (size.y - corners_round * 2);

	//float3 result = stage < 1 ? lerp(color2, lerp(color2, color_back,0.9f), saturate(length(float2(3, 4)*(i.tc - float2(0.5, 0))))) : color_back;
	//	float3 result = stage < 1 ? lerp(color2, lerp(color2, color_back, 0.9f), saturate(length(float2(3, 8)*(i.tc - float2(0.3, 0.2))))) : color_back;
	float3 result = 1;

	if (stage > 1)
	{
		result = color_back;
		alpha *= 0.7;
	}
	else
	{
		float l = length(float2(3, 2) * (float2(i.tc.x - 40 * scale / size.x, i.tc.y* size.y / header_size - (header_size / 2 + corners) / header_size)));
		float a = 1 / (0.5 + l * l);
		result = lerp(color_back, color2, a);
		alpha *= (saturate(a)) * 0.2 + 0.8;
	}
	float shadow2 = stage < 1 ? 1 : (0.5 + 0.5 * saturate((stage - 1) * 5));
	//	shadow2 = saturate(shadow2*shadow2 + 0.3*abs(corrected_tc.x-0.5)*2);

	float4 window_color = float4(shadow2.xxx * result, alpha);
		float4 shadow_color = float4(0, 0, 0, shadow.x / 4);

		float4 res = shadow_color * (1 - window_color.a) + window_color * window_color.a;
		float4 back = float4(sh_color.xyz, alpha2 * sh_color.a);
		res.xyz = lerp(alpha_c*res.xyz, back.xyz, back.a);
	res.a += back.a;
	//res += float4(sh_color.xyz, 0)*(alpha2 - orig_a);
	//res = float4(alpha_c.xxx,1);
	return res;// lerp(float4(0, 0, 0, shadow.x / 4), float4(shadow2.xxx*result, 1), alpha);

}
#endif