#include "../autogen/DDGIProbeResidencyMarkData.h"

// Real hit-point-driven marking (see [[project-ddgi]] planning notes):
// consumes DDGI_ProbeResidencyPending -- whatever TraceIndirectDiffuse
// (IndirectRTX/IndirectRTXHalf, raytracing.hlsl) wrote at its own per-pixel
// indirect ray's hit points LAST frame, one probe cell per hit, see that
// buffer's own comment (ddgi.sig) for why one frame lagged -- into
// DDGI_ProbeResidency, clearing pending back to 0 as it's consumed so each
// mark is used exactly once. The coarsest cascade (DDGIInfo::cascade_info.w)
// is exempt by default: forced fully resident regardless of pending, since
// it has nowhere further to fall back to -- DDGIInfo::flags.z
// (cull_coarsest_cascade, mirrored from DDGIGraph.cpp's own Variable<bool>)
// turns that exemption off, culling it the same as every other cascade.
// See that flag's own comment (ddgi.sig) for what turning it off risks:
// the coarsest-cascade sampling fallback has no residency check of its own
// yet, so a probe this drops can be read back stale.
//
// Eviction has a grace period (DDGI_ProbeMissStreak, DDGIInfo::flags.w):
// a probe that's currently resident but goes unhit doesn't drop the instant
// one frame's pending bit comes back 0 -- it only evicts after
// flags.w CONSECUTIVE missed frames. At 0 grace, a single frame's worth of
// screen-ray noise/occlusion made probes visibly flicker in and out of
// residency (and therefore in and out of being traced/convolved) even
// though the actual need for them hadn't changed. No dilation yet (a probe
// several hops back in the multi-bounce feedback chain can drop out the
// frame it stops being directly hit, independent of this grace period,
// since it was never itself hit to begin with) and no cross-cascade
// fallback for a probe that isn't resident -- both later steps, not yet
// implemented.
//
// ALSO stream-compacts the marked set into DDGI_CompactedProbeList (dense
// list of needed probes' linear indices, from index 0) and
// DDGI_CompactedProbeCount (how many) -- what DDGIProbeDispatchArgsBuild
// reads to size the indirect DispatchRays launch, and what
// ddgi_probe_trace.hlsl's raygen shader looks a compacted dispatch index up
// through.
//
// Two dispatches, same PSO/shader, distinguished by reset_only: the first
// (1 thread) zeroes this cascade's own DDGI_CompactedProbeCount slot: this
// can't be folded into the marking dispatch below, since InterlockedAdd
// from thousands of threads spread across many thread groups has no way to
// guarantee a single thread's zeroing write happens before another
// thread's add without a separate dispatch boundary in between.
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DDGIProbeResidencyMarkData data = GetDDGIProbeResidencyMarkData();
	uint cascade_index = data.GetInfo().GetCascade_info().z;

	if (data.GetReset_only() != 0)
	{
		if (dispatchID.x == 0)
			data.GetCompacted_count()[cascade_index] = 0;
		return;
	}

	uint4 probe_counts = data.GetInfo().GetProbe_counts();
	uint probe_count = probe_counts.x * probe_counts.y * probe_counts.z;

	uint linear_index = dispatchID.x;
	if (linear_index >= probe_count)
		return;

	uint probe_offset = data.GetInfo().GetCascade_info().x;
	uint buffer_index = probe_offset + linear_index;

	uint control_flags = data.GetInfo().GetFlags().z;
	bool cull_coarsest = (control_flags & (uint)DDGIControlFlags::CullCoarsestCascade) != 0;
	bool culling_disabled = (control_flags & (uint)DDGIControlFlags::DisableResidencyCulling) != 0;

	bool is_coarsest = data.GetInfo().GetCascade_info().w != 0;
	bool coarsest_exempt = is_coarsest && !cull_coarsest;

	bool hit_this_cycle = data.GetPending()[buffer_index] != 0;
	bool was_resident = data.GetProbe_residency()[buffer_index] != 0;
	uint grace_frames = data.GetInfo().GetFlags().w;

	uint streak = data.GetMiss_streak()[buffer_index];
	if (hit_this_cycle)
		streak = 0;
	else if (was_resident)
		streak = streak + 1;
	// else: not resident and not hit -- leave streak as-is, it's not read
	// by anything while was_resident is false.

	// culling_disabled forces every probe needed, matching the original
	// pre-culling "trace everything" behavior -- an A/B lever independent
	// of the coarsest-cascade bit above.
	bool needed = culling_disabled || coarsest_exempt || hit_this_cycle || (was_resident && streak <= grace_frames);

	data.GetMiss_streak()[buffer_index] = streak;
	data.GetProbe_residency()[buffer_index] = needed ? 1 : 0;
	data.GetPending()[buffer_index] = 0;

	if (needed)
	{
		uint dest_index;
		InterlockedAdd(data.GetCompacted_count()[cascade_index], 1, dest_index);
		data.GetCompacted_list()[probe_offset + dest_index] = linear_index;
	}
}
