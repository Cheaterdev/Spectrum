#include "../common/common.hlsl"
#include "../common/shading_model.hlsl"

#include "../autogen/FrameInfo.h"
#include "../autogen/SSSBlurParams.h"
#include "../autogen/SSSTilesRead.h"

// Screen-space subsurface scattering, one axis per dispatch (see sss.prism for
// where it runs and why). Only SUBSURFACE pixels are touched.
//
// The blur works on irradiance: the first (horizontal) pass divides the lit
// radiance by albedo, so the albedo's own texture detail is not blurred away,
// and the second multiplies it back. Both use the same albedo floor, so a
// pixel with nothing to blur comes back exactly as it went in.

static const GBuffer gbuffer = GetSSSBlurParams().GetGbuffer();

static const int   TAPS = 16;          // per side, so 33 taps per axis
static const float ALBEDO_FLOOR = 0.05;
// Taps are at most 3 px apart (3 * MAX_SIGMA_PX / TAPS): sparser than that and a wide
// kernel turns into visible copies of the image instead of a blur.
static const float MAX_SIGMA_PX = 16.0;
// A Gaussian of this sigma stands in for the mean free path: ~all of its energy
// lies within the radius.
static const float SIGMA_PER_RADIUS = 0.5;

float view_depth(float raw_z, Camera camera)
{
	return camera.GetProj()._34 * raw_z / (raw_z - camera.GetProj()._33);
}

void blur_axis(uint2 pixel, int2 dir, bool from_radiance, bool final_pass)
{
	float2 dims;
	GetSSSBlurParams().GetTarget().GetDimensions(dims.x, dims.y);

	int2 tc = int2(pixel);
	if (any(tc >= int2(dims)))
		return;

	float raw_z = gbuffer.GetDepth()[tc];
	if (raw_z == 0)
		return;

	float4 material = gbuffer.GetMaterial()[tc];
	if (material_model(material) != SHADING_MODEL_SUBSURFACE)
		return;

	Camera camera = GetFrameInfo().GetCamera();
	float  z = view_depth(raw_z, camera);
	float3 n = normalize(gbuffer.GetNormals()[tc].xyz * 2 - 1);

	float3 albedo = max(gbuffer.GetAlbedo()[tc].rgb, ALBEDO_FLOOR);

	// Pixels per world unit at this depth, along the blur axis.
	float focal = dir.x != 0 ? 0.5 * dims.x * camera.GetProj()._11 : 0.5 * dims.y * camera.GetProj()._22;
	float3 sigma_world = sss_decode_radius(material_params(material)) * SIGMA_PER_RADIUS;
	float3 sigma_px = min(sigma_world * focal / max(z, 1e-3), MAX_SIGMA_PX);
	float  max_sigma_px = max(sigma_px.x, max(sigma_px.y, sigma_px.z));
	float  max_sigma_world = max(sigma_world.x, max(sigma_world.y, sigma_world.z));

	float3 center = GetSSSBlurParams().GetSource()[tc].rgb;
	if (from_radiance)
		center /= albedo;

	float3 result = center;

	// Under a quarter of a pixel nothing is gathered beyond the centre tap.
	if (max_sigma_px > 0.25)
	{
		// Taps cover +-3 sigma of the widest channel; a channel with a narrower
		// sigma just gets small weights on the outer ones.
		float step = max(1.0, 3.0 * max_sigma_px / TAPS);
		// Hard cut on the depth gap: past this the neighbour is another surface.
		float depth_tolerance = max(2.0 * max_sigma_world, 0.02);

		float3 sum = center;
		float3 weight_sum = 1.0;

		[loop]
		for (int i = -TAPS; i <= TAPS; i++)
		{
			if (i == 0)
				continue;

			int2 p = tc + dir * (int)round(i * step);
			if (any(p < 0) || any(p >= int2(dims)))
				continue;

			float raw_zn = gbuffer.GetDepth()[p];
			if (raw_zn == 0)
				continue;

			if (material_model(gbuffer.GetMaterial()[p]) != SHADING_MODEL_SUBSURFACE)
				continue;

			if (abs(view_depth(raw_zn, camera) - z) > depth_tolerance)
				continue;

			// Do not blur around creases.
			if (dot(n, normalize(gbuffer.GetNormals()[p].xyz * 2 - 1)) < 0.5)
				continue;

			float3 e = GetSSSBlurParams().GetSource()[p].rgb;
			if (from_radiance)
				e /= max(gbuffer.GetAlbedo()[p].rgb, ALBEDO_FLOOR);

			float d = i * step;
			float3 w = exp(-0.5 * d * d / max(sigma_px * sigma_px, 1e-4));

			sum += e * w;
			weight_sum += w;
		}

		result = sum / weight_sum;
	}

	if (!final_pass)
	{
		GetSSSBlurParams().GetTarget()[tc] = float4(result, 1);
		return;
	}

	float3 lit = result * albedo;

	// DLSS-RR: RTXCombine left only the diffuse part in the target and parked the
	// specular part separately.
	if (GetSSSBlurParams().GetAdd_specular() != 0)
		lit += GetSSSBlurParams().GetSpecular()[tc].rgb;

	GetSSSBlurParams().GetTarget()[tc] = float4(lit, 1);
}

// Dispatched indirectly, one 16x16 group per tile SSS_Classify listed:
// SV_GroupID.x indexes the list.
uint2 tile_pixel(uint3 groupID, uint3 groupThreadID)
{
	return GetSSSTilesRead().GetTiles()[groupID.x] * 16 + groupThreadID.xy;
}

[numthreads(16, 16, 1)]
void CS_H(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	blur_axis(tile_pixel(groupID, groupThreadID), int2(1, 0), true, false);
}

[numthreads(16, 16, 1)]
void CS_V(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	blur_axis(tile_pixel(groupID, groupThreadID), int2(0, 1), false, true);
}
