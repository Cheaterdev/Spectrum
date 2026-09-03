// DLSS Ray Reconstruction's FrameGraph pass. No new exports of its own —
// PassDefault<Passes::UpscalingDLSSRR>::setup/render (UpscalingDLSSRR.cpp)
// reuse UpscalingDLSS.cpp's globals (g_upscaling_enabled, g_upscaling_dlss_mode).
export module Graphics:UpscalingDLSSRR;

import HAL;
import :UpscalingDLSS;
