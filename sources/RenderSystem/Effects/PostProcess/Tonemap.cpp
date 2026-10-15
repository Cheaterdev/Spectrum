module Graphics:Tonemap;

import RenderSystem;
import Graphics;
import HAL;
import Core;


using namespace FrameGraph;
using namespace HAL;

namespace
{
	VariableContext& tonemap_context()
	{
		static auto ctx = VariableContext::create(L"Tonemap");
		return *ctx;
	}

	Variable<bool>            g_enabled        = { true, "Enabled", &tonemap_context() };
	Variable<Post::TonemapOperator> g_operator       = { Post::TonemapOperator::AgX, "Operator", &tonemap_context() };
	Variable<bool>            g_auto_exposure  = { true, "Auto exposure", &tonemap_context() };
	Variable<float>           g_compensation   = { 0.0f, "Exposure compensation (EV)", &tonemap_context(), -6.0f, 6.0f };
	Variable<float>           g_manual_ev      = { 0.0f, "Manual EV", &tonemap_context(), -10.0f, 16.0f };
	Variable<float>           g_speed_up       = { 3.0f, "Adapt speed brighten", &tonemap_context(), 0.1f, 20.0f };
	Variable<float>           g_speed_down     = { 1.0f, "Adapt speed darken", &tonemap_context(), 0.1f, 20.0f };
	Variable<float>           g_low_percent    = { 0.5f, "Metering low percentile", &tonemap_context(), 0.0f, 0.99f };
	Variable<float>           g_high_percent   = { 0.95f, "Metering high percentile", &tonemap_context(), 0.01f, 1.0f };
	Variable<float>           g_min_ev         = { -10.0f, "Min scene EV", &tonemap_context(), -16.0f, 16.0f };
	Variable<float>           g_max_ev         = { 12.0f, "Max scene EV", &tonemap_context(), -16.0f, 20.0f };

	// HDR output only applies when the window's display is in HDR mode.
	Variable<bool>            g_hdr_output     = { true, "HDR output", &tonemap_context() };
	// Where the scene's diffuse white lands; 203 nits is the BT.2408 reference.
	Variable<float>           g_paper_white    = { 203.0f, "HDR paper white (nits)", &tonemap_context(), 80.0f, 600.0f };
	// 0 = the display's reported peak.
	Variable<float>           g_peak_override  = { 0.0f, "HDR peak override (nits)", &tonemap_context(), 0.0f, 4000.0f };

	bool g_reset = true;
	bool g_prev_auto_exposure = true;
}

void tonemap_update_selectors(FrameGraph::Graph& graph)
{
	graph.get_context<Table::Post::TonemapSelectors>().enabled = g_enabled;

	if (g_auto_exposure && !g_prev_auto_exposure)
		g_reset = true;
	g_prev_auto_exposure = g_auto_exposure;
}

TonemapSettings tonemap_settings()
{
	return {
		.tonemap_operator   = int(Post::TonemapOperator(g_operator)),
		.auto_exposure      = g_auto_exposure,
		.compensation       = g_compensation,
		.manual_ev          = g_manual_ev,
		.speed_up           = g_speed_up,
		.speed_down         = g_speed_down,
		.low_percent        = g_low_percent,
		.high_percent       = g_high_percent,
		.min_ev             = g_min_ev,
		.max_ev             = g_max_ev,
		.hdr_output         = g_hdr_output,
		.paper_white_nits   = g_paper_white,
		.peak_override_nits = g_peak_override,
	};
}

bool tonemap_take_reset()
{
	const bool reset = g_reset;
	g_reset = false;
	return reset;
}
