#define SIZE 4
static const uint TOTAL_THREADS = SIZE * SIZE;
RWTexture3D<float4> output : register(u0);
#include "Common.hlsl"



float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
{
	float3 V;

	float3 N = float3(0.0f, 0.0f, 1.0f);

	V.x = sqrt(1.0f - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;
	const uint NumSamples = 1024;

	//[loop]
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley2d(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));
		if (NoL > 0)
		{
			float G = G_Smith(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = FresnelSchlick(Metallic, VoH);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return float2(A, B) / NumSamples;
}

[numthreads(SIZE, SIZE, SIZE)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
	const uint3 tc = (group_id.xyz * SIZE + thread_id.xyz);
	if (any(tc < 0) || any(tc > 15)) return;

	float3 ftc  = (float3(tc))/15.0;

	output[tc] =  float4(IntegrateBRDF(ftc.x, ftc.y, ftc.z), 0, 1);
}




