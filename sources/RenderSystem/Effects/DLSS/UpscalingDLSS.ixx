export module Graphics:UpscalingDLSS;

import HAL;
// For is_rtx_supported() in upscaler_is_available() -- RenderSystem is a
// standalone module (imports only HAL/Core), so no partition cycle.
import RenderSystem;

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

	// UpscalerType itself is SIG-declared now (UpscalingDLSS.sig), so it's
	// visible via plain `import HAL;` -- needed by autogen/pass_defaults.cpp,
	// a dedicated TU generated [SetupCondition]/[RenderCondition] bodies
	// compile in (see that file's own comment).

	// Is `type` runnable at all on this hardware/SDK? FSR has no gate, so it
	// is always valid -- which makes it both the default and the fallback.
	//
	// DLSS-RR folds in rtx_supported deliberately: ray reconstruction
	// reconstructs from ray-traced inputs, so on a non-RTX device there is
	// nothing for it to reconstruct. Answering "unavailable" here is what
	// lets every RR-gated pass test the selection alone (`upscaler_type ==
	// DLSSRR`) instead of re-ANDing rtx_supported && dlssrr_available at each
	// site -- see set_upscaler_type()'s invariant below.
	bool upscaler_is_available(UpscalerType type)
	{
		switch (type)
		{
		case UpscalerType::DLSS:   return nvidia::DLSS::get().available();
		case UpscalerType::DLSSRR: return RenderSystem::get().device().is_rtx_supported() &&
		                                  nvidia::DLSSRR::get().available();
		case UpscalerType::FSR:
		default:                   return true;
		}
	}
}

namespace UpscalerDetail
{
	UpscalerType g_selected = UpscalerType::FSR;
}

export
{
	// Explicit user choice among the upscalers, when g_upscaling_enabled is
	// on -- replaces the old implicit hardware-priority chain (DLSS-RR >
	// DLSS-SR > FSR) with a direct switch, set by main.cpp's upscaler
	// combobox.
	//
	// INVARIANT: only ever holds a type upscaler_is_available() accepts.
	// Every reader may therefore test the selection ALONE -- `upscaler_type ==
	// DLSSRR` already implies dlssrr_available && rtx_supported, and
	// `!= DLSSRR` already covers "RR was asked for but can't run". That is
	// what keeps the generated [SetupCondition] expressions (FSR.sig,
	// UpscalingDLSS*.sig, voxel.sig, nrd_sig_test.sig) down to one context
	// read instead of three, and it is why this is a read-only reference:
	// assigning to it is a compile error, so the clamp in set_upscaler_type()
	// cannot be bypassed by a new write site.
	const UpscalerType& g_upscaler_type = UpscalerDetail::g_selected;

	// The one mutation point. An unavailable request falls back to FSR rather
	// than being stored and re-checked downstream.
	void set_upscaler_type(UpscalerType type)
	{
		UpscalerDetail::g_selected = upscaler_is_available(type) ? type : UpscalerType::FSR;
	}
}
