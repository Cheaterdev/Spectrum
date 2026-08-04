
#include "autogen/VSMCopyPageDepth.h"

static const VSMCopyPageDepth data = GetVSMCopyPageDepth();
static const Texture2D<float> atlas = data.GetAtlas();
static const RWTexture2D<float> dst_mip0 = data.GetDst_mip0();

// 1:1 copy into VSMPageHiZ's mip 0 (full page resolution, not an 8x
// reduction, so small meshlets still resolve at the finest Hi-Z level).
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	int2 origin = data.GetAtlas_origin();
	dst_mip0[dispatchID.xy] = atlas[origin + (int2)dispatchID.xy];
}
