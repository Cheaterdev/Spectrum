module Graphics:UpscalingDLSS;


import :FrameGraphContext;

import HAL;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;

namespace
{
	// ResultTextureNew is R16G16B16A16_FLOAT (see setup() below) — full HDR,
	// not the roughly-sRGB range SL's non-HDR path assumes.
	constexpr bool kHDR = true;
}

// setup() is fully generated (UpscalingDLSS.prism's own [SetupCondition]).

void PassDefault<Passes::UpscalingDLSS>::render(
	Passes::UpscalingDLSS::Context& data, FrameContext& context)
{
	auto& frame = context.graph->get_context<ViewportInfo>();
	auto& sl    = nvidia::Streamline::get();

	// begin_frame() runs once per frame in main.cpp; read the cached result.
	auto frame_token = sl.current_frame();
	if (!frame_token.valid()) return;

	nvidia::FrameConstants constants{};
	auto& cam = *context.graph->get_context<CameraInfo>().cam;

	// Streamline requires UNJITTERED matrices. get_proj()/get_view() already
	// return the true unjittered originals (only a local copy inside
	// camera::update() carries the jitter).
	constants.view_to_clip = cam.get_proj();
	constants.clip_to_view = cam.get_proj();
	constants.clip_to_view.inverse();

	// clipToPrevClip, all UNJITTERED — camera_cb.prev.viewProj is not usable
	// here (it's the jittered snapshot), hence get_prev_view_proj_unjittered().
	{
		mat4x4 cur_view_proj_unjittered = cam.get_view() * cam.get_proj();
		mat4x4 inv_cur_view_proj_unjittered = cur_view_proj_unjittered;
		inv_cur_view_proj_unjittered.inverse();
		constants.clip_to_prev_clip = inv_cur_view_proj_unjittered * cam.get_prev_view_proj_unjittered();
	}

	// jitter_offset: recovered from camera_cb.current.jitter (UV-space) back
	// into pixel units.
	const auto& cb_jitter = cam.camera_cb.current.jitter;
	constants.jitter_offset = {  cb_jitter.x * float(frame.frame_size.x),
	                            -cb_jitter.y * float(frame.frame_size.y) };
	// `constants` is passed by value to DLSS::upscale() below, which calls
	// set_constants() itself alongside set_tag/evaluate.

	HAL::Resource* result_texture      = data.ResultTexture->get_resource();
	HAL::Resource* gbuffer_depth       = data.GBuffer_Depth->get_resource();
	HAL::Resource* gbuffer_speed       = data.GBuffer_Speed->get_resource();
	HAL::Resource* result_texture_new  = data.ResultTextureNew->get_resource();

	// No transitions here: DLSS::upscale declares the states its tagged
	// resources need, in an operation that wraps the call itself.

	const nvidia::DLSSMode mode = g_upscaling_dlss_mode;

	// A mode change invalidates DLSS's temporal history.
	static nvidia::DLSSMode last_mode = mode;
	constants.reset = (mode != last_mode);
	last_mode = mode;

	nvidia::DLSS::get().upscale(*context.get_list(), frame_token, constants, mode, kHDR,
	                             context.graph->get_upscaler_viewport(),
	                             result_texture, gbuffer_depth, gbuffer_speed, result_texture_new);
}

