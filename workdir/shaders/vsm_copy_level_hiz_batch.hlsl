
#include "autogen/VSMCopyLevelHiZBatch.h"

static const VSMCopyLevelHiZBatch data = GetVSMCopyLevelHiZBatch();

// Matches VSMPageTable::VSM_INVALID_SLOT -- this file has no other reason
// to include VSM_impl.hlsl (where the same #define already lives), so it's
// repeated here rather than pulling that whole file in for one constant.
#define VSM_INVALID_SLOT 0xFFFFFFFFu

// Feeds VSMPageHiZ's level_hiz field (Phase 5.18 follow-up multi-page
// pyramid): one thread per (level, local page) entry -- EVERY local page of
// every active level, every frame, not just dirty ones (see VSM.cpp's
// collection loop for why). Mapped entries copy that page's own page_hiz
// coarsest-mip summary (a single texel -- the whole page's min/max, since
// page_hiz's mip chain now reaches 1x1) into level_hiz's corresponding
// per-page texel at mip 0. Unmapped entries (physical_slot ==
// VSM_INVALID_SLOT -- not needed this frame, or needed but the pool was
// exhausted) write float2(0, 0) instead: the same "treat missing as empty,
// reversed-Z far" convention already used for a genuinely-unrendered atlas
// texel elsewhere in this system, rather than leaving a stale value from
// whenever this page was last mapped (which could persist indefinitely
// otherwise -- pages get evicted and never revisited for a long time).
// Dispatch Z is exactly the entry count (no over-dispatch, matching
// vsm_copy_page_depth_batch.hlsl's own Z-exact-count shape), so no bounds
// check is needed.
[numthreads(1, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	VSMDirtyPageInfo info = data.GetDirty_pages()[dispatchID.z];
	uint slot = info.GetPhysical_slot();
	float2 v = (slot == VSM_INVALID_SLOT)
		? float2(0, 0)
		: data.GetPage_hiz_coarsest().Load(int4(0, 0, (int)slot, 0));
	data.GetLevel_hiz_mip0()[uint3(info.GetPage_x(), info.GetPage_y(), info.GetLevel())] = v;
}
