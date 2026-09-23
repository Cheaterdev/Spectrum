#include "../autogen/NormalRoughnessRepackParams.h"

static const Texture2D<float4>   Normals              = GetNormalRoughnessRepackParams().GetGBuffer_Normals();
static const Texture2D<float4>   Albedo               = GetNormalRoughnessRepackParams().GetGBuffer_Albedo();
static const RWTexture2D<float4> Output               = GetNormalRoughnessRepackParams().GetOutput();
static const RWTexture2D<float4> SpecularAlbedoOutput = GetNormalRoughnessRepackParams().GetSpecularAlbedoOutput();

[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 size;
	Output.GetDimensions(size.x, size.y);
	if (any(dispatchID.xy >= size)) return;

	float4 encoded = Normals.Load(uint3(dispatchID.xy, 0));

	// compress_normals() (FrameData.prism) always scales the unit normal by a
	// POSITIVE scalar (best-fit LUT value / cube-face projection factor)
	// before biasing to [0,1] -- so re-normalizing the decoded value recovers
	// the original direction exactly, without needing the LUT a second time.
	float3 normal = normalize(encoded.xyz * 2.0 - 1.0);

	Output[dispatchID.xy] = float4(normal, encoded.w);

	float4 albedo = Albedo.Load(uint3(dispatchID.xy, 0));
	float3 f0 = lerp(0.04, albedo.rgb, albedo.a);
	SpecularAlbedoOutput[dispatchID.xy] = float4(f0, 1);
}
