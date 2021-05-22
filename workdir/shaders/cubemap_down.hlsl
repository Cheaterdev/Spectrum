
#include "autogen/EnvSource.h"
#include "autogen/EnvFilter.h"

static const uint EnvMapSize = GetEnvFilter().GetSize().x;
static const float roughness = GetEnvFilter().GetScaler().x;

struct quad_output
{
float4 pos : SV_POSITION;
float3 tc: TEXCOORD0;
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
    Output.tc = normalize(mul(mats[GetEnvFilter().GetFace().x], float3(Pos[index], 1)));
    return Output;
}
#endif

#include "Common.hlsl"

#ifdef BUILD_FUNC_PS
float3 PrefilterEnvMap(float Roughness, float3 R)
{
		float3 N = R;
		float3 V = R;
		float3 PrefilteredColor = 0;
		float TotalWeight = 0.0;

		// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize
		float fOmegaP = 4.0 * PI / (6.0 * EnvMapSize * EnvMapSize);

		float3x3 space = CalculateTangent(N);
		float a = Roughness * Roughness;
	//	[unroll(128)]
		for (uint i = 0; i < NumSamples; i++)
		{
			float2 Xi = hammersley2d(i, NumSamples);
			float3 H = ImportanceSampleGGX(Xi, a, space);
			float3 L = 2 * dot(V, H) * H - V;
			float NoL = dot(N, L);

			if (NoL > 0)
			{

				// Vectors to evaluate pdf
				float NdotH = saturate(dot(N, H));
				float LdotH = saturate(dot(L, H));

				// Probability Distribution Function
				float fPdf = D_GGX_Divide_Pi(a, NdotH)*NdotH / (4.0f * LdotH);

				// Solid angle represented by this sample
				float fOmegaS = 1.0 / (NumSamples * fPdf);

				// Original paper suggest biasing the mip to improve the results
				//float fMipBias = 1.0f;
				int fMipLevel = clamp(0.5 * log2(fOmegaS / fOmegaP)+1, 0, 9);

			//	PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL;
				PrefilteredColor += GetEnvSource().GetSourceTex().SampleLevel(linearSampler, L, fMipLevel).rgb * NoL;
				TotalWeight += NoL;
			}
		}
		return PrefilteredColor / TotalWeight;
	
}








float4 PS(quad_output i) : SV_TARGET0
{

	float3 itc = normalize(i.tc);
	//return float4(itc,1);
	
	return float4(PrefilterEnvMap(roughness, itc), 1);

}


#endif

#ifdef BUILD_FUNC_PS_Diffuse


float3 PrefilterDiffuse(float Roughness, float3 R)
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	float TotalWeight = 0.0;

	float3x3 space = CalculateTangent(N);

	const uint NumSamples = 32;
	// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize
	float fOmegaP = 4.0 * PI / (6.0 * EnvMapSize * EnvMapSize);
	//	[unroll(128)]
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley2d(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, space);
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
			PrefilteredColor += GetEnvSource().GetSourceTex().SampleLevel(linearSampler, L, fMipLevel).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return  PrefilteredColor / TotalWeight;

}



float4 PS_Diffuse(quad_output i) : SV_TARGET0
{

	float3 itc = normalize(i.tc);
	//	return float4(float(roughness).xxx ,1);

	return float4(PrefilterDiffuse(1, itc), 1);

}

#endif