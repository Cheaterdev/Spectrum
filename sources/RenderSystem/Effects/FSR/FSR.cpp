module Graphics:FSR;


import :FrameGraphContext;
import :UpscalingDLSS;

import HAL;

// CAS
#define A_CPU
#include "ffx_a.h"
#include "ffx_fsr1.h"

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;

// setup() is fully generated (FSR.prism's own [SetupCondition]).

void PassDefault<Passes::Post::Upscale::FSR>::render(Passes::Post::Upscale::FSR::Context& data, FrameContext& context)
{
	auto& frame   = context.graph->get_context<ViewportInfo>();
	auto& compute = context.get_list()->get_compute();

	compute.set_pipeline<PSOS::Post::Upscale::FSR>();
	{
		Slots::Post::Upscale::FSR fsr;
		auto& constants = fsr.GetConstants();
		FsrEasuCon(
			reinterpret_cast<AU1*>(&constants.GetConst0()),
			reinterpret_cast<AU1*>(&constants.GetConst1()),
			reinterpret_cast<AU1*>(&constants.GetConst2()),
			reinterpret_cast<AU1*>(&constants.GetConst3()),
			static_cast<AF1>(frame.frame_size.x), static_cast<AF1>(frame.frame_size.y),
			static_cast<AF1>(frame.frame_size.x), static_cast<AF1>(frame.frame_size.y),
			static_cast<AF1>(frame.upscale_size.x), static_cast<AF1>(frame.upscale_size.y));
		fsr.GetSource() = data.ResultTexture->texture2D;
		fsr.GetTarget() = data.FSRTemp->rwTexture2D;
		compute.set(fsr);
	}
	compute.dispatch(ivec2{ frame.upscale_size }, ivec2{ 16, 16 });

	compute.set_pipeline<PSOS::Post::Upscale::RCAS>(PSOS::Post::Upscale::RCAS::cas());
	{
		Slots::Post::Upscale::FSR fsr;
		auto& constants = fsr.GetConstants();
		FsrRcasCon(reinterpret_cast<AU1*>(&constants.GetConst0()), 0.5);
		fsr.GetSource() = data.FSRTemp->texture2D;
		fsr.GetTarget() = data.ResultTextureNew->rwTexture2D;
		compute.set(fsr);
	}
	compute.dispatch(ivec2{ frame.upscale_size }, ivec2{ 16, 16 });
}
