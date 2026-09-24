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
	// Chain depth starting at half resolution; each level doubles how far the
	// glow reaches. Clamped to what the viewport size allows.
	Variable<int>   g_levels    = { 8, "Levels", &bloom_context(), 2, 10 };
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

// setup() is fully generated (bloom.prism's own [SetupCondition]).
void PassDefault<Passes::Post::Bloom>::render(Passes::Post::Bloom::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	auto& list    = context.get_list();
	compute.set_signature(Layouts::DefaultLayout);

	HAL::Texture2DView& down = *data.BloomDown;
	HAL::Texture2DView& up   = *data.BloomUp;
	const uint mips = std::min<uint>(uint(std::max(int(g_levels), 2)), down.get_mip_count());
	const ivec2 base_size = down.get_size();

	auto mip_size = [&](uint mip)
	{
		return ivec2(std::max(base_size.x >> mip, 1), std::max(base_size.y >> mip, 1));
	};
	auto texel = [](ivec2 size) { return float2(1.0f / size.x, 1.0f / size.y); };

	{
		PROFILE(L"bloom_downsample");
		compute.set_pipeline<PSOS::Post::BloomDownsample>();

		for (uint mip = 0; mip < mips; mip++)
		{
			Slots::Post::BloomDownsample params;
			if (mip == 0)
			{
				params.GetSource()       = data.ResultTexture->texture2D;
				params.GetSource_texel() = texel(data.ResultTexture->get_size());
				params.GetKaris()        = g_karis ? 1u : 0u;
				params.GetThreshold()    = g_threshold;
				params.GetKnee()         = g_knee;
			}
			else
			{
				params.GetSource()       = down.create_mip(mip - 1, *list).texture2D;
				params.GetSource_texel() = texel(mip_size(mip - 1));
				params.GetKaris()        = 0;
				params.GetThreshold()    = 0;
			}
			params.GetExposure_state() = data.ExposureState->structuredBuffer;
			params.GetTarget() = down.create_mip(mip, *list).rwTexture2D;
			compute.set(params);
			compute.dispatch(mip_size(mip), ivec2{ 8, 8 });
		}
	}
	{
		PROFILE(L"bloom_upsample");
		compute.set_pipeline<PSOS::Post::BloomUpsample>();

		// up[i] = down[i] + tent(up[i + 1]); the chain starts from the
		// smallest downsample, so up's last mip is never written or read.
		for (int mip = int(mips) - 2; mip >= 0; mip--)
		{
			const bool from_down = mip == int(mips) - 2;

			Slots::Post::BloomUpsample params;
			params.GetLow()       = (from_down ? down : up).create_mip(mip + 1, *list).texture2D;
			params.GetHigh()      = down.create_mip(mip, *list).texture2D;
			params.GetTarget()    = up.create_mip(mip, *list).rwTexture2D;
			params.GetLow_texel() = texel(mip_size(mip + 1));
			params.GetRadius()    = g_radius;
			params.GetMode()      = g_mode;
			params.GetScatter()   = g_scatter;
			compute.set(params);
			compute.dispatch(mip_size(mip), ivec2{ 8, 8 });
		}
	}
	{
		PROFILE(L"bloom_composite");
		compute.set_pipeline<PSOS::Post::BloomComposite>();

		const ivec2 size = data.ResultTextureNew->get_size();

		Slots::Post::BloomComposite params;
		params.GetScene()       = data.ResultTexture->texture2D;
		params.GetBloom()       = up.create_mip(0, *list).texture2D;
		params.GetTarget()      = data.ResultTextureNew->rwTexture2D;
		params.GetBloom_texel() = texel(mip_size(0));
		params.GetRadius()      = g_radius;
		params.GetAdditive()    = (g_mode == Post::BloomMode::Sum || g_threshold > 0.0f) ? 1u : 0u;
		params.GetIntensity()   = g_intensity;
		switch (g_mode)
		{
		case Post::BloomMode::Sum:     params.GetBloom_scale() = 1.0f / c_sum_reference_levels; break;
		case Post::BloomMode::Average: params.GetBloom_scale() = 1.0f / float(mips); break;
		default:                       params.GetBloom_scale() = 1.0f; break;
		}
		compute.set(params);
		compute.dispatch(size, ivec2{ 8, 8 });
	}
}
