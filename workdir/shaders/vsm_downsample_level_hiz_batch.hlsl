
#include "autogen/VSMDownsampleLevelHiZBatch.h"

static const VSMDownsampleLevelHiZBatch data = GetVSMDownsampleLevelHiZBatch();

// Downsamples VSMPageHiZ's level_hiz mip chain (pages_per_level^2 texels at
// mip 0, e.g. 4x4, down to 1x1) -- unconditionally, every active level,
// every frame, not gated by a dirty list the way vsm_hiz_downsample_batch.
// hlsl (the per-page version) has to be. At this resolution (a handful of
// texels per level, MaxLevels slices total) it's cheaper to just always
// redo it than to track which levels actually changed. dispatchID.z IS the
// level index directly -- no dirty_slots indirection.
//
// No odd-dimension edge-fold (unlike the per-page downsample): pages_per_
// level is a power of 2 (4), so every mip transition (4->2, 2->1) divides
// evenly. Would need revisiting if pages_per_level ever became non-power-
// of-2.
//
// -INF/+INF poison values (VSM.cpp's cold-start clear, for a page that's
// never been resident) propagate through min()/max() exactly like a real
// spike would -- no special-casing needed here for "unknown" texels; they
// just naturally infect whatever coarser texel they're folded into.
[numthreads(4, 4, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	uint2 dst = dispatchID.xy;
	uint level = dispatchID.z;

	uint dstW, dstH, dstElems;
	data.GetDst_mip().GetDimensions(dstW, dstH, dstElems);
	if (dst.x >= dstW || dst.y >= dstH)
		return;

	uint2 s = dst * 2;
	float2 t00 = data.GetSrc().Load(int4(s.x,     s.y,     level, 0));
	float2 t10 = data.GetSrc().Load(int4(s.x + 1, s.y,     level, 0));
	float2 t01 = data.GetSrc().Load(int4(s.x,     s.y + 1, level, 0));
	float2 t11 = data.GetSrc().Load(int4(s.x + 1, s.y + 1, level, 0));

	float dMin = min(min(t00.x, t10.x), min(t01.x, t11.x));
	float dMax = max(max(t00.y, t10.y), max(t01.y, t11.y));

	data.GetDst_mip()[uint3(dst, level)] = float2(dMin, dMax);
}
