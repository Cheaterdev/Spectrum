module Graphics:UpscalingDLSSRR;

import :FrameGraphContext;
import :UpscalingDLSS;

import RenderSystem;
import HAL;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;

namespace
{
	constexpr bool kHDR = true;
}

// setup() is fully generated (UpscalingDLSSRR.sig's own [SetupCondition]).
// is_rtx_supported(): RTXReflectionNoise only exists when ReflectionRTX
// (voxel.sig) ran this frame, which shares this exact same prerequisite
// (RTX support + DLSSRR::available()) -- keeping both gates identical is
// what guarantees the resource is there whenever this pass needs it.

void PassDefault<Passes::UpscalingDLSSRR>::render(
	Passes::UpscalingDLSSRR::Context& data, FrameContext& context)
{
	auto& frame = context.graph->get_context<ViewportInfo>();
	auto& sl    = nvidia::Streamline::get();

	auto frame_token = sl.current_frame();
	if (!frame_token.valid()) return;

	auto& cam = *context.graph->get_context<CameraInfo>().cam;

	// Same construction as UpscalingDLSS.cpp's render() — see its comments
	// for why these specific matrices/signs.
	nvidia::FrameConstants constants{};
	constants.view_to_clip = cam.get_proj();
	constants.clip_to_view = cam.get_proj();
	constants.clip_to_view.inverse();
	{
		mat4x4 cur_view_proj_unjittered = cam.get_view() * cam.get_proj();
		mat4x4 inv_cur_view_proj_unjittered = cur_view_proj_unjittered;
		inv_cur_view_proj_unjittered.inverse();
		constants.clip_to_prev_clip = inv_cur_view_proj_unjittered * cam.get_prev_view_proj_unjittered();
	}
	const auto& cb_jitter = cam.camera_cb.current.jitter;
	constants.jitter_offset = {  cb_jitter.x * float(frame.frame_size.x),
	                            -cb_jitter.y * float(frame.frame_size.y) };

	HAL::Resource* result_texture      = data.ResultTextureRTXNoise->get_resource();
	HAL::Resource* gbuffer_depth       = data.GBuffer_Depth->get_resource();
	HAL::Resource* gbuffer_speed       = data.GBuffer_Speed->get_resource();
	HAL::Resource* gbuffer_albedo      = data.GBuffer_Albedo->get_resource();
	HAL::Resource* normal_roughness    = data.NormalRoughness->get_resource();
	HAL::Resource* specular_albedo     = data.SpecularAlbedo->get_resource();
	HAL::Resource* specular_hit_dist   = data.RTXReflectionNoise->get_resource();
	HAL::Resource* result_texture_new  = data.ResultTextureNew->get_resource();

	const nvidia::DLSSMode mode = g_upscaling_dlss_mode;

	static nvidia::DLSSMode last_mode = mode;
	constants.reset = (mode != last_mode);
	last_mode = mode;

	nvidia::DLSSRR::get().denoise(*context.get_list(), frame_token, constants,
	                               cam.get_view(), mode, kHDR, context.graph->get_upscaler_viewport(),
	                               result_texture, gbuffer_depth, gbuffer_speed, result_texture_new,
	                               gbuffer_albedo, normal_roughness, specular_hit_dist, specular_albedo);
}

