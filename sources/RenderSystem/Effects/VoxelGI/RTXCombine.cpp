module Graphics:RTXCombine;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

#ifdef HAL_BACKEND_D3D12

FrameGraph::SetupResult PassDefault<Passes::RTXCombine>::setup(
	Passes::RTXCombine::Context& data, TaskBuilder& builder)
{
	// Same gate as its three producers (ReflectionRTX/IndirectRTX/ShadowRTX)
	// -- runs instead of ReflCombine whenever the user has picked DLSS-RR
	// via g_upscaler_type (and it's actually available).
	if (g_upscaler_type != UpscalerType::DLSSRR ||
	    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	return true;
}

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

#else

FrameGraph::SetupResult PassDefault<Passes::RTXCombine>::setup(Passes::RTXCombine::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::RTXCombine>::render(Passes::RTXCombine::Context&, FrameContext&) {}

#endif
