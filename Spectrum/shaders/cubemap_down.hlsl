
struct quad_output
{
float4 pos : SV_POSITION;
float3 tc: TEXCOORD0;
};
cbuffer p: register(b0)
{
    uint face_index;
    uint mip;
};
#ifdef BUILD_FUNC_VS


static float2 Pos[] =
{
    float2(-1, -1),
    float2(-1, 1),
    float2(1, -1),
    float2(1, 1)
};



static float3x3 mats[] =
{
    float3x3(0, 0, 1,   0, 1, 0,   -1, 0, 0), //X+
    float3x3(0, 0, -1,  0, 1, 0,   1, 0, 0), //X-

    float3x3(1, 0, 0,   0, 0, 1,   0, -1, 0), //Y+
    float3x3(1, 0, 0,   0, 0, -1,  0, 1, 0), //Y-

    float3x3(1, 0, 0,   0, 1, 0,   0, 0, 1), //Z+
    float3x3(-1, 0, 0,   0, 1, 0,   0, 0, -1) //Z-
};

quad_output VS(uint index : SV_VERTEXID)
{
    quad_output Output;
    Output.pos = float4(Pos[index], 0.99999, 1);
    Output.tc = normalize(mul(mats[face_index], float3(Pos[index], 1)));
    return Output;
}
#endif

#ifdef BUILD_FUNC_PS
TextureCube tex_cube: register(t0);
SamplerState LinearSampler: register(s0);
//SamplerState PixelSampler : register(s1);

float rnd(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}
static const float PI = 3.14159265358979f;

float3x3 matrixFromVector(float3 n)   // frisvad
{
    float a = 1.0f / (1.0f + n.z);
    float b = -n.x * n.y * a;
    float3 b1 = float3(1.0 - n.x * n.x * a, b, -n.x);
    float3 b2 = float3(b, 1.0 - n.y * n.y * a, -n.y);
    return float3x3(b1, b2, n);
}



float3 hemisphereSample_cos(float2 uv, float3x3 vecSpace, float3 cubeDir, float gloss)   // cos + lerped cone size (better than just lerped)
{
    float phi = uv.y * 2.0 * PI;
    float cosTheta = sqrt(1.0 - uv.x);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 sampleDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    return normalize(lerp(mul(sampleDir, vecSpace), cubeDir, gloss));
}
float3 hemisphereSample_phong(float2 uv, float3x3 vecSpace, float3 cubeDir, float specPow)
{
    float phi = uv.y * 2.0 * PI;
    float cosTheta = pow(1.0 - uv.x, 1.0 / (specPow + 1.0));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 sampleDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    return mul(sampleDir, vecSpace);
}


float3 textureAVG( float3 tc) {
	const float diff0 = 0.035;
	const float diff1 = 0.012;
	float3 s0 = tex_cube.SampleLevel(LinearSampler, tc,0).xyz;
	float3 s1 = tex_cube.SampleLevel(LinearSampler, tc + float3(diff0.xxx),0).xyz;
	float3 s2 = tex_cube.SampleLevel(LinearSampler, tc + float3(-diff0.xxx),0).xyz;
	float3 s3 = tex_cube.SampleLevel(LinearSampler, tc + float3(-diff0, diff0, -diff0),0).xyz;
	float3 s4 = tex_cube.SampleLevel(LinearSampler,tc + float3(diff0, -diff0, diff0),0).xyz;

	float3 s5 = tex_cube.SampleLevel(LinearSampler, tc + float3(diff1.xxx),0).xyz;
	float3 s6 = tex_cube.SampleLevel(LinearSampler, tc + float3(-diff1.xxx),0).xyz;
	float3 s7 = tex_cube.SampleLevel(LinearSampler, tc + float3(-diff1, diff1, -diff1),0).xyz;
	float3 s8 = tex_cube.SampleLevel(LinearSampler, tc + float3(diff1, -diff1, diff1),0).xyz;

	return (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) * 0.111111111;
}
float somestep(float t) {
	return pow(t, 4.0);
}

float3 textureBlured( float3 tc) {
	float3 r = textureAVG( float3(1.0, 0.0, 0.0));
	float3 t = textureAVG( float3(0.0, 1.0, 0.0));
	float3 f = textureAVG( float3(0.0, 0.0, 1.0));
	float3 l = textureAVG( float3(-1.0, 0.0, 0.0));
	float3 b = textureAVG( float3(0.0, -1.0, 0.0));
	float3 a = textureAVG( float3(0.0, 0.0, -1.0));

	float kr = dot(tc, float3(1.0, 0.0, 0.0)) * 0.5 + 0.5;
	float kt = dot(tc, float3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
	float kf = dot(tc, float3(0.0, 0.0, 1.0)) * 0.5 + 0.5;
	float kl = 1.0 - kr;
	float kb = 1.0 - kt;
	float ka = 1.0 - kf;

	kr = somestep(kr);
	kt = somestep(kt);
	kf = somestep(kf);
	kl = somestep(kl);
	kb = somestep(kb);
	ka = somestep(ka);

	float d;
	float3 ret;
	ret = f * kf; d = kf;
	ret += a * ka; d += ka;
	ret += l * kl; d += kl;
	ret += r * kr; d += kr;
	ret += t * kt; d += kt;
	ret += b * kb; d += kb;

	return ret / d;
}

/*

float4 PS(quad_output i) : SV_TARGET0
{

    float3 itc = normalize(i.tc);

	// float4(textureAVG(itc),1);



    float3x3 vecSpace = matrixFromVector(itc);

    float3 result = 0;
    static const int samples = 64;

    for (int i = 0; i < samples; i++)
    {
        float sini = sin(float(i));
        float cosi = cos(float(i));
        float rand = rnd(float2(sini, cosi));
     //   float3 t = hemisphereSample_cos(float2(float(i) / float(samples), rand), vecSpace, itc,0.7);

		float3 t = hemisphereSample_phong(float2(float(i) / float(samples), rand), vecSpace, itc, 512);
        result += tex_cube.SampleLevel(LinearSampler, t, 0);
    }


    return float4(result / samples, 1);

}

*/
float4 PS(quad_output i) : SV_TARGET0
{
	float2 dims;
tex_cube.GetDimensions(dims.x, dims.y);
	float3 itc = normalize(i.tc);

	float3 result = 0;
	int samples = 0;

	static const int R =8;


	for(int x=-R;x<=R;x++)
		for (int y = -R; y <= R; y++)
			for (int z = -R; z <= R; z++)
			{
				result+= tex_cube.SampleLevel(LinearSampler, itc +float3(x,y,z)/ dims.x*pow(1.57,mip-1), 0);
				samples++;
			}
	return float4(result / samples, 1);

}
#endif