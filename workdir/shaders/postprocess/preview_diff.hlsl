#include "../common/common.hlsl"

#include "../autogen/PreviewDiffParams.h"

// |a - b| x gain: the material preview's "what does the lobe add" view.
[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	uint2 dims;
	GetPreviewDiffParams().GetResult().GetDimensions(dims.x, dims.y);

	if (any(id.xy >= dims))
		return;

	float3 a = GetPreviewDiffParams().GetA()[id.xy].rgb;
	float3 b = GetPreviewDiffParams().GetB()[id.xy].rgb;

	GetPreviewDiffParams().GetResult()[id.xy] = float4(saturate(abs(a - b) * GetPreviewDiffParams().GetGain()), 1);
}
