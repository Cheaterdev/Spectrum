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
// used to live in RenderSystem::create_singleton(). NRD is the only
// indirect-GI/reflection denoiser now -- gated purely on RTX/hardware
// support, independent of g_upscaler_type. NRD_GBufferPack (its other input)
// uses the same gate.
bool PassDefault<Passes::NRD_REBLUR_Execute>::setup(
	Passes::NRD_REBLUR_Execute::Context& data, TaskBuilder& builder)
{
	if (!RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
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
	// Both raw candidates per signal are needed unconditionally -- their
	// producers (IndirectRTX/VoxelScreen, ReflectionRTX/ScreenReflection)
	// are unconditional passes themselves, so there's no "only need the
	// selected one" complexity here (see g_indirect_source/g_reflection_source
	// selection in render() below, [[project-nrd-integration]]).
	if( g_indirect_source != IndirectSource::MyVCT) builder.need(data.RTXIndirectNoise, ResourceFlags::ComputeRead);
	if( g_reflection_source != ReflectionSource::MyReflection)builder.need(data.RTXReflectionNoise, ResourceFlags::ComputeRead);
	if( g_indirect_source == IndirectSource::MyVCT) builder.need(data.VoxelIndirectNoiseRaw, ResourceFlags::ComputeRead);
	if( g_reflection_source == ReflectionSource::MyReflection)builder.need(data.VoxelReflectionNoiseRaw, ResourceFlags::ComputeRead);
	builder.create(data.RTXIndirectDenoised,
		{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	builder.create(data.RTXReflectionDenoised,
		{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	// Debug-view-only unpacked preview (see nrd_sig_test.sig's
	// NRD_UnpackDebugParams comment) -- not read by RTXCombine.
	builder.create(data.RTXIndirectDenoisedPreview,
		{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
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
	inputs.diff_noisy        = g_indirect_source == IndirectSource::MyVCT
	                              ? *data.VoxelIndirectNoiseRaw : *data.RTXIndirectNoise;
	inputs.diff_denoised     = *data.RTXIndirectDenoised;
	inputs.spec_noisy        = g_reflection_source == ReflectionSource::MyReflection
	                              ? *data.VoxelReflectionNoiseRaw : *data.RTXReflectionNoise;
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
