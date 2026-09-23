#include "../autogen/DDGIProbeResidencyMarkData.h"
#include "../autogen/tables/DDGIProbes.h"

// Real hit-point-driven marking (see [[project-ddgi]] planning notes):
// consumes DDGI_ProbeResidencyPending -- whatever TraceIndirectDiffuse
// (IndirectRTX/IndirectRTXHalf, raytracing.hlsl) wrote at its own per-pixel
// indirect ray's hit points LAST frame, one probe cell per hit, see that
// buffer's own comment (ddgi.prism) for why one frame lagged -- into
// DDGI_ProbeResidency, clearing pending back to 0 as it's consumed so each
// mark is used exactly once. The coarsest cascade (DDGIInfo::cascade_info.w)
// is exempt by default: forced fully resident regardless of pending, since
// it has nowhere further to fall back to -- DDGIInfo::flags.z
// (cull_coarsest_cascade, mirrored from DDGIGraph.cpp's own Variable<bool>)
// turns that exemption off, culling it the same as every other cascade.
// See that flag's own comment (ddgi.prism) for what turning it off risks:
// the coarsest-cascade sampling fallback has no residency check of its own
// yet, so a probe this drops can be read back stale.
//
// Eviction has a grace period (DDGI_ProbeMissStreak, DDGIInfo::flags.w):
// a probe that's currently resident but goes unhit doesn't drop the instant
// one frame's pending bit comes back 0 -- it only evicts after
// flags.w CONSECUTIVE missed frames. At 0 grace, a single frame's worth of
// screen-ray noise/occlusion made probes visibly flicker in and out of
// residency (and therefore in and out of being traced/convolved) even
// though the actual need for them hadn't changed. Dilation (a probe several
// hops back in the multi-bounce feedback chain marking its own 8 feedback-
// read neighbors needed too, so it doesn't drop the frame it stops being
// directly hit) is handled at the mark sites themselves (TraceIndirectDiffuse/
// TraceReflection/DDGIProbeTrace's own feedback read, raytracing.hlsl/
// ddgi_probe_trace.hlsl) -- this pass only consumes what they wrote. No
// cross-cascade fallback yet for a probe that isn't resident -- still
// deferred.
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

	uint3 probe_counts = data.GetInfo().GetProbe_counts().xyz;
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

	// Toroidal-scroll forced eviction (see DDGIProbeResidencyMarkData's own
	// comment, ddgi.prism, for the derivation): a probe's slot on axis A is
	// being re-tenanted this frame -- its stored data is for whatever cell
	// USED to alias here, not the one that does now -- iff
	// wrap(slot[A] - scroll_lo[A], counts[A]) < scroll_count[A]. Checked per
	// axis since the window can (rarely) move on more than one axis in the
	// same frame; ANY axis matching means this exact probe just changed
	// meaning. Overrides everything below (including the coarsest-cascade
	// exemption -- an evicted coarsest-cascade probe still needs a real
	// retrace before its old data can be trusted again, exemption or not).
	DDGIProbes probes;
	uint3 slot = probes.ddgi_probe_grid_coord(linear_index, probe_counts);
	int3 scroll_lo = data.GetScroll_lo();
	uint3 scroll_count = data.GetScroll_count();
	uint3 scroll_offset = uint3(probes.ddgi_wrap(int3(slot) - scroll_lo, probe_counts));
	bool scroll_evicted = any(scroll_offset < scroll_count);

	bool hit_this_cycle = !scroll_evicted && data.GetPending()[buffer_index] != 0;
	bool was_resident = !scroll_evicted && data.GetProbe_residency()[buffer_index] != 0;
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
	// of the coarsest-cascade bit above. scroll_evicted overrides all of
	// these: forced non-resident this frame regardless of hits/exemptions.
	bool needed = !scroll_evicted && (culling_disabled || coarsest_exempt || hit_this_cycle || (was_resident && streak <= grace_frames));

	data.GetMiss_streak()[buffer_index] = scroll_evicted ? 0 : streak;
	data.GetProbe_residency()[buffer_index] = needed ? 1 : 0;
	data.GetPending()[buffer_index] = 0;

	if (scroll_evicted)
	{
		// Zero this probe's own atlas cell (all texel_size^2 texels) so its
		// next activation -- whenever a screen ray next hits near it -- ramps
		// in from a clean history instead of DDGIProbeConvolve's temporal
		// blend mixing fresh light with the previous tenant's unrelated one
		// (see this struct's own comment, ddgi.prism).
		uint texel_size = data.GetInfo().GetAtlas_info().x;
		uint2 origin = probes.ddgi_atlas_origin(slot, texel_size);
		uint slice = probes.ddgi_atlas_array_slice(slot.y, data.GetInfo().GetCascade_info().y);
		for (uint ty = 0; ty < texel_size; ty++)
		{
			for (uint tx = 0; tx < texel_size; tx++)
			{
				uint3 texel = uint3(origin + uint2(tx, ty), slice);
				data.GetProbe_irradiance()[texel] = float4(0, 0, 0, 1);
				data.GetProbe_visibility()[texel] = float2(0, 0);
			}
		}
	}

	// Stagger gate: which rotating 1/stagger_k-sized subset of THIS cascade's
	// probes is due for actual retrace this frame (DDGIGraph.cpp's
	// g_ddgi_stagger_k/g_ddgi_stagger_bucket, mirrored here via
	// DDGIInfo::rays_per_probe.yz -- see that field's own comment, ddgi.prism).
	// Deliberately NOT folded into `needed`/probe_residency above: a probe
	// that's needed but simply hasn't had its turn this frame stays resident
	// (still sampled, still contributing its last traced value) -- only
	// whether it gets RE-COMPACTED for an actual trace/convolve this frame is
	// gated here. stagger_k=1 (staggering off, or this cascade not yet
	// ramped up) always passes: bucket 0 of a 1-sized cycle is every frame.
	uint stagger_k = max(data.GetInfo().GetRays_per_probe().y, 1);
	uint stagger_bucket = data.GetInfo().GetRays_per_probe().z;
	bool due_this_frame = (linear_index % stagger_k) == stagger_bucket;

	if (needed && due_this_frame)
	{
		uint dest_index;
		InterlockedAdd(data.GetCompacted_count()[cascade_index], 1, dest_index);
		data.GetCompacted_list()[probe_offset + dest_index] = linear_index;
	}
}
