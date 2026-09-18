module Graphics:NRD_SIGMA_Execute;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Real per-frame SIGMA_SHADOW execution (see [[project-nrd-integration]]) --
// denoises whichever raw penumbra signal matches the selected shadow source
// (ShadowRTX's, or VSM_ShadowResolve's own -- diagnostic-only for the VSM
// case, see nrd_sig_test.sig's own PassNode comment), same shape as
// NRD_REBLUR_Execute.cpp but against nvidia::NRD's separate sigma_instance
// (HAL.NRD.ixx's own comment on why REBLUR and SIGMA are two independent
// nrd::Instance objects, not one shared one).
// setup() is fully generated (nrd_sig_test.sig's own [SetupCondition]).

void PassDefault<Passes::NRD_SIGMA_Execute>::pre_setup(FrameGraph::Graph& graph)
{
	// Same gate as [SetupCondition] (nrd_sig_test.sig) -- ensure_pools() is a
	// real side effect [SetupCondition] can't express, so it runs here, once
	// per frame before graph.setup(), guarded by the identical condition
	// repeated, same reasoning as NRD_REBLUR_Execute's own pre_setup().
	auto shadow_source = graph.get_context<Table::VSMSelectors>().shadow_source;
	bool wants_sigma = shadow_source == ShadowSource::RTXReference
		|| (shadow_source == ShadowSource::VSM && graph.get_context<Table::VSMSelectors>().use_vsm_penumbra);

	if (graph.get_context<Table::UpscalerSelectors>().upscaler_type == UpscalerType::DLSSRR ||
	    !graph.get_context<Table::RenderDeviceCapabilities>().rtx_supported ||
	    !graph.get_context<Table::RenderDeviceCapabilities>().dlssrr_available ||
	    !wants_sigma)
		return;

	auto& frame = graph.get_context<ViewportInfo>();
	nvidia::NRD::get().ensure_pools(RenderSystem::get().device(), frame.frame_size);
}

void PassDefault<Passes::NRD_SIGMA_Execute>::render(
	Passes::NRD_SIGMA_Execute::Context& data, FrameContext& context)
{
	auto& cam_info = context.graph->get_context<CameraInfo>();
	camera* cam = cam_info.cam;

	nvidia::NRDFrameInputs inputs;
	inputs.view_z            = *data.NRD_ViewZ;
	inputs.normal_roughness  = *data.NRD_NormalRoughness;
	inputs.mv                = *data.NRD_Mv;
	// Exactly one of these is linked this frame (see nrd_sig_test.sig's own
	// [Optional] guards on both fields, keyed off the same shadow_source).
	inputs.penumbra_noisy    = context.graph->get_context<Table::VSMSelectors>().shadow_source == ShadowSource::VSM
		? *data.VSM_PCSS_ShadowNoise
		: *data.VSM_ShadowNoise;
	inputs.shadow_denoised   = *data.VSM_ShadowDenoised;
	inputs.sun_direction     = context.graph->get_context<SkyInfo>().sunDir;

	memcpy(inputs.world_to_view,      cam->camera_cb.current.view.elems.data(), sizeof(inputs.world_to_view));
	memcpy(inputs.world_to_view_prev, cam->camera_cb.prev.view.elems.data(),    sizeof(inputs.world_to_view_prev));
	memcpy(inputs.view_to_clip,       cam->camera_cb.current.proj.elems.data(), sizeof(inputs.view_to_clip));
	memcpy(inputs.view_to_clip_prev,  cam->camera_cb.prev.proj.elems.data(),    sizeof(inputs.view_to_clip_prev));
	inputs.jitter      = float2(cam->camera_cb.current.jitter.x, cam->camera_cb.current.jitter.y);
	inputs.jitter_prev = float2(cam->camera_cb.prev.jitter.x, cam->camera_cb.prev.jitter.y);

	nvidia::NRD::get().execute_shadow(*context.get_list(), inputs);
}
