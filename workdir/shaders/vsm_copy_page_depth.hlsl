
#include "autogen/VSMCopyPageDepth.h"

static const VSMCopyPageDepth data = GetVSMCopyPageDepth();
static const Texture2D<float> atlas = data.GetAtlas();
static const RWTexture2D<float> dst_mip0 = data.GetDst_mip0();

// 1:1 copy of one page's region of the shared VSM_Atlas into VSMPageHiZ's
// mip 0 for that page's physical slot -- mip 0 needs to stay at the page's
// own resolution (not an 8x reduction like downsample_depth.hlsl) so the
// finest Hi-Z level can still resolve small meshlets.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	int2 origin = data.GetAtlas_origin();
	dst_mip0[dispatchID.xy] = atlas[origin + (int2)dispatchID.xy];
}
