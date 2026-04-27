module Graphics:FSR;

import <RenderSystem.h>;
import :FrameGraphContext;

import HAL;

// CAS
#define A_CPU
#include "ffx_a.h"
#include "ffx_fsr1.h"

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;

bool PassDefault<Passes::FSR>::setup(Passes::FSR::Context& data, TaskBuilder& builder)
{
	auto& frame = builder.graph->get_context<ViewportInfo>();
	builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
	builder.recreate(data.ResultTextureNew,
		{ uint3(frame.upscale_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 },
		ResourceFlags::UnorderedAccess);
	builder.create(data.FSRTemp,
		{ uint3(frame.upscale_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 },
		ResourceFlags::UnorderedAccess);
	return true;
}

void PassDefault<Passes::FSR>::render(Passes::FSR::Context& data, FrameContext& context)
{
	auto& frame   = context.graph->get_context<ViewportInfo>();
	auto& compute = context.get_list()->get_compute();

	compute.set_pipeline<PSOS::FSR>();
	{
		Slots::FSR fsr;
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

	compute.set_pipeline<PSOS::RCAS>(PSOS::RCAS::cas());
	{
		Slots::FSR fsr;
		auto& constants = fsr.GetConstants();
		FsrRcasCon(reinterpret_cast<AU1*>(&constants.GetConst0()), 0.5);
		fsr.GetSource() = data.FSRTemp->texture2D;
		fsr.GetTarget() = data.ResultTextureNew->rwTexture2D;
		compute.set(fsr);
	}
	compute.dispatch(ivec2{ frame.upscale_size }, ivec2{ 16, 16 });
}
