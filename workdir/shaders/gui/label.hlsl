Texture2D<float> tex0 : register(t0);

struct Material_samplers
{
	SamplerState sam7ler0: register(s0);
};


struct PSIn {
	float4 Position : SV_Position;
	float4 GlyphColor : COLOR;
	float2 TexCoord : TEXCOORD;
	float4 ClipDistance : CLIPDISTANCE;
};

float4 PS(PSIn Input) : SV_Target{

	return 1;
	clip(Input.ClipDistance);

	float a = tex0.Sample(sampler0, Input.TexCoord);

	if (a == 0.0f)
		discard;

	return float4(Input.GlyphColor.xyz, a * Input.GlyphColor.w);
}
;
