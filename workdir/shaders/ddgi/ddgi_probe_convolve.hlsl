#include "../autogen/tables/DDGIProbes.h"
#include "../autogen/DDGIProbeConvolveData.h"
#include "octahedral.hlsl"

// v1 (see [[project-ddgi]] planning notes): cosine-weighted convolution of a
// probe's own traced radiance texels into one irradiance texel, plus a
// mean/mean-square hit-distance visibility texel (chebyshev depth-test input
// for the per-pixel probe sample landing in a later plan step). Naive
// O(texel_size^2) loop per output texel, not AC Shadows' LDS-prefiltered
// version -- deferred, see plan.
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const DDGIProbeConvolveData data = GetDDGIProbeConvolveData();
	// Dispatch size is one cascade's own DDGI_AtlasWidth x DDGI_AtlasHeight
	// -- LOCAL to this cascade's slice of the shared, wider atlas. All the
	// octahedral/probe-cell math below stays in this local space; only the
	// final texture reads/writes add this cascade's own X-offset
	// (DDGIInfo::cascade_info.y, see its own comment, ddgi.sig).
	uint2 local_texel = dispatchID.xy;
	uint texel_size = data.GetInfo().GetAtlas_info().x;
	uint atlas_x_offset = data.GetInfo().GetCascade_info().y;

	// No bound buffer -- DDGIProbes here is only ever used for its pure
	// coordinate-math helpers (ddgi_atlas_probe_coord/ddgi_atlas_local_uv,
	// neither reads `this`), see DDGIProbeConvolveData's own comment (ddgi.sig)
	// for why the actual probe buffer isn't plumbed into this pass at all.
	DDGIProbes probes;
	float2 out_uv = probes.ddgi_atlas_local_uv(local_texel, texel_size);
	float3 out_dir = ddgi_oct_decode(out_uv);

	uint2 local_probe_origin = local_texel - (local_texel % texel_size);

	float3 radiance_sum = 0;
	float weight_sum = 0;
	float dist_sum = 0;
	float dist2_sum = 0;
	float depth_weight_sum = 0;

	for (uint y = 0; y < texel_size; y++)
	{
		for (uint x = 0; x < texel_size; x++)
		{
			uint2 local_sample_texel = local_probe_origin + uint2(x, y);
			float2 sample_uv = probes.ddgi_atlas_local_uv(local_sample_texel, texel_size);
			float3 sample_dir = ddgi_oct_decode(sample_uv);

			float cos_theta = dot(out_dir, sample_dir);
			float weight = max(cos_theta, 0.0);
			if (weight <= 0.0)
				continue;

			uint2 global_sample_texel = local_sample_texel + uint2(atlas_x_offset, 0);
			radiance_sum += data.GetProbe_radiance()[global_sample_texel].rgb * weight;
			weight_sum += weight;

			// Distance/visibility needs a much SHARPER, direction-specific
			// kernel than irradiance's wide diffuse lobe -- a hit a few
			// units away in one direction and a sky miss (dist=100000,
			// MyMissShader) a few directions over must NOT blend, or the
			// resulting variance is dominated by the miss outlier and the
			// chebyshev test below (variance/(variance+diff^2)) collapses
			// to ~1.0 (never occluded) regardless of the real occluder --
			// confirmed the hard way as the actual cause of oversized light
			// leaks, not just the probe/backface weighting. pow(.,64) is
			// the standard DDGI-paper sharpening exponent for this.
			float depth_weight = pow(saturate(cos_theta), 64.0);
			float hit_dist = data.GetProbe_gbuffer()[global_sample_texel].w;
			dist_sum  += hit_dist * depth_weight;
			dist2_sum += hit_dist * hit_dist * depth_weight;
			depth_weight_sum += depth_weight;
		}
	}

	float3 irradiance = weight_sum > 0.0 ? radiance_sum / weight_sum : float3(0, 0, 0);
	float mean_dist   = depth_weight_sum > 0.0 ? dist_sum / depth_weight_sum : 0.0;
	float mean_dist2  = depth_weight_sum > 0.0 ? dist2_sum / depth_weight_sum : 0.0;

	uint2 global_texel = local_texel + uint2(atlas_x_offset, 0);
	data.GetProbe_irradiance()[global_texel] = float4(irradiance, 1);
	data.GetProbe_visibility()[global_texel] = float2(mean_dist, mean_dist2);
}
