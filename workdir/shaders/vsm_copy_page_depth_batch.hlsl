
#include "autogen/VSMCopyPageDepthBatch.h"

static const VSMCopyPageDepthBatch data = GetVSMCopyPageDepthBatch();

// Batched form of vsm_copy_page_depth.hlsl: one dispatch covers every dirty
// page this frame (Z = dirty page count) instead of one dispatch per page.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint slot = data.GetDirty_slots().Load(dispatchID.z);
	uint3 coord = uint3(dispatchID.xy, slot);
	float d = data.GetAtlas().Load(int4(coord, 0));
	// Mip 0: min and max both start equal to the single real sample --
	// they only diverge once downsampling reduces more than one texel.
	data.GetDst_mip0()[coord] = float2(d, d);
}
