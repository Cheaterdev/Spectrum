module Graphics:Bloom;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

namespace
{
	VariableContext& bloom_context()
	{
		static auto ctx = VariableContext::create(L"Bloom");
		return *ctx;
	}

	Variable<bool>  g_enabled   = { true, "Enabled", &bloom_context() };
	Variable<Post::BloomMode> g_mode = { Post::BloomMode::Sum, "Mode", &bloom_context() };
	// Sum mode scales it by 1/c_sum_reference_levels so the core strength
	// matches the other modes at the same setting. The default suits the
	// threshold being on (bloom holds only the excess); with Threshold 0 every
	// pixel feeds the chain and ~0.04 (Jimenez's value) is the sane range.
	Variable<float> g_intensity = { 1.0f, "Intensity", &bloom_context(), 0.0f, 3.0f };
	// Bright pass in exposed units: 1.0 = SDR white on screen (last frame's
	// exposure). 0 disables it -- every pixel then scatters a little.
	Variable<float> g_threshold = { 1.0f, "Threshold", &bloom_context(), 0.0f, 8.0f };
	// Soft-knee width as a fraction of Threshold.
	Variable<float> g_knee      = { 0.5f, "Knee", &bloom_context(), 0.0f, 1.0f };
	// Scatter mode: how much each level hands over to the wider ones.
	// 0.7 is Unity HDRP's default.
	Variable<float> g_scatter   = { 0.7f, "Scatter", &bloom_context(), 0.0f, 0.95f };

	constexpr float c_sum_reference_levels = 6.0f;
	// Tent filter radius, in texels of the level being upsampled.
	Variable<float> g_radius    = { 1.0f, "Radius", &bloom_context(), 0.25f, 3.0f };
	// Chain depth starting at a quarter of output resolution; each level
	// doubles how far the glow reaches. 7 reaches as far as 8 did when the
	// chain started at half resolution. Clamped to what the viewport allows.
	Variable<int>   g_levels    = { 7, "Levels", &bloom_context(), 2, 10 };
	// Pre-upscale path only: blend level 0 with reprojected history, against
	// jitter/sub-pixel shimmer. Weight of the history.
	Variable<bool>  g_temporal  = { true, "Temporal stabilization", &bloom_context() };
	Variable<float> g_temporal_weight = { 0.9f, "Temporal history weight", &bloom_context(), 0.0f, 0.98f };
	// Karis-weighted first downsample: tames single-pixel fireflies, but also
	// divides a small very bright source (the sun disk, ~1000) down to near
	// its surroundings, which is why it is off by default -- lighting is
	// already denoised (NRD / DLSS-RR) before bloom runs.
	Variable<bool>  g_karis     = { false, "Firefly suppression (Karis)", &bloom_context() };
}

void bloom_update_selectors(FrameGraph::Graph& graph)
{
	graph.get_context<Table::Post::BloomSelectors>().enabled = g_enabled;
}

namespace
{
	// Level 0 is BloomTAA (quarter output), levels 1+ are BloomDown's mips.
	uint used_levels(HAL::Texture2DView& down)
	{
		return std::min<uint>(uint(std::max(int(g_levels), 2)), down.get_mip_count() + 1);
	}

	float2 texel_of(ivec2 size) { return float2(1.0f / size.x, 1.0f / size.y); }

	// Downsample + bright pass of the scene into level 0 (quarter output).
	// The 13-tap footprint is sized in output-space texels (half of level 0's
	// texel), not the source's: the source is render resolution, whose ratio
	// to the output depends on the upscaler, and this keeps the filter the
	// same at any ratio (exactly the 2x case at DLSS Performance).
	void bloom_level0(HAL::ComputeContext& compute, const HAL::Texture2DView& scene,
		HAL::Texture2DView& target, const auto& exposure, bool unjitter)
	{
		PROFILE(L"bloom_level0");
		compute.set_pipeline<PSOS::Post::BloomDownsample>();

		Slots::Post::BloomDownsample params;
		params.GetSource()         = scene.texture2D;
		params.GetSource_texel()   = texel_of(target.get_size()) * 0.5f;
		params.GetKaris()          = g_karis ? 1u : 0u;
		params.GetThreshold()      = g_threshold;
		params.GetKnee()           = g_knee;
		params.GetUnjitter()       = unjitter ? 1u : 0u;
		params.GetExposure_state() = exposure;
		params.GetTarget()         = target.rwTexture2D;
		compute.set(params);
		compute.dispatch(target.get_size(), ivec2{ 8, 8 });
	}

	// Levels 1+, the upsample chain and the lens flare, from level 0.
	// Shared by BloomProcess and BloomBuildPost: both Contexts carry the fields.
	template<class TContext>
	void bloom_process(TContext& data, FrameContext& context)
	{
		auto& compute = context.get_list()->get_compute();
		auto& list    = context.get_list();

		HAL::Texture2DView& level0 = *data.BloomTAA;
		HAL::Texture2DView& down   = *data.BloomDown;
		HAL::Texture2DView& up     = *data.BloomUp;
		const uint levels = used_levels(down);

		auto level_size = [&](uint level)
		{
			if (level == 0) return level0.get_size();
			const ivec2 base = down.get_size();
			return ivec2(std::max(base.x >> (level - 1), 1), std::max(base.y >> (level - 1), 1));
		};
		auto level_view = [&](uint level)
		{
			return level == 0 ? level0 : down.create_mip(level - 1, *list);
		};

		{
			PROFILE(L"bloom_downsample");
			compute.set_pipeline<PSOS::Post::BloomDownsample>();

			for (uint level = 1; level < levels; level++)
			{
				Slots::Post::BloomDownsample params;
				params.GetSource()       = level_view(level - 1).texture2D;
				params.GetSource_texel() = texel_of(level_size(level - 1));
				params.GetKaris()        = 0;
				params.GetThreshold()    = 0;
				params.GetUnjitter()     = 0;
				params.GetTarget()       = down.create_mip(level - 1, *list).rwTexture2D;
				compute.set(params);
				compute.dispatch(level_size(level), ivec2{ 8, 8 });
			}
		}
		{
			PROFILE(L"bloom_upsample");
			compute.set_pipeline<PSOS::Post::BloomUpsample>();

			// up[k] = level[k] + tent(up[k + 1]), starting from the smallest
			// level itself; up's mip at the last level is never written or read.
			for (int level = int(levels) - 2; level >= 0; level--)
			{
				const bool from_last = level == int(levels) - 2;

				Slots::Post::BloomUpsample params;
				params.GetLow()       = from_last ? level_view(level + 1).texture2D : up.create_mip(level + 1, *list).texture2D;
				params.GetHigh()      = level_view(level).texture2D;
				params.GetTarget()    = up.create_mip(level, *list).rwTexture2D;
				params.GetLow_texel() = texel_of(level_size(level + 1));
				params.GetRadius()    = g_radius;
				params.GetMode()      = g_mode;
				params.GetScatter()   = g_scatter;
				compute.set(params);
				compute.dispatch(level_size(level), ivec2{ 8, 8 });
			}
		}

		// Flare source: level 0 itself, same size as the Flare* targets.
		lens_flare_build(compute, *list, level0,
			*data.FlareGhosts, *data.FlareStreakA, *data.FlareStreakB, *data.FlareStreaks);
	}

	// Last frame BloomBuild's temporal resolve ran, to detect a gap (bloom off,
	// DLSS-RR path, first frame) after which the history is stale.
	uint64_t g_last_temporal_frame = ~0ull;
	ivec2    g_last_temporal_size  = ivec2(0, 0);
}

// setup() is fully generated for all four (bloom.prism's own [SetupCondition]s).
void PassDefault<Passes::Post::BloomBuild>::render(Passes::Post::BloomBuild::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	// camera.jitter for the unjitter, camera/prevCamera for sky reprojection.
	context.graph->set_slot(SlotID::FrameInfo, compute);

	HAL::Texture2DView& current = *data.BloomCurrent;
	HAL::Texture2DView& taa     = *data.BloomTAA;
	HAL::Texture2DView& history = *data.BloomTAAHistory;

	bloom_level0(compute, *data.ResultTexture, current, data.ExposureState->structuredBuffer, true);

	const uint64_t frame = context.graph->builder.current_frame->get_frame();
	const bool reset = !g_temporal
		|| g_last_temporal_frame + 1 != frame
		|| g_last_temporal_size != taa.get_size();
	g_last_temporal_frame = frame;
	g_last_temporal_size  = taa.get_size();

	{
		PROFILE(L"bloom_temporal");
		compute.set_pipeline<PSOS::Post::BloomTemporal>();

		Slots::Post::BloomTemporal params;
		params.GetCurrent()        = current.texture2D;
		params.GetHistory()        = history.texture2D;
		params.GetDepth()          = data.GBuffer_Depth->texture2D;
		params.GetMotion()         = data.GBuffer_Speed->texture2D;
		params.GetTarget()         = taa.rwTexture2D;
		params.GetHistory_weight() = g_temporal_weight;
		params.GetReset()          = reset ? 1u : 0u;
		compute.set(params);
		compute.dispatch(taa.get_size(), ivec2{ 8, 8 });
	}
	{
		PROFILE(L"bloom_history_copy");
		compute.set_pipeline<PSOS::Post::BloomCopy>();

		Slots::Post::BloomCopy params;
		params.GetSource() = taa.texture2D;
		params.GetTarget() = history.rwTexture2D;
		compute.set(params);
		compute.dispatch(history.get_size(), ivec2{ 8, 8 });
	}
}

void PassDefault<Passes::Post::BloomProcess>::render(Passes::Post::BloomProcess::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	// BloomDownsample declares FrameInfo (level 0's unjitter); levels 1+ never
	// read it, but the PSO's slot check can't know that.
	context.graph->set_slot(SlotID::FrameInfo, compute);
	bloom_process(data, context);
}

void PassDefault<Passes::Post::BloomBuildPost>::render(Passes::Post::BloomBuildPost::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);

	bloom_level0(compute, *data.ResultTexture, *data.BloomTAA, data.ExposureState->structuredBuffer, false);
	bloom_process(data, context);
}

void PassDefault<Passes::Post::BloomComposite>::render(Passes::Post::BloomComposite::Context& data, FrameContext& context)
{
	PROFILE(L"bloom_composite");

	auto& compute = context.get_list()->get_compute();
	auto& list    = context.get_list();
	compute.set_signature(Layouts::DefaultLayout);
	compute.set_pipeline<PSOS::Post::BloomComposite>();

	HAL::Texture2DView& up = *data.BloomUp;
	const LensFlareCompositeSettings flare = lens_flare_composite_settings();

	Slots::Post::BloomComposite params;
	params.GetScene()            = data.ResultTexture->texture2D;
	params.GetBloom()            = up.create_mip(0, *list).texture2D;
	params.GetTarget()           = data.ResultTextureNew->rwTexture2D;
	params.GetBloom_texel()      = texel_of(up.get_size());
	params.GetRadius()           = g_radius;
	params.GetAdditive()         = (g_mode == Post::BloomMode::Sum || g_threshold > 0.0f) ? 1u : 0u;
	params.GetIntensity()        = g_intensity;
	switch (g_mode)
	{
	case Post::BloomMode::Sum:     params.GetBloom_scale() = 1.0f / c_sum_reference_levels; break;
	// BloomUp has one level per bloom level (level 0 included), so its mip
	// count bounds the chain the same way BloomDown's + 1 does in processing.
	case Post::BloomMode::Average: params.GetBloom_scale() = 1.0f / float(std::min<uint>(uint(std::max(int(g_levels), 2)), up.get_mip_count())); break;
	default:                       params.GetBloom_scale() = 1.0f; break;
	}
	params.GetFlare_ghosts()     = data.FlareGhosts->texture2D;
	params.GetFlare_streaks()    = data.FlareStreaks->texture2D;
	params.GetUse_ghosts()       = flare.ghosts ? 1u : 0u;
	params.GetUse_streaks()      = flare.streaks ? 1u : 0u;
	params.GetStreak_intensity() = flare.streak_intensity;
	compute.set(params);
	compute.dispatch(data.ResultTextureNew->get_size(), ivec2{ 8, 8 });
}
