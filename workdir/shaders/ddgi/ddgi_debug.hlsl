#include "../autogen/FrameInfo.h"
#include "../autogen/tables/DDGIProbes.h"
#include "../autogen/DDGIDebugData.h"
#include "octahedral.hlsl"

// Debug-only screen-space probe visualization (DDGISelectors::show_probes,
// see [[project-ddgi]] planning notes) -- NOT a real GraphicsPSO/instanced
// mesh draw. One thread per probe: project its world position through the
// camera, manually depth-test against GBuffer_DepthMips (reversed-Z: larger
// raw value = closer -- a probe passes if it's at least as close as the
// real geometry already at that pixel, same convention RenderBoxes'
// GREATER_EQUAL depth_func documents), then splat a small flat-colored
// block directly into ResultTexture. No anti-aliasing, no proper point-
// sprite falloff -- this is a "are the probes even there" check, not a
// polished visualization.
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DDGIDebugData data = GetDDGIDebugData();

	// Every cascade shares the same probe_counts (only spacing/atlas
	// offsets differ) -- cascade0 alone is enough to get the shared
	// per-cascade probe count needed to decode which cascade this dispatch
	// index belongs to.
	DDGIInfo infos[5] = { data.GetCascade0(), data.GetCascade1(), data.GetCascade2(), data.GetCascade3(), data.GetCascade4() };

	uint3 probe_counts = infos[0].GetProbe_counts().xyz;
	uint probe_count = probe_counts.x * probe_counts.y * probe_counts.z;

	uint cascade_index = dispatchID.x / probe_count;
	uint probe_index   = dispatchID.x % probe_count;
	if (cascade_index >= 5)
		return;

	const DDGIInfo info = infos[cascade_index];

	DDGIProbes probes = data.GetProbes();
	uint3 probe_coord = probes.ddgi_probe_grid_coord(probe_index, probe_counts);
	float3 world_pos = probes.ddgi_probe_world_pos(probe_coord, info.GetGrid_min().xyz, info.GetProbe_spacing().xyz, float3(0, 0, 0), probe_counts);

	// Not being traced/convolved this frame (DDGIProbeResidencyMark, ddgi.sig,
	// didn't mark it needed) -- skip entirely rather than draw a stale
	// marker, same as this probe just isn't there right now.
	uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, probe_counts);
	if (data.GetProbe_residency()[info.GetCascade_info().x + probe_linear_index] == 0)
		return;

	const FrameInfo frame = GetFrameInfo();
	// mul(matrix, vector), not mul(vector, matrix) -- this codebase's
	// convention throughout (see depth_to_wpos, common.hlsl, doing the
	// inverse transform the same way); the reversed order silently produces
	// a degenerate transform instead of a compile error, which is what
	// squeezed every probe toward screen center.
	float4 clip = mul(frame.GetCamera().GetViewProj(), float4(world_pos, 1));
	if (clip.w <= 0.0)
		return;

	float3 ndc = clip.xyz / clip.w;
	if (any(abs(ndc.xy) > 1.0) || ndc.z < 0.0 || ndc.z > 1.0)
		return;

	float2 screen_uv = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);

	uint width, height;
	data.GetTarget().GetDimensions(width, height);
	int2 pixel = int2(screen_uv * float2(width, height));

	uint depth_w, depth_h;
	data.GetDepth().GetDimensions(depth_w, depth_h);
	int2 depth_pixel = int2(screen_uv * float2(depth_w, depth_h));
	if (depth_pixel.x < 0 || depth_pixel.y < 0 || (uint)depth_pixel.x >= depth_w || (uint)depth_pixel.y >= depth_h)
		return;

	float scene_depth = data.GetDepth()[depth_pixel];

	// Small bias so a probe embedded exactly at a surface (e.g. resting on
	// a floor) doesn't flicker between visible/occluded.
	const float bias = 0.0005;
	if (ndc.z + bias < scene_depth)
		return;

	// Color by this probe's own convolved irradiance, sampled toward the
	// camera -- see DDGIDebugData's own comment (ddgi.sig) for why that
	// direction, not the surface-facing one a real shading point would use.
	uint texel_size = info.GetAtlas_info().x;
	float3 view_dir = normalize(frame.GetCamera().GetPosition().xyz - world_pos);
	float2 uv = ddgi_oct_encode(view_dir) * 0.5 + 0.5;
	// ddgi_atlas_origin gives the (x,z) plane position; probe_coord.y plus
	// this cascade's own slice offset gives the array slice (same fix
	// ddgi_sample_irradiance itself needed, ddgi_sample.hlsl).
	uint2 atlas_origin = probes.ddgi_atlas_origin(probe_coord, texel_size);
	uint slice = probes.ddgi_atlas_array_slice(probe_coord.y, info.GetCascade_info().y);
	uint2 sample_texel = atlas_origin + min(uint2(uv * texel_size), texel_size - 1);
	float3 irradiance = data.GetProbe_irradiance()[uint3(sample_texel, slice)].rgb;

	// Debug-only boost + floor so a genuinely dim/zero probe still shows up
	// as a visible (dark grey, not invisible-black) marker -- not
	// representative of the real irradiance magnitude.
	float3 debug_color = irradiance  + float3(0.01, 0.01, 0.01)/4;

	// Bigger markers for coarser cascades -- with all 5 drawn at once, size
	// is what actually lets you tell which cascade a given dot belongs to
	// (color already encodes irradiance, not cascade).
	int radius = 2 + (int)cascade_index;
	float4 color = float4(debug_color, 1);
	for (int y = -radius; y <= radius; y++)
		for (int x = -radius; x <= radius; x++)
		{
			int2 p = pixel + int2(x, y);
			if (p.x < 0 || p.y < 0 || (uint)p.x >= width || (uint)p.y >= height)
				continue;
			data.GetTarget()[p] = color;
		}
}
