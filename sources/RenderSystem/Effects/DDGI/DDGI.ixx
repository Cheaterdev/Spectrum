// DDGI probe-volume GI (see [[project-ddgi]] planning notes) -- v1 scaffold.
// See ddgi.sig's PassNode doc comments for the pass shape itself.
export module Graphics:DDGI;

import HAL;
import FrameGraph;
import Core;
import :VSM;

// Shared by DDGIGraph.cpp's own passes and IndirectRTX.cpp's per-pixel
// feedback sample (raytracing.hlsl's TraceIndirectDiffuse) -- both need the
// exact same grid_min/probe_spacing/probe_counts/atlas_info every frame, or
// the per-pixel sample would read probe positions inconsistent with where
// DDGIProbeTrace/DDGIProbeConvolve actually wrote them. camera_pos recenters
// the grid every frame (snapped to whole probe-spacing steps) -- every call
// site must pass THIS frame's camera position, not a stale one. cascade_index
// selects which of the DDGI_CascadeCount (5) levels to build -- spacing
// doubles per level (cascade 0 = finest, cascade 4 = coarsest), all centered
// on the same camera_pos; see ddgi.sig's own comment on DDGIInfo::cascade_info
// for the buffer/atlas offsets this also fills in.
export Slots::DDGIInfo ddgi_make_info(float3 camera_pos, uint32_t cascade_index);

// Mirrors DDGIGraph.cpp's own free-standing Variable<bool>s (no owning DDGI
// instance exists yet, see DDGISelectors' own comment, ddgi.sig) into
// Table::DDGISelectors for the generated setups below to read. Must run
// before graph.setup(), same reasoning as VoxelGI::update_frame() -- called
// from main.cpp next to voxel_gi->update_frame(graph).
export void ddgi_update_selectors(FrameGraph::Graph& graph);

// Diagnostic (see [[project-ddgi]] planning notes): "Disable sky fallback"
// -- read by main.cpp when filling the shared FrameInfo (FrameData.sig's
// debugFlags, RTXDebugFlags::DisableSkyFallback bit) each frame, so
// MyMissShader (raytracing.hlsl) can return flat black instead of the sky
// cubemap on a miss. Lives here (a DDGI-motivated toggle) even though its
// effect isn't DDGI-exclusive -- every RayPayload-based ColorPass consumer
// in the engine shares the one miss shader.
export bool ddgi_sky_fallback_disabled();

// [Multiple=5] pass render bodies (ddgi.sig) -- plain free functions, not
// PassDefault<T>::render specializations, because [Multiple] passes are
// runtime-wired via render_funcs[N] arrays, not the [Static] generated-
// specialization path (see PSSM_Cascade/PSSM.ixx's own template ctor, the
// reference for this exact pattern). No per-instance captured state is
// needed -- each call reads its own cascade index from data.pass_index --
// so the same function is assigned into every array slot.
export void ddgi_probe_select_render(Passes::DDGIProbeSelect::Context& data, FrameGraph::FrameContext& context);
export void ddgi_probe_residency_mark_render(Passes::DDGIProbeResidencyMark::Context& data, FrameGraph::FrameContext& context);
export void ddgi_probe_dispatch_args_build_render(Passes::DDGIProbeDispatchArgsBuild::Context& data, FrameGraph::FrameContext& context);
export void ddgi_probe_trace_render(Passes::DDGIProbeTrace::Context& data, FrameGraph::FrameContext& context, const VSM& vsm);
export void ddgi_probe_convolve_render(Passes::DDGIProbeConvolve::Context& data, FrameGraph::FrameContext& context);

// Called once at startup (main.cpp, next to VoxelGI's own construction) to
// wire the three [Multiple=5] passes' render_funcs arrays. Templated on the
// concrete Pipeline type for the same reason VoxelGI/PSSM's own registration
// constructors are (that type is only known in main.cpp's own translation
// unit, via the generated pipeline header it includes).
// `vsm` must outlive the pipeline: DDGIProbeTrace fills its
// VSMShadowLookupData from it every frame.
export template<typename TPipeline>
void ddgi_register_passes(TPipeline& pipeline, const VSM& vsm)
{
	for (uint32_t i = 0; i < 5; i++)
	{
		pipeline.dDGIProbeSelect.render_funcs[i]             = ddgi_probe_select_render;
		pipeline.dDGIProbeResidencyMark.render_funcs[i]      = ddgi_probe_residency_mark_render;
		pipeline.dDGIProbeDispatchArgsBuild.render_funcs[i]  = ddgi_probe_dispatch_args_build_render;
		pipeline.dDGIProbeTrace.render_funcs[i]              = [&vsm](Passes::DDGIProbeTrace::Context& data, FrameGraph::FrameContext& context)
		{
			ddgi_probe_trace_render(data, context, vsm);
		};
		pipeline.dDGIProbeConvolve.render_funcs[i]           = ddgi_probe_convolve_render;
	}
}
