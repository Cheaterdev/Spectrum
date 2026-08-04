
#include "autogen/DownsampleDepthMip.h"

static const Texture2D<float> src_mip = GetDownsampleDepthMip().GetSrcMip();
static const RWTexture2D<float> dst_mip = GetDownsampleDepthMip().GetDstMip();

// 2x2 -> 1 mip halving, unlike downsample_depth.hlsl's one-shot 8x8 -> 1.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 dst = dispatchID.xy;

	uint dstW, dstH;
	dst_mip.GetDimensions(dstW, dstH);
	if (dst.x >= dstW || dst.y >= dstH) return;

	uint srcW, srcH;
	src_mip.GetDimensions(srcW, srcH);

	uint2 s = dst * 2;

	// reversed-Z: min keeps the farthest depth (conservative occluder).
	float d = min(min(src_mip[s], src_mip[s + uint2(1, 0)]),
	              min(src_mip[s + uint2(0, 1)], src_mip[s + uint2(1, 1)]));

	// Odd source dimension: floor(src/2) leaves a trailing row/column no 2x2
	// step ever reads. Dropping it raises the min, i.e. reports surfaces as
	// CLOSER than they are, which over-culls. Fold it into the last texel.
	if ((srcW & 1) && dst.x == dstW - 1)
	{
		d = min(d, src_mip[uint2(srcW - 1, s.y)]);
		d = min(d, src_mip[uint2(srcW - 1, min(s.y + 1, srcH - 1))]);
	}
	if ((srcH & 1) && dst.y == dstH - 1)
	{
		d = min(d, src_mip[uint2(s.x, srcH - 1)]);
		d = min(d, src_mip[uint2(min(s.x + 1, srcW - 1), srcH - 1)]);
	}
	if ((srcW & 1) && (srcH & 1) && dst.x == dstW - 1 && dst.y == dstH - 1)
		d = min(d, src_mip[uint2(srcW - 1, srcH - 1)]);

	dst_mip[dst] = d;
}
