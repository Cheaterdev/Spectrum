module Graphics:NRD_ShadowCombine;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// Composites NRD's denoised RTX-reference shadow onto ResultTexture, sibling
// of NRD_IndirectCombine.cpp -- OVERWRITES the shadow term (see
// nrd_shadow_combine.hlsl) rather than adding, since it's redoing the same
// direct-lighting term VSM's own passes would otherwise have written. Runs
// only when ShadowSource::RTXReference is selected outside DLSS-RR (see
// [[project-nrd-integration]]).
// setup() is fully generated (nrd_sig_test.prism's own [SetupCondition]).

void PassDefault<Passes::NRD_ShadowCombine>::render(
	Passes::NRD_ShadowCombine::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();
	GBuffer gbuffer     = GBufferViewDesc::actualize(data);
	auto  target_tex    = *data.ResultTexture;
	auto  sz            = target_tex.get_size();
	auto& compute       = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);

	compute.set_pipeline<PSOS::NRD_ShadowCombine>();

	{
		Slots::NRD_ShadowCombineParams params;
		gbuffer.SetTable(params.GetGbuffer());
		params.GetShadow_denoised() = data.VSM_ShadowDenoised->texture2D;
		params.GetTarget()          = data.ResultTexture->rwTexture2D;
		compute.set(params);
	}

	compute.dispatch(sz);
}
