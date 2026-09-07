module Graphics:NRD_REBLUR_Execute;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Real per-frame REBLUR_DIFFUSE/REBLUR_SPECULAR execution (see
// [[project-nrd-integration]]), replacing the one-shot smoke-test call that
// used to live in RenderSystem::create_singleton(). Gated purely on
// g_indirect_denoiser/g_reflection_denoiser -- independent of
// g_upscaler_type, so NRD works under FSR/DLSS too, not just DLSS-RR.
// NRD_GBufferPack (its other input) uses the same gate. The two denoisers are
// independent: this pass runs whenever *either* wants NRD, and render() only
// populates the inputs for whichever one(s) actually do (HAL.NRD::execute()
// infers want_diffuse/want_specular from which NRDFrameInputs fields are set).
bool PassDefault<Passes::NRD_REBLUR_Execute>::setup(
	Passes::NRD_REBLUR_Execute::Context& data, TaskBuilder& builder)
{
	bool want_diffuse  = g_indirect_denoiser == IndirectDenoiser::NRD;
	bool want_specular = g_reflection_denoiser == ReflectionDenoiserKind::NRD;
	if ((!want_diffuse && !want_specular) ||
	    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	auto& frame = builder.graph->get_context<ViewportInfo>();
	auto  sz    = frame.frame_size;

	// Pool textures are sized off the real render resolution, not the
	// placeholder 1920x1080 the old one-shot smoke test used -- idempotent,
	// a no-op once already sized for this resolution.
	nvidia::NRD::get().ensure_pools(RenderSystem::get().device(), sz);

	builder.need(data.NRD_ViewZ, ResourceFlags::ComputeRead);
	builder.need(data.NRD_NormalRoughness, ResourceFlags::ComputeRead);
	builder.need(data.NRD_Mv, ResourceFlags::ComputeRead);

	// Only need() the one raw candidate actually selected per signal -- the
	// other candidate's producer (e.g. ReflectionRTX, an entire raytracing
	// dispatch) may not have run at all this frame, and needing an unproduced
	// resource is a hard FrameGraph error (exists() assert), not just wasted
	// work. Same reasoning as RTXCombine's own conditional need() below.
	if (want_diffuse)
	{
		if (g_indirect_source == IndirectSource::MyVCT)
			builder.need(data.VoxelIndirectNoiseRaw, ResourceFlags::ComputeRead);
		else
			builder.need(data.RTXIndirectNoise, ResourceFlags::ComputeRead);
		builder.create(data.RTXIndirectDenoised,
			{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
		// Debug-view-only unpacked preview (see nrd_sig_test.sig's
		// NRD_UnpackDebugParams comment) -- not read by RTXCombine.
		builder.create(data.RTXIndirectDenoisedPreview,
			{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	}
	if (want_specular)
	{
		if (g_reflection_source == ReflectionSource::MyReflection)
			builder.need(data.VoxelReflectionNoiseRaw, ResourceFlags::ComputeRead);
		else
			builder.need(data.RTXReflectionNoise, ResourceFlags::ComputeRead);
		builder.create(data.RTXReflectionDenoised,
			{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	}
	return true;
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

	// Left unset (null resource) when their denoiser isn't NRD this frame --
	// HAL.NRD::execute() infers want_diffuse/want_specular from exactly this,
	// so REBLUR_DIFFUSE/REBLUR_SPECULAR only dispatch when actually selected,
	// even though this pass itself runs whenever *either* wants NRD.
	if (g_indirect_denoiser == IndirectDenoiser::NRD)
	{
		inputs.diff_noisy = g_indirect_source == IndirectSource::MyVCT
		                     ? *data.VoxelIndirectNoiseRaw
		                     : *data.RTXIndirectNoise;
		inputs.diff_denoised = *data.RTXIndirectDenoised;
	}
	if (g_reflection_denoiser == ReflectionDenoiserKind::NRD)
	{
		inputs.spec_noisy = g_reflection_source == ReflectionSource::MyReflection
		                     ? *data.VoxelReflectionNoiseRaw
		                     : *data.RTXReflectionNoise;
		inputs.spec_denoised = *data.RTXReflectionDenoised;
	}

	memcpy(inputs.world_to_view,      cam->camera_cb.current.view.elems.data(), sizeof(inputs.world_to_view));
	memcpy(inputs.world_to_view_prev, cam->camera_cb.prev.view.elems.data(),    sizeof(inputs.world_to_view_prev));
	memcpy(inputs.view_to_clip,       cam->camera_cb.current.proj.elems.data(), sizeof(inputs.view_to_clip));
	memcpy(inputs.view_to_clip_prev,  cam->camera_cb.prev.proj.elems.data(),    sizeof(inputs.view_to_clip_prev));
	inputs.jitter      = float2(cam->camera_cb.current.jitter.x, cam->camera_cb.current.jitter.y);
	inputs.jitter_prev = float2(cam->camera_cb.prev.jitter.x, cam->camera_cb.prev.jitter.y);

	nvidia::NRD::get().execute(*context.get_list(), inputs);

	// Debug-view-only unpack (see setup()'s comment) -- decodes REBLUR's
	// packed YCoCg+hitdist output into plain RGB for a true-color preview.
	// RTXIndirectDenoised/Preview only exist when want_diffuse (see setup()).
	if (g_indirect_denoiser == IndirectDenoiser::NRD)
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
