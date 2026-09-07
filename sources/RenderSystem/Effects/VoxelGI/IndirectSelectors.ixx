export module Graphics:IndirectSelectors;

// Selectors for the indirect-GI source/denoiser experiment (see
// [[project-nrd-integration]]). Plain globals set by main.cpp's debug
// toolbar, same shape as g_upscaler_type (UpscalingDLSS.ixx).
export
{
	// Which raw/noisy indirect-GI signal is available as a denoiser's input:
	// the legacy voxel-cone-traced path (VoxelIndirectNoiseRaw -- its own
	// temporal lerp skipped so NRD can do its own accumulation instead of
	// double-accumulating) or the RTX-only reference path (RTXIndirectNoise,
	// IndirectRTX's raygen). Independent of g_indirect_denoiser below --
	// only observed when that picks NRD.
	enum class IndirectSource { MyVCT, RTXReference };
	IndirectSource g_indirect_source = IndirectSource::RTXReference;

	// Which processed signal supplies the final indirect contribution: the
	// legacy full VCT pipeline (VoxelIndirectFiltered, spatial-blurred,
	// unaffected by g_indirect_source) or NRD's REBLUR-denoised output
	// (RTXIndirectDenoised, whose *input* is picked by g_indirect_source).
	// Independent of g_upscaler_type -- RTXCombine reads this under DLSS-RR,
	// NRD_IndirectCombine/VoxelCombine's own composite under FSR/DLSS (see
	// [[project-nrd-integration]]).
	enum class IndirectDenoiser { Legacy, NRD };
	IndirectDenoiser g_indirect_denoiser = IndirectDenoiser::NRD;

	// Reflection counterparts of the pair above (see
	// [[project-nrd-integration]]'s reflection-denoiser plan): which raw
	// signal feeds NRD REBLUR_SPECULAR -- the legacy ScreenReflection path
	// (VoxelReflectionNoiseRaw, pre-ReflectionDenoiser_Reproject) or the
	// RTX-only reference path (RTXReflectionNoise, ReflectionRTX's raygen).
	enum class ReflectionSource { MyReflection, RTXReference };
	ReflectionSource g_reflection_source = ReflectionSource::RTXReference;

	// Which processed signal supplies the final reflection contribution: the
	// legacy FFX-denoised VoxelReflectionNoise (ReflectionDenoiser_Reproject,
	// unaffected by g_reflection_source) or NRD's REBLUR_SPECULAR-denoised
	// output (RTXReflectionDenoised, whose *input* is picked by
	// g_reflection_source). Independent of g_upscaler_type -- RTXCombine
	// reads this under DLSS-RR, ReflCombine under FSR/DLSS.
	// Named ReflectionDenoiserKind, not ReflectionDenoiser -- that name is
	// already taken by the ReflectionDenoiser class (Effects/Denoisers/
	// ReflectionDenoiser.ixx), an unrelated FFX-denoiser GraphGenerator.
	enum class ReflectionDenoiserKind { Legacy, NRD };
	ReflectionDenoiserKind g_reflection_denoiser = ReflectionDenoiserKind::Legacy;
}
