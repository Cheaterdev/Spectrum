module Graphics:NRD_REBLUR_Execute;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Real per-frame REBLUR_DIFFUSE/REBLUR_SPECULAR execution (see
// [[project-nrd-integration]]), replacing the one-shot smoke-test call that
// used to live in RenderSystem::create_singleton(). NRD is the only
// indirect-GI/reflection denoiser under FSR/plain-DLSS -- gated off under
// DLSS-RR (which does its own reconstruction/denoising; see RTXCombine's
// own comment). NRD_GBufferPack (its other input, and now the only place
// that packs radiance+hitdist for NRD -- see its own comment) uses the same
// gate.
// setup() is fully generated (nrd_sig_test.sig's own [SetupCondition]).

void PassDefault<Passes::NRD_REBLUR_Execute>::pre_setup(FrameGraph::Graph& graph)
{
	// Same gate as [SetupCondition] (nrd_sig_test.sig) -- ensure_pools() is a
	// real side effect [SetupCondition] can't express, so it runs here,
	// once per frame before graph.setup(), guarded by the identical
	// condition repeated (this function's whole reason to exist is that the
	// condition and the side effect had to be split apart).
	if (graph.get_context<Table::UpscalerSelectors>().upscaler_type == UpscalerType::DLSSRR ||
	    !graph.get_context<Table::RenderDeviceCapabilities>().rtx_supported ||
	    !graph.get_context<Table::RenderDeviceCapabilities>().dlssrr_available)
		return;

	// Pool textures are sized off the real render resolution, not the
	// placeholder 1920x1080 the old one-shot smoke test used -- idempotent,
	// a no-op once already sized for this resolution.
	auto& frame = graph.get_context<ViewportInfo>();
	nvidia::NRD::get().ensure_pools(RenderSystem::get().device(), frame.frame_size);
}

void PassDefault<Passes::NRD_REBLUR_Execute>::render(
	Passes::NRD_REBLUR_Execute::Context& data, FrameContext& context)
{
	auto& cam_info = context.graph->get_context<CameraInfo>();
	camera* cam = cam_info.cam;

	nvidia::NRDFrameInputs inputs;
	inputs.view_z            = *data.NRD_ViewZ;
	inputs.normal_roughness  = *data.NRD_NormalRoughness;
	inputs.mv                = *data.NRD_Mv;
	inputs.diff_noisy        = *data.NRD_DiffuseRadianceHitDist;
	inputs.diff_denoised     = *data.RTXIndirectDenoised;
	inputs.spec_noisy        = *data.NRD_SpecularRadianceHitDist;
	inputs.spec_denoised     = *data.RTXReflectionDenoised;

	memcpy(inputs.world_to_view,      cam->camera_cb.current.view.elems.data(), sizeof(inputs.world_to_view));
	memcpy(inputs.world_to_view_prev, cam->camera_cb.prev.view.elems.data(),    sizeof(inputs.world_to_view_prev));
	memcpy(inputs.view_to_clip,       cam->camera_cb.current.proj.elems.data(), sizeof(inputs.view_to_clip));
	memcpy(inputs.view_to_clip_prev,  cam->camera_cb.prev.proj.elems.data(),    sizeof(inputs.view_to_clip_prev));
	inputs.jitter      = float2(cam->camera_cb.current.jitter.x, cam->camera_cb.current.jitter.y);
	inputs.jitter_prev = float2(cam->camera_cb.prev.jitter.x, cam->camera_cb.prev.jitter.y);

	nvidia::NRD::get().execute(*context.get_list(), inputs);

	// Debug-view-only unpack (see setup()'s comment) -- decodes REBLUR's
	// packed YCoCg+hitdist output into plain RGB for a true-color preview.
	{
		auto& compute = context.get_list()->get_compute();
		compute.set_pipeline<PSOS::NRD_UnpackDebug>();
		Slots::NRD_UnpackDebugParams params;
		params.GetPacked() = data.RTXIndirectDenoised->texture2D;
		params.GetUnpacked() = data.RTXIndirectDenoisedPreview->rwTexture2D;
		compute.set(params);
		compute.dispatch(uint3(context.graph->get_context<ViewportInfo>().frame_size, 1), uint3(8, 8, 1));
	}
}
