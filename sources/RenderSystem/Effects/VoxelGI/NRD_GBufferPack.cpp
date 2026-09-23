module Graphics:NRD_GBufferPack;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Front-end packing for NRD REBLUR_DIFFUSE/REBLUR_SPECULAR (see
// [[project-nrd-integration]]), including the radiance+hitdist pack --
// exactly one candidate per channel, whichever g_indirect_source/
// g_reflection_source actually selects (see this pass's own .prism comment
// for the full reasoning: packing/needing the OTHER candidate too was
// either a wasted write (RTX side, since IndirectRTX runs regardless of
// selection) or a crash (VCT side, since VoxelScreen/ScreenReflection only
// run when actually selected -- hit that once already). Gated on RTX/
// hardware support AND g_upscaler_type != DLSSRR -- NRD isn't used at all
// under DLSS-RR any more (RTXCombine reads the raw RTX candidates directly
// there instead, see RTXCombine's own comment), so there's no reason to run
// this or NRD_REBLUR_Execute (its only real consumer, same gate) while
// DLSS-RR is selected.
// setup() is fully generated (nrd_sig_test.prism's own [SetupCondition]).

void PassDefault<Passes::NRD_GBufferPack>::render(
	Passes::NRD_GBufferPack::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	auto  sz      = context.graph->get_context<ViewportInfo>().frame_size;

	GBuffer gbuffer = GBufferViewDesc::actualize(data);

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
	// Exactly one of each pair set -- data.X is only a valid,
	// dereferenceable handle when it was actually need()'d in setup()
	// above, matching the same condition. The other is left at its
	// [Auto = Texture_Null] default; harmless since the shader branches on
	// the *_use_vct flags, not on which happens to be bound.
	auto& selectors = context.graph->get_context<Table::IndirectGISelectors>();
	bool useVctIndirect   = selectors.indirect_source   == IndirectSource::MyVCT;
	bool useVctReflection = selectors.reflection_source == ReflectionSource::MyReflection;
	if (useVctIndirect)   params.GetVoxelIndirectNoiseRaw()   = data.VoxelIndirectNoiseRaw->texture2D;
	else                  params.GetRTXIndirectNoise()        = data.RTXIndirectNoise->texture2D;
	if (useVctReflection) params.GetVoxelReflectionNoiseRaw() = data.VoxelReflectionNoiseRaw->texture2D;
	else                  params.GetRTXReflectionNoise()      = data.RTXReflectionNoise->texture2D;
	params.GetIndirect_use_vct()   = useVctIndirect;
	params.GetReflection_use_vct() = useVctReflection;
	params.GetNRD_ViewZ() = data.NRD_ViewZ->rwTexture2D;
	params.GetNRD_NormalRoughness() = data.NRD_NormalRoughness->rwTexture2D;
	params.GetNRD_Mv() = data.NRD_Mv->rwTexture2D;
	params.GetNRD_DiffuseRadianceHitDist()  = data.NRD_DiffuseRadianceHitDist->rwTexture2D;
	params.GetNRD_SpecularRadianceHitDist() = data.NRD_SpecularRadianceHitDist->rwTexture2D;
	compute.set(params);

	// dispatch(a, b) takes the full pixel size and divides by the group size
	// itself -- see NormalRoughnessRepack's own comment on this, same gotcha.
	compute.dispatch(uint3(sz, 1), uint3(8, 8, 1));
}
