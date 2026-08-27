#include "VSM_impl.hlsl"

// Non-penumbra fallback only (VSM_PENUMBRA off, VSM.ixx's use_vsm_penumbra
// toggle) -- fixed single-tap-per-corner 3x3 hardware-PCF, no blocker
// search/blur at all. When VSM_PENUMBRA is ON, VSM.hlsl's combine_result
// doesn't call this at all -- it samples stage 3's precomputed
// VSM_ShadowResult directly instead (see that file's own VSM_ShadowResolve
// PassNode comment for where the real per-pixel work now lives).
float get_shadow_vsm_simple(VSMConstants c, VSMLighting lighting, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return 1.0;

	int resolved_level = level;
	uint slot = get_vsm_slot(c, lighting, pos_ls, level, resolved_level);
	if (slot == VSM_INVALID_SLOT)
		return 1.0;
	level = resolved_level;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	if (pos_l.z < 0 || pos_l.z > 1 || any(light_tc < 0) || any(light_tc > 1))
		return 1.0;

	// Still averaged over a 3x3 grid of manually offset taps (Texture2DArray
	// has no SampleCmp overload taking a raw texel offset the way SampleLevel
	// does, so int2 offset here comes from re-deriving the UV per tap rather
	// than an offset param) -- four-tap hardware PCF per sample point, times
	// nine sample points, for a wider soft edge than a single hardware-PCF
	// tap alone would give.
	float2 texel_size = 1.0 / float2(c.GetPage_size(), c.GetPage_size());
	float shadow = 0;
	[unroll]
	for (int oy = -1; oy <= 1; oy++)
	{
		[unroll]
		for (int ox = -1; ox <= 1; ox++)
		{
			float2 tc = light_tc + float2(ox, oy) * texel_size;
			shadow += lighting.GetVsm_atlas().SampleCmpLevelZero(vsmShadowSampler, float3(tc, (float)slot), pos_l.z * 0.9999);
		}
	}
	return shadow / 9.0;
}

// Debug: raw sampled atlas depth at the page this world pos maps to (0 if
// out of range). Reversed-Z, clear=0 -- if this is 0 everywhere, nothing was
// ever rasterized into the atlas at that page; a real surface written there
// will read as a small-to-large positive value depending on distance from
// the light.
float get_vsm_debug_raw_depth(VSMConstants c, VSMLighting lighting, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return 0;

	// Unused here (this debug helper never looks anything up by level after
	// resolving slot) -- required by get_vsm_slot's signature regardless.
	int unused_resolved_level = level;
	uint slot = get_vsm_slot(c, lighting, pos_ls, level, unused_resolved_level);
	if (slot == VSM_INVALID_SLOT)
		return 0;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	return lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(light_tc, (float)slot), 0);
}

// Debug heatmap: one flat color per clipmap level, for visually confirming
// page/level coverage (Phase 1a "done" criterion).
float3 get_vsm_debug_color(VSMConstants c, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return float3(0, 0, 0);

	static const float3 palette[8] = {
		float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(1, 1, 0),
		float3(0, 1, 1), float3(1, 0, 1), float3(1, 0.5, 0), float3(0.5, 0, 1)
	};
	return palette[level % 8];
}

// Debug view: same one-flat-color-per-level palette as get_vsm_debug_color
// above, but ALSO darkened on a checkerboard by (page_x + page_y) parity
// within that level -- makes page seams visible as a brightness step, not
// just level boundaries. Built to check whether a visual artifact (e.g. a
// thin line inside an otherwise-uniformly-classified Hi-Z region) actually
// lines up with a real page/level boundary.
float3 get_vsm_debug_page_grid_color(VSMConstants c, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return float3(0, 0, 0);

	float4 info = c.GetLevel_info(level);
	int2 page = int2(floor((pos_ls - info.xy) / max(info.z, 0.0001)));

	static const float3 palette[8] = {
		float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(1, 1, 0),
		float3(0, 1, 1), float3(1, 0, 1), float3(1, 0.5, 0), float3(0.5, 0, 1)
	};
	bool checker = ((page.x + page.y) & 1) != 0;
	return palette[level % 8] * (checker ? 0.5 : 1.0);
}
