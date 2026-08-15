
#include "autogen/VSMDownsampleHiZBatch.h"

static const VSMDownsampleHiZBatch data = GetVSMDownsampleHiZBatch();

// Batched form of downsample_depth_mip.hlsl's 2x2->1 mip halving: one
// dispatch covers every dirty page's mip step this frame (Z = dirty page
// count) instead of one dispatch per dirty page per mip. src/dst_mip are
// both array-spanning views narrowed to exactly ONE mip each (not a view
// spanning the whole mip chain -- keeps HAL::Transitions::stop_using()'s
// per-bind cost proportional to one mip's worth of subresources instead of
// the whole pyramid's). Load's mip component is always 0, relative to the
// already-narrowed view.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 dst = dispatchID.xy;
	uint slot = data.GetDirty_slots().Load(dispatchID.z);

	uint dstW, dstH, dstElems;
	data.GetDst_mip().GetDimensions(dstW, dstH, dstElems);
	if (dst.x >= dstW || dst.y >= dstH) return;

	uint srcW, srcH, srcElems;
	data.GetSrc().GetDimensions(srcW, srcH, srcElems);

	uint2 s = dst * 2;

	// reversed-Z: min keeps the farthest depth (conservative occluder).
	float d = min(min(data.GetSrc().Load(int4(s.x,     s.y,     slot, 0)),
	                   data.GetSrc().Load(int4(s.x + 1, s.y,     slot, 0))),
	              min(data.GetSrc().Load(int4(s.x,     s.y + 1, slot, 0)),
	                  data.GetSrc().Load(int4(s.x + 1, s.y + 1, slot, 0))));

	// Odd source dimension: floor(src/2) leaves a trailing row/column no 2x2
	// step ever reads. Dropping it raises the min, i.e. reports surfaces as
	// CLOSER than they are, which over-culls. Fold it into the last texel.
	if ((srcW & 1) && dst.x == dstW - 1)
	{
		d = min(d, data.GetSrc().Load(int4(srcW - 1, s.y, slot, 0)));
		d = min(d, data.GetSrc().Load(int4(srcW - 1, min(s.y + 1, srcH - 1), slot, 0)));
	}
	if ((srcH & 1) && dst.y == dstH - 1)
	{
		d = min(d, data.GetSrc().Load(int4(s.x, srcH - 1, slot, 0)));
		d = min(d, data.GetSrc().Load(int4(min(s.x + 1, srcW - 1), srcH - 1, slot, 0)));
	}
	if ((srcW & 1) && (srcH & 1) && dst.x == dstW - 1 && dst.y == dstH - 1)
		d = min(d, data.GetSrc().Load(int4(srcW - 1, srcH - 1, slot, 0)));

	data.GetDst_mip()[uint3(dst, slot)] = d;
}
