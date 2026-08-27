
#include "autogen/VSMDownsampleHiZBatch.h"

static const VSMDownsampleHiZBatch data = GetVSMDownsampleHiZBatch();

// Batched form of downsample_depth_mip.hlsl's 2x2->1 mip halving: one
// dispatch covers every dirty page's mip step this frame (Z = dirty page
// count) instead of one dispatch per dirty page per mip. src/dst_mip are
// both array-spanning views narrowed to exactly ONE mip each (not a view
// spanning the whole mip chain -- keeps HAL::Transitions::stop_using()'s
// per-bind cost proportional to one mip's worth of subresources instead of
// the whole pyramid's). src is a UAV (RWTexture2DArray), not an SRV, even
// though this shader only ever reads it -- see VSMDownsampleHiZBatch's own
// comment in vsm.sig for why (keeps the whole VSM_PageHiZ resource in one
// layout for this entire pass, avoiding a real barrier-layout conflict
// confirmed via GPU-Based Validation).
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

	// .x = running MIN (farthest depth, conservative occluder -- unchanged
	// semantics). .y = running MAX (closest depth, Phase 5.18 Part A --
	// drives VSM_BlockerSearch's classification). Same 2x2 (+ edge-fold)
	// footprint for both, just min() and max() of the same four taps.
	float2 t00 = data.GetSrc()[uint3(s.x,     s.y,     slot)];
	float2 t10 = data.GetSrc()[uint3(s.x + 1, s.y,     slot)];
	float2 t01 = data.GetSrc()[uint3(s.x,     s.y + 1, slot)];
	float2 t11 = data.GetSrc()[uint3(s.x + 1, s.y + 1, slot)];
	float dMin = min(min(t00.x, t10.x), min(t01.x, t11.x));
	float dMax = max(max(t00.y, t10.y), max(t01.y, t11.y));

	// Odd source dimension: floor(src/2) leaves a trailing row/column no 2x2
	// step ever reads. Dropping it raises the min, i.e. reports surfaces as
	// CLOSER than they are, which over-culls (and symmetrically would lower
	// the max, under-reporting the closest occluder). Fold it into the last
	// texel, both channels.
	if ((srcW & 1) && dst.x == dstW - 1)
	{
		float2 e0 = data.GetSrc()[uint3(srcW - 1, s.y, slot)];
		float2 e1 = data.GetSrc()[uint3(srcW - 1, min(s.y + 1, srcH - 1), slot)];
		dMin = min(dMin, min(e0.x, e1.x));
		dMax = max(dMax, max(e0.y, e1.y));
	}
	if ((srcH & 1) && dst.y == dstH - 1)
	{
		float2 e0 = data.GetSrc()[uint3(s.x, srcH - 1, slot)];
		float2 e1 = data.GetSrc()[uint3(min(s.x + 1, srcW - 1), srcH - 1, slot)];
		dMin = min(dMin, min(e0.x, e1.x));
		dMax = max(dMax, max(e0.y, e1.y));
	}
	if ((srcW & 1) && (srcH & 1) && dst.x == dstW - 1 && dst.y == dstH - 1)
	{
		float2 e = data.GetSrc()[uint3(srcW - 1, srcH - 1, slot)];
		dMin = min(dMin, e.x);
		dMax = max(dMax, e.y);
	}

	data.GetDst_mip()[uint3(dst, slot)] = float2(dMin, dMax);
}
