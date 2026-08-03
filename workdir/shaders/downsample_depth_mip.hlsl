
#include "autogen/DownsampleDepthMip.h"

static const Texture2D<float> src_mip = GetDownsampleDepthMip().GetSrcMip();
static const RWTexture2D<float> dst_mip = GetDownsampleDepthMip().GetDstMip();

// Standard mip-chain halving (2x2 -> 1), unlike downsample_depth.hlsl's
// one-shot 8x8 -> 1 reduction. No groupshared reduction needed for only 4
// taps -- direct loads are simpler and just as fast here.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 tc = dispatchID.xy;
	uint2 srcCoord = tc * 2;

	float d00 = src_mip[srcCoord + uint2(0, 0)];
	float d10 = src_mip[srcCoord + uint2(1, 0)];
	float d01 = src_mip[srcCoord + uint2(0, 1)];
	float d11 = src_mip[srcCoord + uint2(1, 1)];

	// reversed-Z: keep the FARTHEST depth (min) so each coarser mip stays a
	// conservative occluder, same convention as downsample_depth.hlsl.
	dst_mip[tc] = min(min(d00, d10), min(d01, d11));
}
