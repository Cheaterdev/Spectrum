module Graphics:FSR;
import :FrameGraphContext;

import HAL;

// CAS
#define A_CPU
#include "ffx_a.h"
#include "ffx_fsr1.h"

using namespace FrameGraph;

void FSR::generate(Graph& graph)
{
	struct SkyData
	{
		Handlers::Texture H(ResultTexture);
		Handlers::Texture ResultTextureUpscaled = ResultTexture;

		Handlers::Texture H(FSRTemp);

	};

	graph.pass<SkyData>("FSR", [this, &graph](SkyData& data, TaskBuilder& builder)
		{
		auto& frame = graph.get_context<ViewportInfo>();
			builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
			builder.recreate(data.ResultTextureUpscaled, { uint3(frame.upscale_size,0), HAL::Format::R16G16B16A16_FLOAT, 1 }, ::ResourceFlags::UnorderedAccess);
			builder.create(data.FSRTemp, { uint3(frame.upscale_size,0), HAL::Format::R16G16B16A16_FLOAT, 1 }, ::ResourceFlags::UnorderedAccess);


			return true;
		}, [this, &graph](SkyData& data, FrameContext& _context)
		{
			auto& list = *_context.get_list();
			auto& compute = list.get_compute();

			auto& frame = graph.get_context<ViewportInfo>();
			{

				compute.set_pipeline<PSOS::FSR>();
				{
					Slots::FSR fsr;
					auto& constants = fsr.GetConstants();
					FsrEasuCon(reinterpret_cast<AU1*>(&constants.GetConst0()), reinterpret_cast<AU1*>(&constants.GetConst1()), reinterpret_cast<AU1*>(&constants.GetConst2()), reinterpret_cast<AU1*>(&constants.GetConst3()), static_cast<AF1>(frame.frame_size.x), static_cast<AF1>(frame.frame_size.y), static_cast<AF1>(frame.frame_size.x), static_cast<AF1>(frame.frame_size.y), (AF1)frame.upscale_size.x, (AF1)frame.upscale_size.y);
					fsr.GetSource() = data.ResultTexture->texture2D;
					fsr.GetTarget() = data.FSRTemp->rwTexture2D;

					fsr.set(compute);
				}

				compute.dispach(ivec2{ frame.upscale_size }, ivec2{ 16,16 });
			}


			{

				compute.set_pipeline<PSOS::RCAS>(PSOS::RCAS::cas());
				{
					Slots::FSR fsr;
					auto& constants = fsr.GetConstants();
					FsrRcasCon(reinterpret_cast<AU1*>(&constants.GetConst0()), 0.5);
					fsr.GetSource() = data.FSRTemp->texture2D;
					fsr.GetTarget() = data.ResultTextureUpscaled->rwTexture2D;

					fsr.set(compute);
				}

				compute.dispach(ivec2{ frame.upscale_size }, ivec2{ 16,16 });
			}

		}, PassFlags::Compute);
}

