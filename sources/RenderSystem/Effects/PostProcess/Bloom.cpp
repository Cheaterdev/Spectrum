module Graphics:Bloom;

import RenderSystem;
import Graphics;
import HAL;
import Core;


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

BloomSettings bloom_settings()
{
	return {
		.mode            = int(Post::BloomMode(g_mode)),
		.intensity       = g_intensity,
		.threshold       = g_threshold,
		.knee            = g_knee,
		.scatter         = g_scatter,
		.radius          = g_radius,
		.levels          = g_levels,
		.temporal        = g_temporal,
		.temporal_weight = g_temporal_weight,
		.karis           = g_karis,
	};
}
