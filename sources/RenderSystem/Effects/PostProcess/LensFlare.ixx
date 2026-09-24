export module Graphics:LensFlare;

import HAL;

// Image-based lens flare. Owns its settings; the passes that run it are
// BloomBuild/BloomBuildPost (build) and BloomComposite (add), see bloom.prism.

// Ghosts + halo and streaks from `source` (a thresholded bloom level) into the
// quarter-size flare targets.
export void lens_flare_build(HAL::ComputeContext& compute, HAL::CommandList& list,
	const HAL::Texture2DView& source,
	HAL::Texture2DView& ghosts, HAL::Texture2DView& streak_a,
	HAL::Texture2DView& streak_b, HAL::Texture2DView& streaks);

export struct LensFlareCompositeSettings
{
	bool  ghosts = false;
	bool  streaks = false;
	float streak_intensity = 0.0f;
};

export LensFlareCompositeSettings lens_flare_composite_settings();
