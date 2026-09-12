module Graphics:NRD_IndirectCombine;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// The FSR/DLSS-side equivalent of the indirect term RTXCombine computes for
// DLSS-RR (see [[project-nrd-integration]]). Runs exactly in the
// complementary case to RTXCombine (which handles DLSS-RR itself).
// setup() is fully generated (nrd_sig_test.sig's own [SetupCondition]).

void PassDefault<Passes::NRD_IndirectCombine>::render(
	Passes::NRD_IndirectCombine::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();
	GBuffer gbuffer     = GBufferViewDesc::actualize(data);
	auto  target_tex    = *data.ResultTexture;
	auto  sz            = target_tex.get_size();
	auto& compute       = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);

	compute.set_pipeline<PSOS::NRD_IndirectCombine>();

	{
		Slots::NRD_IndirectCombineParams params;
		gbuffer.SetTable(params.GetGbuffer());
		params.GetIndirect() = data.RTXIndirectDenoised->texture2D;
		params.GetTarget()   = data.ResultTexture->rwTexture2D;
		compute.set(params);
	}

	compute.dispatch(sz);
}
