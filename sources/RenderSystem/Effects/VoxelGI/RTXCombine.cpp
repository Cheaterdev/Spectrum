module Graphics:RTXCombine;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// setup() is fully generated (voxel.prism's own [SetupCondition]) -- same
// gate as its three producers (ReflectionRTX/IndirectRTX/ShadowRTX), runs
// instead of ReflCombine whenever the user has picked DLSS-RR (and it's
// actually available).

void PassDefault<Passes::RTXCombine>::render(
	Passes::RTXCombine::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();
	auto  target_tex   = *data.ResultTextureRTXNoise;
	GBuffer gbuffer    = GBufferViewDesc::actualize(data);
	auto  sz           = target_tex.get_size();
	auto& compute      = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	compute.set_pipeline<PSOS::RTXCombine>();

	{
		Slots::RTXCombine combine;
		gbuffer.SetTable(combine.GetGbuffer());
		combine.GetReflection() = data.RTXReflectionNoise->texture2D;
		combine.GetIndirect()   = data.RTXIndirectNoise->texture2D;
		combine.GetShadow()     = data.RTXShadowNoise->texture2D;
		combine.GetTarget()     = data.ResultTextureRTXNoise->rwTexture2D;
		compute.set(combine);
	}

	compute.dispatch(sz);
}

