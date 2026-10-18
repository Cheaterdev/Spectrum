// The Tonemap pass body. A member of PassDefault<Passes::Post::Tonemap>, which module FrameGraph owns, so it
// is defined from a FrameGraph unit; the settings come through tonemap_settings() (Tonemap.cpp).
module FrameGraph;

import RenderSystem;
import Graphics;
import HAL;
import Core;

using namespace FrameGraph;
using namespace HAL;

namespace
{
	// Histogram coverage in log2 luminance; wider than the min/max EV clamp so
	// the clamp, not the bin range, is what limits adaptation.
	constexpr float c_min_log_lum   = -16.0f;
	constexpr float c_log_lum_range = 36.0f;
}

void PassDefault<Passes::Post::Tonemap>::render(Passes::Post::Tonemap::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);

	const TonemapSettings settings = tonemap_settings();
	const ivec2 size = context.graph->get_context<ViewportInfo>().upscale_size;

	const auto& display = context.graph->get_context<Table::UI::DisplayState>();
	const bool  hdr         = display.hdr && settings.hdr_output;
	const float sdr_white   = std::max(display.sdr_white_nits, 1.0f);
	const float peak_nits   = settings.peak_override_nits > 0.0f ? settings.peak_override_nits : display.max_nits;
	// Never below 1: the curve's peak can't sit under SDR white.
	const float peak        = std::max(peak_nits / sdr_white, 1.0f);
	const float paper_white = std::min(settings.paper_white_nits / sdr_white, peak);
	// A freshly created ExposureState holds no adapted value (Bloom zeroes it),
	// so snap to the target instead of adapting up from EV 0 -- per graph, not
	// only for the first graph in the process, which the settings' reset alone covers.
	const bool reset = tonemap_take_reset() || data.ExposureState.is_new();

	// Every dispatch rebinds the full struct so each one records its own UAV
	// usages -- that is what makes the HAL insert the UAV barriers between
	// clear -> build -> adapt -> apply.
	auto bind = [&]()
	{
		Slots::Post::TonemapData params;
		params.GetMin_log_lum()           = c_min_log_lum;
		params.GetLog_lum_range()         = c_log_lum_range;
		params.GetLow_percent()           = std::min<float>(settings.low_percent, settings.high_percent);
		params.GetHigh_percent()          = settings.high_percent;
		params.GetSpeed_up()              = settings.speed_up;
		params.GetSpeed_down()            = settings.speed_down;
		params.GetDelta_time()            = context.graph->get_context<TimeInfo>().time;
		params.GetExposure_compensation() = settings.compensation;
		params.GetMin_ev()                = settings.min_ev;
		params.GetMax_ev()                = settings.max_ev;
		params.GetReset()                 = reset ? 1u : 0u;
		params.GetAuto_exposure()         = settings.auto_exposure ? 1u : 0u;
		params.GetManual_ev()             = settings.manual_ev;
		params.GetTonemap_operator()      = Post::TonemapOperator(settings.tonemap_operator);
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
}
