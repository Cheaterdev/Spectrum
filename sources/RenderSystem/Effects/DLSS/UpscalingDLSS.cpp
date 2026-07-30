module Graphics:UpscalingDLSS;


import :FrameGraphContext;

import HAL;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;

// D3D12-only for now; Vulkan is WIP and out of scope for Streamline (see
// [[project-streamline-dlss-integration]]) — the #else stub keeps the Vulkan
// build satisfying pass_defaults.h's declaration.
#ifdef HAL_BACKEND_D3D12

namespace
{
	// Streamline's plugins issue their own legacy ResourceBarrier calls on
	// tagged resources internally, requiring legacy/enhanced barrier interop
	// hand-off at COMMON (D3D12 #1350). NOT transition_present() — that
	// requests NO_ACCESS ("handed to the OS"), which is wrong here and trips
	// a DEV-only assert. Sync is COMPUTE_SHADING, not NONE, since these
	// resources are used again downstream.
	const HAL::ResourceState kCommonState{ HAL::BarrierSync::COMPUTE_SHADING, HAL::BarrierAccess::COMMON, HAL::TextureLayout::PRESENT };

	// ResultTextureNew is WRITTEN by SL's internal compute work (unlike the
	// other 3 read-only tags) — forcing it to COMMON tripped D3D12 #1334 on
	// SL's own UAV clears, so it stays in UNORDERED_ACCESS instead.
	const HAL::ResourceState kUnorderedAccessState{ HAL::BarrierSync::COMPUTE_SHADING, HAL::BarrierAccess::UNORDERED_ACCESS, HAL::TextureLayout::UNORDERED_ACCESS };

	// ResultTextureNew is R16G16B16A16_FLOAT (see setup() below) — full HDR,
	// not the roughly-sRGB range SL's non-HDR path assumes.
	constexpr bool kHDR = true;
}

bool PassDefault<Passes::UpscalingDLSS>::setup(
	Passes::UpscalingDLSS::Context& data, TaskBuilder& builder)
{
	// Mirrors FSR.cpp's inverted condition — exactly one of the two producers
	// of ResultTextureNew is active per frame.
	if (!nvidia::DLSS::get().available())
		return false;

	auto& frame = builder.graph->get_context<ViewportInfo>();

	builder.need(data.ResultTexture, ResourceFlags::ComputeRead);

	// ExclusiveRead: this pass transitions these to PRESENT/COMMON for
	// Streamline (see kCommonState), which must not be folded into the
	// shared SRV read-window every other pass uses for them.
	builder.need(data.GBuffer_Depth, ResourceFlags::ComputeRead | ResourceFlags::ExclusiveRead);
	builder.need(data.GBuffer_Speed, ResourceFlags::ComputeRead | ResourceFlags::ExclusiveRead);

	builder.recreate(data.ResultTextureNew,
		{ uint3(frame.upscale_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 },
		ResourceFlags::UnorderedAccess);

	return true;
}

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
	// into pixel units. Motion-vector convention still unaudited against
	// GBuffer_Speed — see [[project-streamline-dlss-integration]].
	const auto& cb_jitter = cam.camera_cb.current.jitter;
	constants.jitter_offset = {  cb_jitter.x * float(frame.frame_size.x),
	                            -cb_jitter.y * float(frame.frame_size.y) };
	// `constants` is passed by value to DLSS::upscale() below, which calls
	// set_constants() itself alongside set_tag/evaluate.

	HAL::Resource* result_texture      = data.ResultTexture->get_resource();
	HAL::Resource* gbuffer_depth       = data.GBuffer_Depth->get_resource();
	HAL::Resource* gbuffer_speed       = data.GBuffer_Speed->get_resource();
	HAL::Resource* result_texture_new  = data.ResultTextureNew->get_resource();

	context.get_list()->transition(result_texture,     kCommonState);
	context.get_list()->transition(gbuffer_depth,      kCommonState);
	context.get_list()->transition(gbuffer_speed,      kCommonState);
	context.get_list()->transition(result_texture_new, kUnorderedAccessState);

	const nvidia::DLSSMode mode = g_upscaling_dlss_mode;

	// A mode change invalidates DLSS's temporal history.
	static nvidia::DLSSMode last_mode = mode;
	constants.reset = (mode != last_mode);
	last_mode = mode;

	nvidia::DLSS::get().upscale(*context.get_list(), frame_token, constants, mode, kHDR,
	                             context.graph->get_upscaler_viewport(),
	                             result_texture, gbuffer_depth, gbuffer_speed, result_texture_new);
}

#else

bool PassDefault<Passes::UpscalingDLSS>::setup(Passes::UpscalingDLSS::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::UpscalingDLSS>::render(Passes::UpscalingDLSS::Context&, FrameContext&) {}

#endif
