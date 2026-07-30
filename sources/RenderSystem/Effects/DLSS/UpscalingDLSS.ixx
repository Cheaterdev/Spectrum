export module Graphics:UpscalingDLSS;

import HAL;

export
{
	// Single source of truth for the DLSS quality mode this pipeline uses —
	// read by both UpscalingDLSS.cpp's upscale() call and main.cpp's frame_size
	// query. Mutable so main.cpp's DLSS-quality combobox can change it at runtime.
	nvidia::DLSSMode g_upscaling_dlss_mode = nvidia::DLSSMode::MaxPerformance;
}
