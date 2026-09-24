module Graphics:Tonemap;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

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

	// Histogram coverage in log2 luminance; wider than the min/max EV clamp so
	// the clamp, not the bin range, is what limits adaptation.
	constexpr float c_min_log_lum   = -16.0f;
	constexpr float c_log_lum_range = 36.0f;

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

// setup() is fully generated (tonemap.prism's own [SetupCondition]).
void PassDefault<Passes::Post::Tonemap>::render(Passes::Post::Tonemap::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);

	const ivec2 size = context.graph->get_context<ViewportInfo>().upscale_size;

	const auto& display = context.graph->get_context<Table::UI::DisplayState>();
	const bool  hdr         = display.hdr && g_hdr_output;
	const float sdr_white   = std::max(display.sdr_white_nits, 1.0f);
	const float peak_nits   = g_peak_override > 0.0f ? float(g_peak_override) : display.max_nits;
	// Never below 1: the curve's peak can't sit under SDR white.
	const float peak        = std::max(peak_nits / sdr_white, 1.0f);
	const float paper_white = std::min(g_paper_white / sdr_white, peak);

	// Every dispatch rebinds the full struct so each one records its own UAV
	// usages -- that is what makes the HAL insert the UAV barriers between
	// clear -> build -> adapt -> apply.
	auto bind = [&]()
	{
		Slots::Post::TonemapData params;
		params.GetMin_log_lum()           = c_min_log_lum;
		params.GetLog_lum_range()         = c_log_lum_range;
		params.GetLow_percent()           = std::min<float>(g_low_percent, g_high_percent);
		params.GetHigh_percent()          = g_high_percent;
		params.GetSpeed_up()              = g_speed_up;
		params.GetSpeed_down()            = g_speed_down;
		params.GetDelta_time()            = context.graph->get_context<TimeInfo>().time;
		params.GetExposure_compensation() = g_compensation;
		params.GetMin_ev()                = g_min_ev;
		params.GetMax_ev()                = g_max_ev;
		params.GetReset()                 = g_reset ? 1u : 0u;
		params.GetAuto_exposure()         = g_auto_exposure ? 1u : 0u;
		params.GetManual_ev()             = g_manual_ev;
		params.GetTonemap_operator()      = g_operator;
		params.GetHdr()                   = hdr ? 1u : 0u;
		params.GetPaper_white()           = paper_white;
		params.GetPeak()                  = peak;
		params.GetHistogram()             = data.ExposureHistogram->rwStructuredBuffer;
		params.GetExposure_state()        = data.ExposureState->rwStructuredBuffer;
		params.GetColor()                 = data.ResultTexture->rwTexture2D;
		compute.set(params);
	};

	{
		PROFILE(L"tonemap_histogram");
		bind();
		compute.set_pipeline<PSOS::Post::TonemapHistogramClear>();
		compute.dispatch(ivec2{ 256, 1 }, ivec2{ 256, 1 });

		bind();
		compute.set_pipeline<PSOS::Post::TonemapHistogramBuild>();
		compute.dispatch(size, ivec2{ 16, 16 });
	}
	{
		PROFILE(L"tonemap_exposure_adapt");
		bind();
		compute.set_pipeline<PSOS::Post::TonemapExposureAdapt>();
		compute.dispatch(ivec2{ 256, 1 }, ivec2{ 256, 1 });
	}
	{
		PROFILE(L"tonemap_apply");
		bind();
		compute.set_pipeline<PSOS::Post::TonemapApply>();
		compute.dispatch(size, ivec2{ 8, 8 });
	}

	g_reset = false;
}
