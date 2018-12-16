
struct quad_output
{
float4 pos : SV_POSITION;
float3 tc: TEXCOORD0;
};
cbuffer p: register(b0)
{
    uint face_index : packoffset(c0);
    float roughness : packoffset(c0.y);
	uint EnvMapSize : packoffset(c0.z);
//const	uint NumSamples : packoffset(c0.w);
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

TextureCube tex_cube: register(t0);
SamplerState LinearSampler: register(s0);
//SamplerState PixelSampler : register(s1);

#include "Common.hlsl"

#ifdef BUILD_FUNC_PS
float3 PrefilterEnvMap(float Roughness, float3 R)
{
		float3 N = R;
		float3 V = R;
		float3 PrefilteredColor = 0;
		float TotalWeight = 0.0;

		float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 TangentX = normalize(cross(UpVector, N));
		float3 TangentY = cross(N, TangentX);

		// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize
		float fOmegaP = 4.0 * PI / (6.0 * EnvMapSize * EnvMapSize);
	//	[unroll(128)]
		for (uint i = 0; i < NumSamples; i++)
		{
			float2 Xi = hammersley2d(i, NumSamples);
			float3 H = ImportanceSampleGGX(Xi, Roughness, N, TangentX, TangentY);
			float3 L = 2 * dot(V, H) * H - V;
			float NoL = dot(N, L);

			if (NoL > 0)
			{

				// Vectors to evaluate pdf
				float NdotH = saturate(dot(N, H));
				float LdotH = saturate(dot(L, H));

				// Probability Distribution Function
				float fPdf = D_GGX_Divide_Pi(Roughness, NdotH)*NdotH / (4.0f * LdotH);

				// Solid angle represented by this sample
				float fOmegaS = 1.0 / (NumSamples * fPdf);

				// Original paper suggest biasing the mip to improve the results
				//float fMipBias = 1.0f;
				int fMipLevel = clamp(0.5 * log2(fOmegaS / fOmegaP)+1, 0, 9);

			//	PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL;
				PrefilteredColor += tex_cube.SampleLevel(LinearSampler, L, fMipLevel).rgb * NoL;
				TotalWeight += NoL;
			}
		}
		return  PrefilteredColor / TotalWeight;
	
}








float4 PS(quad_output i) : SV_TARGET0
{

	float3 itc = normalize(i.tc);
//	return float4(float(roughness).xxx ,1);
	
	return float4(PrefilterEnvMap(roughness,itc),1);

}


#endif

#ifdef BUILD_FUNC_PS_Diffuse


float3 PrefilterDiffuse(float Roughness, float3 R)
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	float TotalWeight = 0.0;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	const uint NumSamples = 32;
	// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize
	float fOmegaP = 4.0 * PI / (6.0 * EnvMapSize * EnvMapSize);
	//	[unroll(128)]
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley2d(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N, TangentX, TangentY);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = dot(N, L);

		if (NoL > 0)
		{

			// Vectors to evaluate pdf
			float NdotH = saturate(dot(N, H));
			float LdotH = saturate(dot(L, H));

			// Probability Distribution Function
			float fPdf = D_GGX_Divide_Pi(Roughness, NdotH)*NdotH / (4.0f * LdotH);

			// Solid angle represented by this sample
			float fOmegaS = 1.0 / (NumSamples * fPdf);

			// Original paper suggest biasing the mip to improve the results
			//float fMipBias = 1.0f;
			int fMipLevel = clamp(0.5 * log2(fOmegaS / fOmegaP) + 1, 0, 9);

			//	PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL;
			PrefilteredColor += tex_cube.SampleLevel(LinearSampler, L, fMipLevel).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return  PrefilteredColor / TotalWeight;

}



float4 PS_Diffuse(quad_output i) : SV_TARGET0
{

	float3 itc = normalize(i.tc);
	//	return float4(float(roughness).xxx ,1);

		return float4(PrefilterDiffuse(1,itc),1);

}


/*
float4 PS(quad_output i) : SV_TARGET0
{

    float3 itc = normalize(i.tc);

	// float4(textureAVG(itc),1);



    float3x3 vecSpace = matrixFromVector(itc);

    float3 result = 0;
    static const int samples = 256;

    for (int i = 0; i < samples; i++)
    {
        float sini = sin(float(i));
        float cosi = cos(float(i));
        float rand = rnd(float2(sini, cosi));
        float3 t = hemisphereSample_cos(float2(float(i) / float(samples), rand), vecSpace, itc,(1-0.8*float(mip)/8));

		//float3 t = hemisphereSample_phong(float2(float(i) / float(samples), rand), vecSpace, itc, 512);
        result += tex_cube.SampleLevel(LinearSampler, t, 0);
    }


    return float4(result / samples, 1);

}*/



/*
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

}*/

#endif