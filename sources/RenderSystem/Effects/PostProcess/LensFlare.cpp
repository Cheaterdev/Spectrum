module Graphics:LensFlare;

import RenderSystem;
import Graphics;
import HAL;
import Core;

using namespace HAL;

namespace
{
	VariableContext& lens_flare_context()
	{
		static auto ctx = VariableContext::create(L"Lens flare");
		return *ctx;
	}

	Variable<bool>               g_enabled          = { true, "Enabled", &lens_flare_context() };
	Variable<float>              g_ghost_intensity  = { 0.01f, "Ghosts", &lens_flare_context(), 0.0f, 0.1f };
	Variable<float>              g_halo_intensity   = { 0.005f, "Halo", &lens_flare_context(), 0.0f, 0.1f };
	// Screen-height units.
	Variable<float>              g_halo_radius      = { 0.45f, "Halo radius", &lens_flare_context(), 0.1f, 1.0f };
	Variable<float>              g_chromatic        = { 0.02f, "Chromatic", &lens_flare_context(), 0.0f, 0.1f };
	Variable<Post::FlareStreaks> g_streaks          = { Post::FlareStreaks::Star6, "Streaks", &lens_flare_context() };
	Variable<float>              g_streak_intensity = { 0.2f, "Streak intensity", &lens_flare_context(), 0.0f, 2.0f };
	// Weight of each successive (4x longer) streak level: 1 = every level
	// equal, so the tail runs to the screen edge; lower = shorter streaks.
	Variable<float>              g_streak_falloff   = { 1.0f, "Streak falloff", &lens_flare_context(), 0.3f, 1.0f };
	Variable<float>              g_streak_rotation  = { 15.0f, "Streak rotation (deg)", &lens_flare_context(), 0.0f, 90.0f };

	// Kawase: the tap spacing grows 4x per iteration; each level's taps span
	// 3.5 steps (lens_flare.hlsl's CS_Streak), so level k reaches
	// 3.5 * (1 + 4 + ... + 4^k) texels.
	constexpr float c_streak_step_base = 4.0f;
	constexpr float c_streak_level_span = 3.5f;

	// Enough levels that the last one spans the whole flare texture.
	int streak_iterations(ivec2 size)
	{
		int n = 1;
		float reach = c_streak_level_span;
		for (float step = c_streak_step_base; reach < float(std::max(size.x, size.y)); step *= c_streak_step_base)
		{
			reach += c_streak_level_span * step;
			n++;
		}
		return n;
	}

	uint streak_direction_count(Post::FlareStreaks s)
	{
		switch (s)
		{
		case Post::FlareStreaks::Anamorphic: return 2;
		case Post::FlareStreaks::Star4:      return 4;
		case Post::FlareStreaks::Star6:      return 6;
		case Post::FlareStreaks::Star8:      return 8;
		default:                             return 0;
		}
	}
}

LensFlareCompositeSettings lens_flare_composite_settings()
{
	LensFlareCompositeSettings s;
	const uint directions = g_enabled ? streak_direction_count(g_streaks) : 0;
	s.ghosts  = g_enabled;
	s.streaks = directions > 0;
	// Normalized to the 2-direction (Anamorphic) case, so switching patterns
	// keeps overall streak brightness.
	s.streak_intensity = directions > 0 ? g_streak_intensity * 2.0f / float(directions) : 0.0f;
	return s;
}

void lens_flare_build(HAL::ComputeContext& compute, HAL::CommandList& list,
	const HAL::Texture2DView& source,
	HAL::Texture2DView& ghosts, HAL::Texture2DView& streak_a,
	HAL::Texture2DView& streak_b, HAL::Texture2DView& streaks)
{
	if (!g_enabled)
		return;

	const ivec2 flare_size = ghosts.get_size();
	const float2 texel = float2(1.0f / flare_size.x, 1.0f / flare_size.y);

	{
		PROFILE(L"lens_flare_ghosts");
		compute.set_pipeline<PSOS::Post::LensFlareGhosts>();

		Slots::Post::LensFlareGhosts params;
		params.GetSource()          = source.texture2D;
		params.GetTarget()          = ghosts.rwTexture2D;
		params.GetGhost_intensity() = g_ghost_intensity;
		params.GetHalo_intensity()  = g_halo_intensity;
		params.GetHalo_radius()     = g_halo_radius;
		params.GetChromatic()       = g_chromatic;
		params.GetAspect()          = float(flare_size.x) / float(flare_size.y);
		compute.set(params);
		compute.dispatch(flare_size, ivec2{ 8, 8 });
	}

	const uint directions = streak_direction_count(g_streaks);
	if (directions == 0)
		return;

	PROFILE(L"lens_flare_streaks");
	compute.set_pipeline<PSOS::Post::LensFlareStreak>();

	const float rotation   = g_streak_rotation * Math::pi / 180.0f;
	const int   iterations = streak_iterations(flare_size);
	for (uint d = 0; d < directions; d++)
	{
		const float angle = rotation + 2.0f * Math::pi * d / directions;
		const float2 dir = float2(std::cos(angle), std::sin(angle));

		// source -> A -> B -> A -> ... ping-pong; every level is also summed
		// into `streaks` (the very first write overwrites it).
		float step   = 1.0f;
		float weight = 1.0f;
		for (int it = 0; it < iterations; it++)
		{
			const bool to_a = (it % 2) == 0;

			Slots::Post::LensFlareStreak params;
			params.GetSource()       = it == 0 ? source.texture2D : (to_a ? streak_b.texture2D : streak_a.texture2D);
			params.GetTarget()       = to_a ? streak_a.rwTexture2D : streak_b.rwTexture2D;
			params.GetAccum()        = streaks.rwTexture2D;
			params.GetDirection()    = dir;
			params.GetTexel()        = texel;
			params.GetStep()         = step;
			params.GetAccum_weight() = weight;
			params.GetAccumulate()   = (d > 0 || it > 0) ? 1u : 0u;
			compute.set(params);
			compute.dispatch(flare_size, ivec2{ 8, 8 });

			step   *= c_streak_step_base;
			weight *= g_streak_falloff;
		}
	}
}
