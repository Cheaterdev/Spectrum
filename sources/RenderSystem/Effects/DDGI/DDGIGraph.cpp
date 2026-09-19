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
	Variable<bool> g_ddgi_use_indirect_dispatch = { false, "Use indirect DispatchRays", &ddgi_debug_context() };
}

void ddgi_update_selectors(FrameGraph::Graph& graph)
{
	graph.get_context<Table::DDGISelectors>().show_probes = g_ddgi_show_probes;
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
// when the camera crosses a spacing-sized cell boundary. No toroidal
// wraparound / relight-only-the-new-ring preservation yet (deferred, see
// ddgi.sig's DDGIProbes comment on ddgi_toroidal_wrap): when the grid steps,
// every probe just retraces fresh next frame, which v1 already does anyway
// (every probe retraces every frame regardless of whether the grid moved).
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
	info.GetProbe_spacing().xyz = float3(spacing, spacing, spacing);
	info.GetProbe_counts().xyz  = counts;
	// Informational only in v1 -- DDGIProbeTrace currently dispatches one
	// ray per radiance-atlas texel instead of a fixed per-probe ray count
	// (see its own doc comment).
	info.GetRays_per_probe().x = Constants::DDGI_ProbeTexelSize * Constants::DDGI_ProbeTexelSize;
	info.GetAtlas_info().x     = Constants::DDGI_ProbeTexelSize;
	// See DDGIInfo's own comment (ddgi.sig) for what these offsets are.
	info.GetCascade_info().x = cascade_index * probe_count;
	info.GetCascade_info().y = cascade_index * Constants::DDGI_AtlasWidth;
	info.GetCascade_info().z = cascade_index;
	info.GetCascade_info().w = (cascade_index == Constants::DDGI_CascadeCount - 1) ? 1 : 0;
	info.GetFlags().x = g_ddgi_use_fallback ? 1 : 0;
	info.GetFlags().y = g_ddgi_use_indirect_dispatch ? 1 : 0;

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

// v1 placeholder (see [[project-ddgi]] planning notes): marks every probe
// needed unconditionally -- proves the residency buffer's create/read/write
// plumbing before the real hit-point-driven marking + neighbor dilation
// replaces this body. Plain free function -- see ddgi_probe_select_render's
// own comment on why.
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
		params.GetReset_only() = 1;
		compute.set(params);
		compute.dispatch(1, 1, 1);
	}

	// Second dispatch: mark + stream-compact.
	{
		Slots::DDGIProbeResidencyMarkData params;
		params.GetInfo() = info;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->rwStructuredBuffer;
		params.GetCompacted_list()   = data.DDGI_CompactedProbeList->rwStructuredBuffer;
		params.GetCompacted_count()  = data.DDGI_CompactedProbeCount->rwStructuredBuffer;
		params.GetPending()          = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
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
	// Width = DDGI_CompactedProbeCount[cascade] * texels/probe -- a real
	// GPU-computed launch size (DDGIProbeResidencyMark's own stream
	// compaction), not a CPU literal, though numerically identical to the
	// old fixed atlas size today since residency marking is still the
	// "mark everything" placeholder (see that PassNode's own comment).
	params.GetWidth_multiplier() = Constants::DDGI_ProbeTexelSize * Constants::DDGI_ProbeTexelSize;
	params.GetCount_index()      = cascade;
	params.GetCompacted_count()  = data.DDGI_CompactedProbeCount->structuredBuffer;
	params.GetDest_index()  = cascade;
	params.GetArgs()        = data.DDGI_DispatchRaysArgs->rwStructuredBuffer;
	compute.set(params);

	compute.set_pipeline<PSOS::DispatchRaysArgsBuild>();
	compute.dispatch(1, 1, 1);
}

// Traces one ray per DDGI_ProbeRadiance/DDGI_ProbeGBuffer atlas texel,
// reusing the same ColorPass hit group (MyClosestHitShader) IndirectRTX's
// own per-pixel GI ray uses, and samples DDGI_ProbeIrradiance/
// DDGI_ProbeVisibility (LAST frame's convolved output -- see
// ddgi_probe_trace.hlsl's own doc comment for why reading them here is
// exactly the multi-bounce feedback mechanism). Still deferred: real
// per-probe ray budget, retrace skip. Plain free function -- see
// ddgi_probe_select_render's own comment on why.
void ddgi_probe_trace_render(Passes::DDGIProbeTrace::Context& data, FrameContext& context)
{
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
		params.GetProbe_radiance()  = data.DDGI_ProbeRadiance->rwTexture2D;
		params.GetProbe_gbuffer()   = data.DDGI_ProbeGBuffer->rwTexture2D;
		params.GetPrev_irradiance() = data.DDGI_ProbeIrradiance->texture2D;
		params.GetPrev_visibility() = data.DDGI_ProbeVisibility->texture2D;
		params.GetProbe_residency() = data.DDGI_ProbeResidency->rwStructuredBuffer;
		params.GetCompacted_list()  = data.DDGI_CompactedProbeList->structuredBuffer;
		compute.set(params);
	}

	if (g_ddgi_use_indirect_dispatch)
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
		compute.exec_indirect<DispatchRaysArguments>(*data.DDGI_DispatchRaysArgs, 1);
	}
	else
	{
		ivec2 atlas_size = { Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight };
		RTX::get().render<DDGIProbeTrace>(compute, sceneinfo.scene->raytrace_scene, atlas_size);
	}
}

// Cosine-weighted convolution of each probe's own traced radiance into its
// irradiance/visibility atlas texels -- see ddgi_probe_convolve.hlsl for the
// per-texel math. Plain free function -- see ddgi_probe_select_render's own
// comment on why.
void ddgi_probe_convolve_render(Passes::DDGIProbeConvolve::Context& data, FrameContext& context)
{
	uint32_t cascade = data.pass_index;

	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	{
		Slots::DDGIProbeConvolveData params;
		params.GetInfo() = ddgi_make_info(ddgi_camera_pos(context), cascade);
		params.GetProbe_radiance()   = data.DDGI_ProbeRadiance->texture2D;
		params.GetProbe_gbuffer()    = data.DDGI_ProbeGBuffer->texture2D;
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->rwTexture2D;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->rwTexture2D;
		params.GetProbe_residency()  = data.DDGI_ProbeResidency->rwStructuredBuffer;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIProbeConvolve>();
	compute.dispatch(ivec2(Constants::DDGI_AtlasWidth, Constants::DDGI_AtlasHeight), ivec2{ 8, 8 });
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
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->texture2D;
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
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

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
		params.GetProbe_irradiance() = data.DDGI_ProbeIrradiance->texture2D;
		params.GetProbe_visibility() = data.DDGI_ProbeVisibility->texture2D;
		params.GetTarget()           = data.DDGIIndirectDebug->rwTexture2D;
		compute.set(params);
	}

	compute.set_pipeline<PSOS::DDGIIndirectDebug>();
	compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 8, 8 });
}
