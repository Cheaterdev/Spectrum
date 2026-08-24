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
}
