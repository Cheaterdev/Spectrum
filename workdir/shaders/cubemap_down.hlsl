
#include "autogen/EnvSource.h"
#include "autogen/EnvFilter.h"

// Edge length of the SOURCE cubemap -- feeds the solid-angle estimate that
// picks which source mip each sample reads. Not the size of what we write.
static const uint EnvMapSize = GetEnvFilter().GetSize().x;

static float3x3 mats[] =
{
    float3x3(0, 0, 1,   0, 1, 0,   -1, 0, 0), //X+
    float3x3(0, 0, -1,  0, 1, 0,   1, 0, 0), //X-

    float3x3(1, 0, 0,   0, 0, 1,   0, -1, 0), //Y+
    float3x3(1, 0, 0,   0, 0, -1,  0, 1, 0), //Y-

    float3x3(1, 0, 0,   0, 1, 0,   0, 0, 1), //Z+
    float3x3(-1, 0, 0,   0, 1, 0,   0, 0, -1) //Z-
};

// Texel -> cube direction. The old VS normalized the corner rays before
// interpolation, which warped the face non-linearly; taking mul() of the
// interpolated clip position and normalizing once is the direction a hardware
// TextureCube lookup actually maps to that texel.
float3 face_direction(uint2 texel, uint face, uint size)
{
    float2 uv = (float2(texel) + 0.5) / float(size);
    float2 clip = uv * float2(2, -2) + float2(-1, 1);
    return normalize(mul(mats[face], float3(clip, 1)));
}

#include "Common.hlsl"

#ifdef BUILD_FUNC_CS

// Sample count per output mip, indexed by min(mip, 4). Used to be the
// NumSamples PSO define; it is a per-thread lookup now that one dispatch spans
// every mip.
static const uint MipSamples[] = { 1, 8, 32, 64, 128 };

float3 PrefilterEnvMap(float Roughness, float3 R, uint NumSamples)
{
		float3 N = R;
		float3 V = R;
		float3 PrefilteredColor = 0;
		float TotalWeight = 0.0;

		// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize
		float fOmegaP = 4.0 * PI / (6.0 * EnvMapSize * EnvMapSize);

		float3x3 space = CalculateTangent(N);
		float a = Roughness * Roughness;

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

				PrefilteredColor += GetEnvSource().GetSourceTex().SampleLevel(linearSampler, L, fMipLevel).rgb * NoL;
				TotalWeight += NoL;
			}
		}
		return PrefilteredColor / TotalWeight;
	
}


// Every mip of every face of the specular cubemap in one dispatch.
//
// The output mips have different sizes, so the domain is the concatenation of
// all of them -- mip 0's 6*w*w texels, then mip 1's, and so on -- walked as a
// flat 1D index. Threads stay one-texel-per-thread that way, which keeps the
// expensive high-roughness mips spread across the grid instead of piled into
// one corner of a mip-0-sized dispatch.
[numthreads(64, 1, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	const uint mip_count = GetEnvFilter().GetSize().y;
	const uint base_size = GetEnvFilter().GetSize().z;

	uint index = DTid.x;

	// Walk the per-mip spans until the one holding this thread's texel.
	uint mip = 0;
	uint size = base_size;
	uint start = 0;

	[loop]
	while (mip + 1 < mip_count && index >= start + size * size * 6)
	{
		start += size * size * 6;
		size = max(size >> 1, 1u);
		mip++;
	}

	// Tail of the last thread group.
	uint local = index - start;
	if (local >= size * size * 6)
		return;

	uint face = local / (size * size);
	uint texel = local - face * size * size;
	uint2 xy = uint2(texel % size, texel / size);

	float roughness = (float(mip) + 0.5) / mip_count;
	float3 itc = face_direction(xy, face, size);

	GetEnvFilter().GetTargets(mip)[uint3(xy, face)] =
		float4(PrefilterEnvMap(roughness, itc, MipSamples[min(mip, 4u)]), 1);
}

#endif

#ifdef BUILD_FUNC_CS_Diffuse


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

			PrefilteredColor += GetEnvSource().GetSourceTex().SampleLevel(linearSampler, L, fMipLevel).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return  PrefilteredColor / TotalWeight;

}


// The diffuse cubemap is mip 0 only, so one (w, h, 6) dispatch covers it.
[numthreads(8, 8, 1)]
void CS_Diffuse(uint3 DTid : SV_DispatchThreadID)
{
	uint3 dims;
	GetEnvFilter().GetTargets(0).GetDimensions(dims.x, dims.y, dims.z);

	if (any(DTid >= dims))
		return;

	float3 itc = face_direction(DTid.xy, DTid.z, dims.x);

	GetEnvFilter().GetTargets(0)[DTid] = float4(PrefilterDiffuse(1, itc), 1);
}

#endif
