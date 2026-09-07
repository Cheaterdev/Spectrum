module Graphics:NRD_GBufferPack;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Front-end packing for NRD REBLUR_DIFFUSE (see [[project-nrd-integration]]).
// Gated purely on g_indirect_denoiser -- independent of g_upscaler_type, so
// NRD works under FSR/DLSS too, not just DLSS-RR (NRD_REBLUR_Execute, its
// only real consumer, uses the same gate).
bool PassDefault<Passes::NRD_GBufferPack>::setup(
	Passes::NRD_GBufferPack::Context& data, TaskBuilder& builder)
{
	if (g_indirect_denoiser != IndirectDenoiser::NRD ||
	    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	auto& frame = builder.graph->get_context<ViewportInfo>();
	auto  sz    = frame.frame_size;

	builder.create(data.NRD_ViewZ,
		{ ivec3(sz, 0), HAL::Format::R32_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	builder.create(data.NRD_NormalRoughness,
		{ ivec3(sz, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 }, ResourceFlags::UnorderedAccess);
	builder.create(data.NRD_Mv,
		{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	GBufferViewDesc::need(builder, data.gbuffer);
	return true;
}

void PassDefault<Passes::NRD_GBufferPack>::render(
	Passes::NRD_GBufferPack::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	auto  sz      = context.graph->get_context<ViewportInfo>().frame_size;

	GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

	// FrameInfo (camera matrices, jitter -- FrameLayout::CameraData) isn't
	// bound automatically; every other pass reading it (IndirectRTX,
	// RTXCombine) sets it explicitly first. Missed this originally --
	// CreateFrameInfo() in gbuffer_pack.hlsl was reading whatever the root
	// signature's CameraData slot happened to still hold from a previous
	// dispatch (or nothing, on the first dispatch of this list), not this
	// frame's real camera.
	context.graph->set_slot(SlotID::FrameInfo, compute);

	compute.set_pipeline<PSOS::NRD_GBufferPack>();
	Slots::NRD_GBufferPackParams params;
	gbuffer.SetTable(params.GetGbuffer());
	params.GetNRD_ViewZ() = data.NRD_ViewZ->rwTexture2D;
	params.GetNRD_NormalRoughness() = data.NRD_NormalRoughness->rwTexture2D;
	params.GetNRD_Mv() = data.NRD_Mv->rwTexture2D;
	compute.set(params);

	// dispatch(a, b) takes the full pixel size and divides by the group size
	// itself -- see NormalRoughnessRepack's own comment on this, same gotcha.
	compute.dispatch(uint3(sz, 1), uint3(8, 8, 1));
}
