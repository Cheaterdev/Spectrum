#include "../autogen/DDGIProbeResidencyMarkData.h"

// Real hit-point-driven marking (see [[project-ddgi]] planning notes):
// consumes DDGI_ProbeResidencyPending -- whatever TraceIndirectDiffuse
// (IndirectRTX/IndirectRTXHalf, raytracing.hlsl) wrote at its own per-pixel
// indirect ray's hit points LAST frame, one probe cell per hit, see that
// buffer's own comment (ddgi.sig) for why one frame lagged -- into
// DDGI_ProbeResidency, clearing pending back to 0 as it's consumed so each
// mark is used exactly once. The coarsest cascade (DDGIInfo::cascade_info.w)
// is exempt: forced fully resident regardless of pending, since it has
// nowhere further to fall back to. No dilation yet (a probe several hops
// back in the multi-bounce feedback chain can drop out the frame it stops
// being directly hit) and no cross-cascade fallback for a probe that isn't
// resident -- both later steps, not yet implemented; start with direct
// hit-marking alone and observe whether either is visibly needed.
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

	bool is_coarsest = data.GetInfo().GetCascade_info().w != 0;
	bool needed = is_coarsest || (data.GetPending()[buffer_index] != 0);

	data.GetProbe_residency()[buffer_index] = needed ? 1 : 0;
	data.GetPending()[buffer_index] = 0;

	if (needed)
	{
		uint dest_index;
		InterlockedAdd(data.GetCompacted_count()[cascade_index], 1, dest_index);
		data.GetCompacted_list()[probe_offset + dest_index] = linear_index;
	}
}
