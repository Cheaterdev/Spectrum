module Graphics:SSS;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// setup() is fully generated (sss.prism's own [SetupCondition]s). SSS_Classify
// lists the 16x16 tiles holding a SUBSURFACE pixel; the four blur passes share
// one shape and dispatch indirectly over that list: horizontal reads the lit
// target and writes SSSTemp, vertical reads SSSTemp and updates the target; the
// DLSS-RR pair differs only in which texture is the target and in adding
// RTXCombine's parked specular.

namespace
{
	// Indirect-dispatch arguments for the tile list: {tile count, 1, 1}. The
	// classify pass copies the list's own GPU counter into the first word; the
	// blur passes (later passes, so Graph's single shared per-queue args buffer
	// would be overwritten in between) only read it. Created on first use,
	// owned by the graph, the same way VSM owns its tile-list args.
	struct SSSTileArgs
	{
		HAL::StructuredBufferView<DispatchArguments> args;
	};

	HAL::StructuredBufferView<DispatchArguments>& tile_args(FrameContext& context)
	{
		auto& holder = context.graph->get_context<SSSTileArgs>();

		if (!holder.args.resource)
		{
			auto& device = RenderSystem::get().device();
			holder.args = HAL::StructuredBufferView<DispatchArguments>(device, 1);

			DispatchArguments init{ 0, 1, 1 };
			auto upload = device.get_upload_list();
			upload->get_copy().update(holder.args, 0, std::span{ &init, 1 });
			upload->execute_and_wait();
		}

		return holder.args;
	}

	template<class Data>
	void dispatch_blur(Data& data, FrameContext& context, bool vertical, HAL::Texture2DView source, HAL::Texture2DView target,
		const HAL::Texture2DView* specular)
	{
		auto& command_list = context.get_list();
		GBuffer gbuffer    = GBufferViewDesc::actualize(data);
		auto& compute      = command_list->get_compute();

		context.graph->set_slot(SlotID::FrameInfo, compute);

		if (vertical)
			compute.set_pipeline<PSOS::Subsurface::SSSBlurV>();
		else
			compute.set_pipeline<PSOS::Subsurface::SSSBlurH>();

		{
			Slots::Subsurface::SSSBlurParams params;
			gbuffer.SetTable(params.GetGbuffer());
			params.GetSource() = source.texture2D;
			params.GetTarget() = target.rwTexture2D;

			if (specular)
			{
				params.GetSpecular()     = specular->texture2D;
				params.GetAdd_specular() = 1;
			}

			compute.set(params);
		}

		{
			Slots::Subsurface::SSSTilesRead tiles;
			tiles.GetTiles() = data.SSSTiles->structuredBuffer;
			compute.set(tiles);
		}

		compute.exec_indirect(tile_args(context), 1);
	}
}

void PassDefault<Passes::Subsurface::SSS_Classify>::render(Passes::Subsurface::SSS_Classify::Context& data, FrameContext& context)
{
	PROFILE_GPU(L"sss_classify");

	auto& command_list = context.get_list();
	GBuffer gbuffer    = GBufferViewDesc::actualize(data);
	auto& compute      = command_list->get_compute();
	auto& args         = tile_args(context);

	{
		Slots::Subsurface::SSSClassifyParams params;
		gbuffer.SetTable(params.GetGbuffer());
		compute.set(params);
	}

	{
		Slots::Subsurface::SSSTilesAppend tiles;
		tiles.GetTiles() = data.SSSTiles->appendStructuredBuffer;
		compute.set(tiles);
	}

	compute.clear_counter(*data.SSSTiles);
	compute.set_pipeline<PSOS::Subsurface::SSSClassify>();
	compute.dispatch(gbuffer.albedo.get_size(), ivec2{ 16, 16 });

	// Same render() as the append above, the counter-to-args copy VSM's tile
	// lists use: no shader needed to turn the count into dispatch arguments.
	command_list->get_copy().copy_buffer(args.resource.get(), 0,
		data.SSSTiles->get_counter_buffer().get(), data.SSSTiles->get_counter_offset(), 4);
}

void PassDefault<Passes::Subsurface::SSS_BlurH>::render(Passes::Subsurface::SSS_BlurH::Context& data, FrameContext& context)
{
	PROFILE_GPU(L"sss_blur_h");
	dispatch_blur(data, context, false, *data.ResultTexture, *data.SSSTemp, nullptr);
}

void PassDefault<Passes::Subsurface::SSS_BlurV>::render(Passes::Subsurface::SSS_BlurV::Context& data, FrameContext& context)
{
	PROFILE_GPU(L"sss_blur_v");
	dispatch_blur(data, context, true, *data.SSSTemp, *data.ResultTexture, nullptr);
}

void PassDefault<Passes::Subsurface::SSSRR_BlurH>::render(Passes::Subsurface::SSSRR_BlurH::Context& data, FrameContext& context)
{
	PROFILE_GPU(L"sss_rr_blur_h");
	dispatch_blur(data, context, false, *data.ResultTextureRTXNoise, *data.SSSTemp, nullptr);
}

void PassDefault<Passes::Subsurface::SSSRR_BlurV>::render(Passes::Subsurface::SSSRR_BlurV::Context& data, FrameContext& context)
{
	PROFILE_GPU(L"sss_rr_blur_v");
	HAL::Texture2DView specular = *data.RTXCombineSpecular;
	dispatch_blur(data, context, true, *data.SSSTemp, *data.ResultTextureRTXNoise, &specular);
}
