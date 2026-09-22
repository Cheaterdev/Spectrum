module Graphics:DDGI;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

namespace
{
	// No owning DDGI instance exists yet (see DDGISelectors' own comment,
	// ddgi.sig), so this is a free-standing Meyer's-singleton VariableContext,
	// same pattern GBufferDownsampler's own g_roughness_threshold/
	// g_metallic_threshold use (VoxelGIGraph.cpp).
	VariableContext& ddgi_debug_context()
	{
		static auto ctx = VariableContext::create(L"DDGI");
		return *ctx;
	}

	Variable<bool>  g_ddgi_show_probes   = { false, "Show probes", &ddgi_debug_context() };
	// Cascade 0's spacing -- each further cascade doubles it (5 levels
	// total, DDGI_CascadeCount), same clipmap-style doubling AC Shadows'
	// own 5 cascades use, so there are more, denser probes near the camera
	// and progressively coarser/wider coverage further out instead of one
	// fixed-density grid that's either too coarse nearby or too small
	// overall.
	Variable<float> g_ddgi_probe_spacing = { 4.0f, "Probe spacing (cascade 0)", &ddgi_debug_context(), 0.5f, 20.0f };
	// Master on/off for whether the probe-volume term actually gets added
	// anywhere it's sampled (DDGIProbeTrace's own self-feedback AND
	// TraceIndirectDiffuse's per-pixel term) -- see DDGIInfo::flags' own
	// comment (ddgi.sig). DDGI keeps tracing/convolving either way, so
	// toggling this back on doesn't need to reconverge from cold; it just
	// excludes/includes the contribution from actual lighting, for an A/B
	// comparison against plain 1-bounce RTX.
	Variable<bool> g_ddgi_use_fallback = { true, "Use probe fallback", &ddgi_debug_context() };
	// A/B toggle: real GPU-driven ExecuteIndirect(DISPATCH_RAYS) (raytracing.sig's
	// DispatchRaysArguments/DispatchRaysArgsBuild) vs. the original CPU-recorded
	// fixed-size dispatch_rays. v1 wiring makes both launch the identical
	// Width/Height (this cascade's full atlas), so toggling this should be a
	// no-op visually -- it only proves the ExecuteIndirect path itself is
	// correct before Width/Height get wired to a real residency-compacted
	// count (stream compaction, not yet implemented -- see [[project-ddgi]]
	// planning notes).
	Variable<bool> g_ddgi_use_indirect_dispatch = { true, "Use indirect DispatchRays", &ddgi_debug_context() };
	// Skips DDGIProbeTrace and DDGIProbeConvolve entirely -- no probe ray
	// dispatch at all, atlases keep whatever they last held. Isolates whether
	// a GPU hang comes from probe tracing or from something else in the frame.
	Variable<bool> g_ddgi_freeze_probes = { false, "Freeze probe updates", &ddgi_debug_context() };	// Off by default: the coarsest cascade (DDGI_CascadeCount-1) is normally
	// exempt from residency culling -- always fully resident, since it's the
	// floor everything else falls back to and there's nowhere further for
	// IT to fall back to. Since the 4x/dimension probe density bump it's the
	// same size as every other cascade (131,072 probes) and never shrinks no
	// matter how good hit-point culling gets elsewhere, so this exists to
	// actually observe the cost/quality tradeoff of culling it too. Real
	// risk turning it on: ddgi_sample_irradiance_cascaded's fallback-to-
	// coarsest path (ddgi_sample.hlsl) has no residency check of its own
	// yet, so a probe this drops can be sampled back stale/never-traced
	// wherever nothing else in the scene is marking it -- watch for that
	// with DDGI's own "Show probes" debug view before trusting this in a
	// real scene.
	Variable<bool> g_ddgi_cull_coarsest_cascade = { true, "Cull coarsest cascade", &ddgi_debug_context() };
	// How many CONSECUTIVE frames a resident probe can go unhit before
	// DDGIProbeResidencyMark actually evicts it (DDGI_ProbeMissStreak) --
	// see that shader's own comment. At 0, a single frame's worth of
	// screen-ray noise/occlusion flicker was enough to visibly flip probes
	// in and out of residency even with no real change in what's needed.
	Variable<int> g_ddgi_eviction_grace_frames = { 6, "Eviction grace (frames)", &ddgi_debug_context(), 0, 60 };
	// Master on/off for residency culling as a whole, independent of the
	// coarsest-cascade toggle above -- off means every probe traces/
	// convolves every frame regardless of pending marks (the original
	// pre-culling behavior), for an A/B comparison of full density against
	// culled.
	Variable<bool> g_ddgi_enable_residency_culling = { true, "Enable residency culling", &ddgi_debug_context() };
	// See ddgi_probe_trace.hlsl's own comment: without this, a probe's
	// traced directions are byte-for-byte identical every frame it's
	// resident, so a single ray grazing past geometry into the sky is a
	// permanent bias rather than noise the multi-bounce feedback loop can
	// average away over time.
	Variable<bool> g_ddgi_jitter_rays = { true, "Jitter probe rays", &ddgi_debug_context() };
	// Diagnostic: independent of "Use probe fallback" above (which also
	// gates TraceIndirectDiffuse's own final per-pixel screen term), this
	// one only stops DDGIProbeTrace's own multi-bounce self-feedback sample
	// while probes are actually being (re)traced -- isolates whether a leak
	// comes from direct-lit shading alone or is introduced/amplified by the
	// recursive probe-to-probe feedback loop across frames.
	Variable<bool> g_ddgi_fallback_while_generating = { false, "Use fallback while generating probes", &ddgi_debug_context() };
	// See ddgi_sky_fallback_disabled's own comment (DDGI.ixx) -- affects
	// every RayPayload-based ColorPass consumer in the engine, not just
	// DDGI, since they all share one miss shader (MyMissShader,
	// raytracing.hlsl).
	Variable<bool> g_ddgi_disable_sky_fallback = { false, "Disable sky fallback (all RTX)", &ddgi_debug_context() };
	// Scales DDGIProbeTrace's own multi-bounce self-feedback term (see
	// DDGIInfo::probe_spacing.w's own comment, ddgi.sig) -- turns two probes
	// feeding each other's irradiance back across frames into a dial rather
	// than an all-or-nothing switch (that's "Use fallback while generating
	// probes" above). 1.0 = unscaled (the original behavior); lower values
	// damp how strongly that loop reinforces itself per hop, on top of the
	// existing albedo attenuation. Only affects DDGIProbeTrace's own read --
	// TraceIndirectDiffuse/TraceReflection's final per-pixel screen term is
	// untouched.
	Variable<float> g_ddgi_feedback_strength = { 1.0f, "Feedback strength", &ddgi_debug_context(), 0.0f, 2.0f };
	// Which occlusion test ddgi_sample_irradiance (ddgi_sample.hlsl) uses per
	// trilinear probe corner -- see DDGIOcclusionMode's own comment (ddgi.sig)
	// for what each value does. Enum-typed Variable renders as a combo box
	// (ParameterWindow.ixx's magic_enum-driven dropdown, same as any other
	// enum Variable in this codebase) -- no separate bool toggles needed.
	// RTXRay default matches this session's own prior behavior (every real
	// call site always used a traced visibility ray before this toggle
	// existed).
	Variable<DDGIOcclusionMode> g_ddgi_occlusion_mode = { DDGIOcclusionMode::RTXRay, "Occlusion test", &ddgi_debug_context() };
	// See DDGIInfo::grid_min.w's own comment (ddgi.sig) -- fraction of this
	// cascade's own probe spacing added on top of a probe's stored hit
	// distance before ddgi_probe_depth_test (ddgi_sample.hlsl,
	// DDGIOcclusionMode::ProbeDepthTest) calls a shading point occluded.
	// Too small and the depth test self-occludes on the very surface the
	// probe itself measured (the classic shadow-acne failure mode); too
	// large and it stops rejecting real occluders. 0.05 is a starting point,
	// not a measured value -- tune visually.
	Variable<float> g_ddgi_depth_test_bias = { 0.05f, "Depth test bias (x spacing)", &ddgi_debug_context(), 0.0f, 1.0f };

	// Amortize probe refresh over time instead of retracing every cascade
	// every frame: cascade N refreshes over base^N frames, so cascade 0 stays
	// per-frame and the coarse (largest, least locally relevant) levels cost a
	// fraction of what they used to.
	//
	// NOT an all-or-nothing per-cascade skip (that was the original v1 of
	// this -- see the plan/git history): a cascade either firing its FULL
	// probe count on its one "due" frame or nothing at all on the other
	// base^N-1 frames is exactly the frame-time SPIKE this exists to avoid,
	// it just moves the spike from "every frame" to "every base^N frames"
	// instead of removing it. Rotating bucket instead: cascade N's own
	// probe_count is split into base^N equal buckets by `linear_index %
	// stagger_k`, and exactly ONE bucket is due each frame (`frame %
	// stagger_k`, cycling through all of them over stagger_k frames) -- so
	// every cascade retraces some (~1/stagger_k) of its own probes on EVERY
	// frame, and the same total work happens over the same base^N-frame
	// window, just spread flat instead of bunched onto one frame in every
	// base^N. Consumed by DDGIProbeResidencyMark's own compaction step
	// (ddgi_probe_residency_mark.hlsl) -- a probe can be `needed` (resident,
	// sampled, kept alive) but not `due` this exact frame, in which case it
	// keeps contributing its last traced value and simply waits its turn
	// instead of being evicted or read as stale.
	Variable<bool> g_ddgi_stagger_updates = { true, "Stagger cascade updates", &ddgi_debug_context() };
	// Interval base: 2 gives 1, 2, 4, 8, 16 buckets (frames to fully cycle)
	// for cascades 0..4. 1 is every probe every frame, i.e. the toggle off.
	// Cascade 0 is exempt from this (always k=1 here) regardless of the
	// setting -- see g_ddgi_stagger_cascade0's own comment for why it gets a
	// separate control instead of just folding it into base^0.
	Variable<int>  g_ddgi_stagger_base = { 8, "Stagger interval base", &ddgi_debug_context(), 1, 8 };

	// Cascade 0 (finest spacing, densest and most frequently relevant probe
	// set) is the single most expensive cascade to retrace -- 3-4ms/frame
	// observed, dwarfing the coarser cascades the base^cascade progression
	// above already staggers. It's deliberately excluded from that
	// progression (base^0 is always 1, i.e. never staggered) because it was
	// designed around "cascade 0 stays fully live, only the coarse levels
	// amortize" -- but that assumption is exactly what's being revisited
	// here, so it gets its OWN toggle/interval instead of just changing what
	// base^0 means (which would force it to inherit the same cycle length as
	// whatever's tuned for the coarse cascades, which have very different
	// cost/relevance characteristics).
	Variable<bool> g_ddgi_stagger_cascade0 = { false, "Stagger level 0 too", &ddgi_debug_context() };
	// Cycle length in frames -- e.g. 4 means 1/4 of cascade 0's probes
	// retrace each frame, cycling through all of them every 4 frames.
	Variable<int>  g_ddgi_cascade0_stagger_k = { 4, "Level 0 stagger interval (frames)", &ddgi_debug_context(), 1, 32 };

	// This frame's per-cascade (stagger_k, stagger_bucket) -- computed once
	// in ddgi_update_selectors (before any pass runs) rather than per render
	// call, so every pass of one cascade agrees on it. Mirrored into
	// DDGIInfo::rays_per_probe.yz (ddgi_make_info) -- see that field's own
	// comment (ddgi.sig) for why those two lanes were free to reuse.
	uint32_t g_ddgi_stagger_k[Constants::DDGI_CascadeCount]      = { 1, 1, 1, 1, 1 };
	uint32_t g_ddgi_stagger_bucket[Constants::DDGI_CascadeCount] = { 0, 0, 0, 0, 0 };

	// Toroidal-scroll tracking (see [[project-ddgi]] planning notes and
	// DDGIProbeResidencyMarkData's own comment, ddgi.sig): the grid's window
	// origin (in integer probe-cell units) remembered per cascade across
	// frames, so ddgi_probe_select_render -- which runs once per cascade per
	// frame, first in the pipeline among DDGI's own passes -- can compute
	// this frame's scroll delta exactly once and hand the result to
	// ddgi_probe_residency_mark_render (which runs right after it for the
	// same cascade). ddgi_make_info() itself can't own this: it's called
	// fresh, independently, several times per frame per cascade (Select,
	// ResidencyMark, ArgsBuild, Trace, Convolve, IndirectRTX, ReflectionRTX
	// all call it), so tracking "the previous frame's origin" inside it would
	// treat each of those calls as its own frame.
	ivec3 g_ddgi_prev_window_origin[Constants::DDGI_CascadeCount];
	bool  g_ddgi_scroll_initialized[Constants::DDGI_CascadeCount] = { false, false, false, false, false };
	// This frame's per-cascade eviction range, in the same (lo, count) shape
	// DDGIProbeResidencyMarkData carries to the shader -- see that struct's
	// own comment for the wrap-test derivation. count=0 on every axis when
	// the window didn't move on that axis (or hasn't been established yet).
	ivec3 g_ddgi_scroll_lo[Constants::DDGI_CascadeCount];
	uint3 g_ddgi_scroll_count[Constants::DDGI_CascadeCount];
}

void ddgi_update_selectors(FrameGraph::Graph& graph)
{
	graph.get_context<Table::DDGISelectors>().show_probes = g_ddgi_show_probes;

	// Runs once per frame, before setup, so this is the one place a frame
	// counter can live without being bumped several times per frame (every
	// cascade calls into the render functions separately).
	static uint64_t frame = 0;
	++frame;

	const uint32_t base = (uint32_t)std::max(1, (int)g_ddgi_stagger_base);
	const bool stagger_upper = g_ddgi_stagger_updates && base > 1;

	// k tracks base^cascade as the loop advances (1, base, base^2, ...) --
	// same exponential scaling the old due-mask used for cascades 1-4.
	// Cascade 0 is computed from its own independent toggle/interval instead
	// (see g_ddgi_stagger_cascade0's own comment), so k's value at cascade 0
	// is never actually used for it, only carried forward so cascade 1 still
	// starts its own progression at base^1.
	uint64_t k = 1;
	for (uint32_t cascade = 0; cascade < Constants::DDGI_CascadeCount; ++cascade)
	{
		uint64_t cascade_k = 1;
		if (cascade == 0)
		{
			if (g_ddgi_stagger_cascade0)
				cascade_k = (uint64_t)std::max(1, (int)g_ddgi_cascade0_stagger_k);
		}
		else if (stagger_upper)
		{
			cascade_k = k;
		}

		g_ddgi_stagger_k[cascade]      = (uint32_t)cascade_k;
		g_ddgi_stagger_bucket[cascade] = (uint32_t)(frame % cascade_k);

		k *= base;
	}
}

bool ddgi_sky_fallback_disabled()
{
	return g_ddgi_disable_sky_fallback;
}

namespace
{
	float3 ddgi_camera_pos(FrameContext& context)
	{
		return context.graph->get_context<CameraInfo>().cam->position;
	}
}

// Recenters on the camera every frame, snapped to whole probe_spacing steps
// so the grid doesn't jitter continuously as the camera moves -- only steps
// when the camera crosses a spacing-sized cell boundary. Toroidal (ring-
// buffer) addressing is real now (ddgi_wrap/ddgi_probe_world_pos/
// ddgi_world_to_slot, ddgi.sig): a probe's atlas slot is a fixed function of
// its own absolute world-cell, independent of where this window currently
// sits, so most probes keep their exact stored history across a step instead
// of every probe's slot meaning a different world position each time the
// grid moves. ddgi_probe_select_render (below) tracks the per-cascade scroll
// delta and forces only the newly-scrolled-in slice to evict/retrace fresh
// -- see its own comment for the eviction-range derivation.
// cascade_index's spacing is g_ddgi_probe_spacing * 2^cascade_index (0 =
// finest). Exported (DDGI.ixx) so IndirectRTX.cpp's per-pixel feedback
// sample uses the exact same grids every frame that DDGIProbeTrace/
// DDGIProbeConvolve wrote into. Recomputed per call rather than compiled
// once and bound via a global SlotID (VoxelInfo's own pattern,
// VoxelGIGraph.cpp) because the call sites don't share a single render pass
// to compile it once in.
Slots::DDGIInfo ddgi_make_info(float3 camera_pos, uint32_t cascade_index)
{
	Slots::DDGIInfo info;

	float spacing = g_ddgi_probe_spacing * (float)(1u << cascade_index);
	uint3 counts = uint3(Constants::DDGI_ProbeCountX, Constants::DDGI_ProbeCountY, Constants::DDGI_ProbeCountZ);
	uint32_t probe_count = counts.x * counts.y * counts.z;

	float3 half_extent = float3(counts) * spacing * 0.5f;
	float3 snapped_center = {
		floorf(camera_pos.x / spacing + 0.5f) * spacing,
		floorf(camera_pos.y / spacing + 0.5f) * spacing,
		floorf(camera_pos.z / spacing + 0.5f) * spacing
	};

	info.GetGrid_min().xyz      = snapped_center - half_extent;
	info.GetGrid_min().w       = g_ddgi_depth_test_bias;
	info.GetProbe_spacing().xyz = float3(spacing, spacing, spacing);
	info.GetProbe_spacing().w   = g_ddgi_feedback_strength;
	info.GetProbe_counts().xyz  = counts;
	// The real per-probe ray budget now -- DDGIProbeTrace dispatches exactly
	// this many spherical-fibonacci rays per probe, independent of the
	// output atlas's own texel resolution (see DDGI_ProbeRayCount's own
	// comment, ddgi.sig, for why that decoupling replaced the old 1-ray-
	// per-texel scheme).
	info.GetRays_per_probe().x = Constants::DDGI_ProbeRayCount;
	// This cascade's rotating retrace subset -- see g_ddgi_stagger_k/
	// g_ddgi_stagger_bucket's own comment and rays_per_probe's field comment
	// (ddgi.sig) for what these drive (DDGIProbeResidencyMark's compaction
	// gate).
	info.GetRays_per_probe().y = g_ddgi_stagger_k[cascade_index];
	info.GetRays_per_probe().z = g_ddgi_stagger_bucket[cascade_index];	info.GetAtlas_info().x     = Constants::DDGI_ProbeTexelSize;
	// See DDGIInfo's own comment (ddgi.sig) for what these offsets are.
	info.GetCascade_info().x = cascade_index * probe_count;
	info.GetCascade_info().y = cascade_index * Constants::DDGI_ProbeCountY;
	info.GetCascade_info().z = cascade_index;
	info.GetCascade_info().w = (cascade_index == Constants::DDGI_CascadeCount - 1) ? 1 : 0;
	info.GetFlags().x = g_ddgi_use_fallback ? 1 : 0;
	info.GetFlags().y = g_ddgi_use_indirect_dispatch ? 1 : 0;
	// See DDGIInfo::flags' own comment (ddgi.sig) for the bit layout.
	info.GetFlags().z = (g_ddgi_cull_coarsest_cascade ? (uint32_t)DDGIControlFlags::CullCoarsestCascade : 0u)
		| (g_ddgi_enable_residency_culling ? 0u : (uint32_t)DDGIControlFlags::DisableResidencyCulling)
		| (g_ddgi_jitter_rays ? (uint32_t)DDGIControlFlags::JitterRays : 0u)
		| (g_ddgi_fallback_while_generating ? 0u : (uint32_t)DDGIControlFlags::DisableTraceFeedback)
		| (static_cast<uint32_t>((DDGIOcclusionMode)g_ddgi_occlusion_mode) << 4);
	info.GetFlags().w = static_cast<uint32_t>(g_ddgi_eviction_grace_frames);

	return info;
}

// v1 scaffold: naive round-robin budgeting is deferred (see ddgi.sig's own
// comment) -- stamps every probe's last_full_update_frame to 0 every frame.
// Enough to prove the SIG declarations/FrameGraph wiring compile and run
// cleanly, and to touch DDGI_Probes for real (see
// [[feedback_pso_empty_slots_assert]] for why a truly empty body isn't an
// option). Plain free function, not a PassDefault<T>::render specialization
// -- see DDGI.ixx's own comment on why [Multiple=5] passes are wired this
// way (ddgi_register_passes). setup() is still fully generated (ddgi.sig's
// own [SetupCondition]).
void ddgi_probe_select_render(Passes::DDGIProbeSelect::Context& data, FrameContext& context)
{
	uint32_t cascade = data.pass_index;

	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	Slots::DDGIInfo info = ddgi_make_info(ddgi_camera_pos(context), cascade);

	// Toroidal-scroll delta, computed exactly once per cascade per frame
	// here (see g_ddgi_prev_window_origin's own comment) -- grid_min is
	// always an exact multiple of spacing (ddgi_make_info's own snapping),
	// so dividing back out and rounding recovers the window's origin in
	// whole probe-cell units.
	{
		vec4 grid_min = info.GetGrid_min();
		vec4 spacing  = info.GetProbe_spacing();
		ivec3 window_origin;
		window_origin.x = (int)std::lround(grid_min.x / spacing.x);
		window_origin.y = (int)std::lround(grid_min.y / spacing.y);
		window_origin.z = (int)std::lround(grid_min.z / spacing.z);

		uint4 probe_counts = info.GetProbe_counts();
		ivec3 lo(0, 0, 0);
		uint3 count(0, 0, 0);

		if (g_ddgi_scroll_initialized[cascade])
		{
			ivec3 prev = g_ddgi_prev_window_origin[cascade];
			int delta_x = window_origin.x - prev.x;
			int delta_y = window_origin.y - prev.y;
			int delta_z = window_origin.z - prev.z;

			// See DDGIProbeResidencyMarkData's own comment (ddgi.sig) for
			// the derivation: lo = min(old_origin, new_origin) on this axis,
			// count = |delta| clamped to this axis's own probe count --
			// clamping to the grid size is what turns a large jump
			// (teleport/cut) into "evict everything on this axis" instead
			// of needing a separate full-reset code path (a wrap test with
			// count == counts is unconditionally true for every slot).
			if (delta_x != 0) { lo.x = prev.x + std::min(0, delta_x); count.x = (uint32_t)std::min(std::abs(delta_x), (int)probe_counts.x); }
			if (delta_y != 0) { lo.y = prev.y + std::min(0, delta_y); count.y = (uint32_t)std::min(std::abs(delta_y), (int)probe_counts.y); }
			if (delta_z != 0) { lo.z = prev.z + std::min(0, delta_z); count.z = (uint32_t)std::min(std::abs(delta_z), (int)probe_counts.z); }
		}
		// else: first frame this cascade has ever run -- nothing to evict
		// yet (count stays 0 on every axis), just establish the baseline.

		g_ddgi_scroll_lo[cascade] = lo;
		g_ddgi_scroll_count[cascade] = count;
		g_ddgi_prev_window_origin[cascade] = window_origin;
		g_ddgi_scroll_initialized[cascade] = true;
	}

	{
		Slots::DDGIProbeSelectData params;
		params.GetInfo() = info;
		params.GetProbes().GetProbe_counts() = info.GetProbe_counts();
		params.GetProbes().GetProbes()       = data.DDGI_Probes->rwStructuredBuffer;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIProbeSelect>();
	compute.dispatch(uint3(Constants::DDGI_ProbeCount, 1, 1), uint3(64, 1, 1));
}

// Real hit-point-driven marking with an eviction grace period -- see
// ddgi_probe_residency_mark.hlsl's own comment. Plain free function -- see
// ddgi_probe_select_render's own comment on why.
void ddgi_probe_residency_mark_render(Passes::DDGIProbeResidencyMark::Context& data, FrameContext& context)
{
	uint32_t cascade = data.pass_index;

	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	Slots::DDGIInfo info = ddgi_make_info(ddgi_camera_pos(context), cascade);

	compute.set_pipeline<PSOS::DDGIProbeResidencyMark>();

	// First dispatch: zero this cascade's own compacted-count slot -- see
	// ddgi_probe_residency_mark.hlsl's own comment for why this can't be
	// folded into the marking dispatch below.
	{
		Slots::DDGIProbeResidencyMarkData params;
		params.GetInfo() = info;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->rwStructuredBuffer;
		params.GetCompacted_list()   = data.DDGI_CompactedProbeList->rwStructuredBuffer;
		params.GetCompacted_count()  = data.DDGI_CompactedProbeCount->rwStructuredBuffer;
		params.GetPending()          = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
		params.GetMiss_streak()      = data.DDGI_ProbeMissStreak->rwStructuredBuffer;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->rwTexture2DArray;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->rwTexture2DArray;
		params.GetScroll_lo()    = g_ddgi_scroll_lo[cascade];
		params.GetScroll_count() = g_ddgi_scroll_count[cascade];
		params.GetReset_only() = 1;
		compute.set(params);
		compute.dispatch(1, 1, 1);
	}

	// Second dispatch: mark + stream-compact + toroidal-scroll eviction (see
	// ddgi_probe_select_render's own comment on where scroll_lo/scroll_count
	// come from, and this shader's own comment for how it uses them).
	{
		Slots::DDGIProbeResidencyMarkData params;
		params.GetInfo() = info;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->rwStructuredBuffer;
		params.GetCompacted_list()   = data.DDGI_CompactedProbeList->rwStructuredBuffer;
		params.GetCompacted_count()  = data.DDGI_CompactedProbeCount->rwStructuredBuffer;
		params.GetPending()          = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
		params.GetMiss_streak()      = data.DDGI_ProbeMissStreak->rwStructuredBuffer;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->rwTexture2DArray;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->rwTexture2DArray;
		params.GetScroll_lo()    = g_ddgi_scroll_lo[cascade];
		params.GetScroll_count() = g_ddgi_scroll_count[cascade];
		params.GetReset_only() = 0;
		compute.set(params);
		compute.dispatch(uint3(Constants::DDGI_ProbeCount, 1, 1), uint3(64, 1, 1));
	}
}

// Packs this cascade's DDGI_DispatchRaysArgs record (DispatchRaysArguments,
// raytracing.sig) from the RTXPSO's own shader-table addresses plus this
// cascade's dispatch size, so DDGIProbeTrace's own render() (below) can
// optionally issue a real ExecuteIndirect(DISPATCH_RAYS) instead of a fixed
// dispatch_rays call -- see g_ddgi_use_indirect_dispatch's own comment for
// why both still launch identically for now. Shader-table addresses are
// re-read every frame (cheap: a few resource-address lookups, no upload)
// rather than cached, since materials can hot-reload and rebuild them.
// Plain free function -- see ddgi_probe_select_render's own comment on why.
void ddgi_probe_dispatch_args_build_render(Passes::DDGIProbeDispatchArgsBuild::Context& data, FrameContext& context)
{
	uint32_t cascade = data.pass_index;

	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	auto& rtx = RTX::get().rtx;
	auto split_address = [](HAL::GPUAddressPtr v) {
		return uint2(static_cast<uint>(v & 0xFFFFFFFFull), static_cast<uint>(v >> 32));
	};

	Slots::DispatchRaysArgsBuildData params;
	params.GetHit_addr()    = split_address(rtx.hitgroup_ids->buffer.get_resource_address().get_ptr());
	params.GetHit_stride()  = static_cast<uint>(sizeof(std::remove_cvref_t<decltype(rtx)>::hit_type));
	params.GetHit_count()   = static_cast<uint>(rtx.hitgroup_ids->max_size());
	params.GetMiss_addr()   = split_address(rtx.miss_ids.get_resource_address().get_ptr());
	params.GetMiss_stride() = static_cast<uint>(sizeof(HAL::shader_identifier));
	params.GetMiss_count()  = static_cast<uint>(rtx.miss_ids.get_count());
	// DDGIProbeTrace here names the RaytraceRaygen<> tag type (raytracing.sig),
	// the same unqualified name RTX::get().render<DDGIProbeTrace>() below uses
	// -- not the Passes::DDGIProbeTrace FrameGraph PassNode type.
	params.GetRaygen_addr() = split_address(rtx.raygen_address<DDGIProbeTrace>().get_ptr());
	params.GetRaygen_size() = static_cast<uint>(sizeof(HAL::shader_identifier));
	// Width = DDGI_CompactedProbeCount[cascade] * rays/probe -- a real
	// GPU-computed launch size (DDGIProbeResidencyMark's own stream
	// compaction). Unconditional: staggering no longer zeroes a whole
	// cascade's width on its off frames (see g_ddgi_stagger_k's own
	// comment) -- the compacted count itself already shrinks to only the
	// probes that are both needed AND due this frame, via the same
	// compaction gate that residency culling uses. rays/probe is
	// DDGI_ProbeRayCount now, not DDGI_ProbeTexelSize^2 -- see that
	// constant's own comment (ddgi.sig) for the ray-count/texel-resolution
	// decoupling.
	params.GetWidth_multiplier() = Constants::DDGI_ProbeRayCount;
	params.GetCount_index()      = cascade;
	params.GetCompacted_count()  = data.DDGI_CompactedProbeCount->structuredBuffer;
	params.GetDest_index()  = cascade;
	params.GetArgs()        = data.DDGI_DispatchRaysArgs->rwStructuredBuffer;
	compute.set(params);

	compute.set_pipeline<PSOS::DispatchRaysArgsBuild>();
	compute.dispatch(1, 1, 1);
}

// Traces DDGI_ProbeRayCount rays per selected probe, reusing the same
// ColorPass hit group (MyClosestHitShader) IndirectRTX's own per-pixel GI ray
// uses, and samples DDGI_ProbeIrradiance/DDGI_ProbeVisibility (LAST frame's
// convolved output -- see ddgi_probe_trace.hlsl's own doc comment for why
// reading them here is exactly the multi-bounce feedback mechanism). Retrace
// skip is real now (residency culling + the rotating per-probe stagger gate,
// both in ddgi_probe_trace.hlsl); ray count is now a real, independently
// tunable budget too (DDGI_ProbeRayCount, decoupled from the output atlas's
// own texel resolution -- see that constant's own comment, ddgi.sig). Plain
// free function -- see ddgi_probe_select_render's own comment on why.
void ddgi_probe_trace_render(Passes::DDGIProbeTrace::Context& data, FrameContext& context, const VSM& vsm)
{
	if (g_ddgi_freeze_probes)
		return;

	uint32_t cascade = data.pass_index;

	auto& compute   = context.get_list()->get_compute();
	auto& sceneinfo = context.graph->get_context<SceneInfo>();

	compute.set_signature(RTX::get().rtx.m_root_sig);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	Slots::DDGIInfo info = ddgi_make_info(ddgi_camera_pos(context), cascade);

	{
		Slots::DDGIProbeTraceData params;
		params.GetInfo() = info;
		params.GetProbes().GetProbe_counts() = info.GetProbe_counts();
		params.GetProbes().GetProbes()       = data.DDGI_Probes->rwStructuredBuffer;
		params.GetProbe_ray_radiance() = data.DDGI_ProbeRayRadiance->rwStructuredBuffer;
		params.GetPrev_irradiance() = data.DDGI_ProbeIrradiance->texture2DArray;
		params.GetPrev_visibility() = data.DDGI_ProbeVisibility->texture2DArray;
		params.GetProbe_residency() = data.DDGI_ProbeResidency->structuredBuffer;
		params.GetCompacted_list()  = data.DDGI_CompactedProbeList->structuredBuffer;
		params.GetResidency_pending() = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
		compute.set(params);
	}

	{
		PROFILE(L"ddgi_trace_vsm_lookup");
		Slots::VSMShadowLookupData vsm_data;
		auto& lookup = vsm_data.GetLookup();
		vsm.fill_shadow_lookup_constants(lookup, ddgi_camera_pos(context));
		lookup.GetVsm_atlas()    = data.VSM_Atlas->texture2DArray;
		lookup.GetPage_table()   = data.VSM_PageTable->texture2DArray;
		lookup.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
		compute.set(vsm_data);
	}

	// The coarsest cascade is exempt from residency culling by default (see
	// DDGIControlFlags::CullCoarsestCascade's own comment, ddgi.sig) --
	// unconditionally, 100% of its probes, every frame, so compaction never
	// actually drops anything for it. Confirmed via a temporary CPU readback
	// (ddgi_compacted_count.temp / ddgi_cascade4_residency.temp, ask before
	// removing those diagnostics once this is settled): cascade 4's
	// DDGI_CompactedProbeCount really is the full 131072 every frame, giving
	// an indirect ExecuteIndirect(DISPATCH_RAYS) a single 1D dispatch with
	// Width = DDGI_ProbeCount * DDGI_ProbeRayCount (131072 * 32 = 4,194,304)
	// -- the exact same TOTAL ray count the fixed 3D dispatch below already
	// launches for it (2048x64x32 = 4,194,304) and runs fine, but reshaped
	// into one flat 1D width instead of a 3D shape. That reshaping is the
	// prime suspect for the device-removed/hang seen after fixing the
	// cascade-offset bug above (see exec_indirect's own comment): DXR ray
	// schedulers lean on 2D/3D dispatch tiling for BVH-traversal locality,
	// and a single enormous 1D width is a much less common, apparently much
	// slower path for hardware/driver to take, easily enough to trip the
	// OS's ~2s TDR timeout on identical total work.
	// Since indirect dispatch buys zero benefit here anyway (nothing is ever
	// culled from an always-fully-resident cascade), route it through the
	// same safe fixed 3D dispatch the non-indirect path uses instead of
	// through ExecuteIndirect, regardless of the toggle -- only cascades
	// that can actually have probes culled take the indirect path.
	bool is_coarsest_and_exempt = info.GetCascade_info().w != 0
		&& (info.GetFlags().z & (uint32_t)DDGIControlFlags::CullCoarsestCascade) == 0;

	if (g_ddgi_use_indirect_dispatch && !is_coarsest_and_exempt)
	{
		// Real GPU-driven ExecuteIndirect(DISPATCH_RAYS) -- DDGIProbeDispatchArgsBuild
		// (above) already packed this cascade's DDGI_DispatchRaysArgs record
		// this frame. Mirrors what RTX::render<T>()/RTXPSO::dispatch<T>() do
		// for the fixed-size path: bind the scene, set the DXR state object,
		// then launch -- exec_indirect<DispatchRaysArguments> just reads its
		// dispatch dimensions (and shader-table addresses) from that buffer
		// instead of from template/call-site arguments.
		Slots::Raytracing raytracing;
		raytracing.GetScene() = sceneinfo.scene->raytrace_scene->get_handle();
		compute.set(raytracing);

		compute.set_pipeline(RTX::get().rtx.m_dxrStateObject);
		// `offset` (element index into DDGI_DispatchRaysArgs, one record per
		// cascade -- ddgi.sig's own comment) -- missing this made every
		// cascade read element 0 regardless of which cascade was actually
		// tracing: cascades 1-4 launched with cascade 0's own needed-probe
		// count instead of their own.
		compute.exec_indirect<DispatchRaysArguments>(*data.DDGI_DispatchRaysArgs, 1, cascade,
			&RTX::get().rtx.raygen_slots<DDGIProbeTrace>());
	}
	else
	{
		// 3D, not one flat width -- see this function's own comment above on
		// why a single enormous 1D DispatchRays width is the specific thing
		// to avoid here. X folds in the ray dimension (probe_x*RayCount +
		// ray_index, see ddgi_probe_trace.hlsl's own decode); Y/Z stay real
		// dispatch dimensions the same way DDGI_ProbeCountY already did
		// before rays were their own axis.
		ivec3 dispatch_size = {
			Constants::DDGI_ProbeCountX * Constants::DDGI_ProbeRayCount,
			Constants::DDGI_ProbeCountZ,
			Constants::DDGI_ProbeCountY
		};
		RTX::get().render<DDGIProbeTrace>(compute, sceneinfo.scene->raytrace_scene, dispatch_size);
	}
}

// Cosine-weight-resamples each probe's own fixed DDGI_ProbeRayRadiance ray
// set into its irradiance/visibility atlas texels -- see
// ddgi_probe_convolve.hlsl for the per-texel math. Plain free function --
// see ddgi_probe_select_render's own comment on why.
void ddgi_probe_convolve_render(Passes::DDGIProbeConvolve::Context& data, FrameContext& context)
{
	if (g_ddgi_freeze_probes)
		return;

	uint32_t cascade = data.pass_index;

	// No more cascade-level skip here (see g_ddgi_stagger_k's own comment)
	// -- the dispatch always runs at full size, and each thread's own
	// per-probe due-check (ddgi_probe_convolve.hlsl, alongside its existing
	// residency early-out) decides whether that probe's texels get
	// reconvolved this frame.
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	{
		Slots::DDGIProbeConvolveData params;
		params.GetInfo() = ddgi_make_info(ddgi_camera_pos(context), cascade);
		params.GetProbe_ray_radiance() = data.DDGI_ProbeRayRadiance->structuredBuffer;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->rwTexture2DArray;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->rwTexture2DArray;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->structuredBuffer;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIProbeConvolve>();
	// 3D: the array dimension (this cascade's own DDGI_ProbeCountY slice
	// range) is a real dispatch dimension now, same reasoning as
	// ddgi_probe_trace_render's own fixed-dispatch path.
	compute.dispatch(ivec3(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight, Constants::DDGI_ProbeCountY), ivec3{ 8, 8, 1 });
}

// Debug-only screen-space probe splat (DDGISelectors::show_probes, toggled
// via the "Show probes" Variable<bool> in the Properties panel under "DDGI")
// -- see ddgi_debug.hlsl for the per-probe projection/depth-test/splat math.
// Shows all 5 cascades' probes at once (dispatch size is
// DDGI_ProbeCount*DDGI_CascadeCount; the shader decodes which cascade each
// dispatch index belongs to from the shared per-cascade probe count).
// setup() is fully generated (ddgi.sig's own [SetupCondition]).
void PassDefault<Passes::DDGIDebug>::render(
	Passes::DDGIDebug::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	float3 cam_pos = ddgi_camera_pos(context);
	Slots::DDGIInfo info0 = ddgi_make_info(cam_pos, 0);

	{
		Slots::DDGIDebugData params;
		params.GetCascade0() = info0;
		params.GetCascade1() = ddgi_make_info(cam_pos, 1);
		params.GetCascade2() = ddgi_make_info(cam_pos, 2);
		params.GetCascade3() = ddgi_make_info(cam_pos, 3);
		params.GetCascade4() = ddgi_make_info(cam_pos, 4);
		params.GetProbes().GetProbe_counts()  = info0.GetProbe_counts();
		params.GetProbes().GetProbes()        = data.DDGI_Probes->rwStructuredBuffer;
		params.GetDepth()            = data.GBuffer_DepthMips->texture2D;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->texture2DArray;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->structuredBuffer;
		params.GetTarget()           = data.ResultTexture->rwTexture2D;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIDebug>();
	compute.dispatch(uint3(Constants::DDGI_ProbeCount * Constants::DDGI_CascadeCount, 1, 1), uint3(64, 1, 1));
}

// Full-screen debug view of the exact per-pixel feedback sample
// (FrameGraph::DebugMode::DDGIIndirect, see Base.cpp's debug_source()) --
// see ddgi_indirect_debug.hlsl for the per-pixel math (picks the finest of
// all 5 cascades that actually contains each pixel's world position).
// setup() is fully generated (ddgi.sig's own [SetupCondition]).
void PassDefault<Passes::DDGIIndirectDebug>::render(
	Passes::DDGIIndirectDebug::Context& data, FrameContext& context)
{
	auto& compute   = context.get_list()->get_compute();
	auto& sceneinfo = context.graph->get_context<SceneInfo>();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	// Inline-ray-traced occlusion (DDGIProbeDispatchArgsBuild-style scene
	// binding, see ddgi_sample.hlsl's ddgi_sample_irradiance_cascaded_inline_traced)
	// -- explicitly bound here rather than relying on the RTX-signature
	// passes earlier in the pipeline having left it set, since this pass
	// uses the plain DefaultLayout signature, not RTX::get().rtx.m_root_sig.
	{
		Slots::Raytracing raytracing;
		raytracing.GetScene() = sceneinfo.scene->raytrace_scene->get_handle();
		compute.set(raytracing);
	}

	float3 cam_pos = ddgi_camera_pos(context);

	{
		Slots::DDGIIndirectDebugData params;
		params.GetCascade0()         = ddgi_make_info(cam_pos, 0);
		params.GetCascade1()         = ddgi_make_info(cam_pos, 1);
		params.GetCascade2()         = ddgi_make_info(cam_pos, 2);
		params.GetCascade3()         = ddgi_make_info(cam_pos, 3);
		params.GetCascade4()         = ddgi_make_info(cam_pos, 4);
		params.GetDepth()            = data.GBuffer_DepthMips->texture2D;
		params.GetNormals()          = data.GBuffer_Normals->texture2D;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->texture2DArray;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->texture2DArray;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->structuredBuffer;
		params.GetResidency_pending() = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
		params.GetTarget()           = data.DDGIIndirectDebug->rwTexture2D;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIIndirectDebug>();
	compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 8, 8 });
}
