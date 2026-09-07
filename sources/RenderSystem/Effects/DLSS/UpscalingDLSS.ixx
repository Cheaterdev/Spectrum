export module Graphics:UpscalingDLSS;

import HAL;

export
{
	// Single source of truth for the DLSS quality mode this pipeline uses —
	// read by both UpscalingDLSS.cpp's upscale() call and main.cpp's frame_size
	// query. Mutable so main.cpp's DLSS-quality combobox can change it at runtime.
	nvidia::DLSSMode g_upscaling_dlss_mode = nvidia::DLSSMode::MaxPerformance;

	// User override for render scale, as a percent between
	// DLSSOptimalSettings::render_size_min and render_size_max. -1 means
	// "use DLSS's own recommended render_size" (the default); set by
	// main.cpp's scale slider/"Recommended" button.
	float g_upscaling_dlss_scale_override = -1.0f;

	// False when main.cpp's "downsampled" toggle is off — frame_size ==
	// upscale_size that frame, so there is nothing to upscale. UpscalingDLSS
	// and FSR both gate on this (in addition to their existing DLSS-
	// availability check) so neither runs a pointless native-to-native
	// upscale; SMAA gates on the opposite so native rendering still gets AA.
	bool g_upscaling_enabled = true;

	enum class UpscalerType { FSR, DLSS, DLSSRR };

	// Explicit user choice among the upscalers, when g_upscaling_enabled is
	// on — replaces the old implicit hardware-priority chain (DLSS-RR >
	// DLSS-SR > FSR) with a direct switch, set by main.cpp's upscaler
	// combobox (which only lists options actually available on this
	// hardware). FSR is always valid (no hardware gate), so it's both the
	// default and the automatic fallback (see upscaler_is_available())
	// when the selected type turns out not to be available.
	UpscalerType g_upscaler_type = UpscalerType::DLSSRR; // TEMP: REBLUR diagnostic run, revert to FSR before finishing

	bool upscaler_is_available(UpscalerType type)
	{
		switch (type)
		{
		case UpscalerType::DLSS:   return nvidia::DLSS::get().available();
		case UpscalerType::DLSSRR: return nvidia::DLSSRR::get().available();
		case UpscalerType::FSR:
		default:                   return true;
		}
	}
}
