// Copyright 2023 Sony Interactive Entertainment.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Adapted from Bend Studio's public screen-space shadow projection code
// (../../denoiser/ss_shadow.hlsl, used unmodified by RTXShadow elsewhere in
// this engine) -- a deliberately SEPARATE copy, not a shared #include. See
// vsm.sig's own VSMScreenSpaceShadowParams comment for why: this copy's
// EarlyOutPixel reads VSM_BlockerSearch's own per-tile ambiguity verdict
// instead of a generic depth-bounds check, and VSM's own quality knobs
// (SurfaceThickness/BilinearThreshold/ShadowContrast/SAMPLE_COUNT below) are
// hardcoded rather than exposed as a shared, runtime-tunable CB -- both
// diverge freely from RTXShadow's own tuning without touching it.

#include "../../common/common.hlsl"
#include "../../autogen/VSMScreenSpaceShadowParams.h"

#define WAVE_SIZE 64
#define SAMPLE_COUNT 32
#define HARD_SHADOW_SAMPLES 16
#define FADE_OUT_SAMPLES 8

#define USE_HALF_PIXEL_OFFSET 1
#define USE_UV_PIXEL_BIAS 1

// Fixed "visual configuration" knobs Bend's own DispatchParameters exposes
// as runtime CB fields -- hardcoded here instead (see this file's own top
// comment), except SurfaceThickness (VSMScreenSpaceShadowParams' own
// surface_thickness field, VSM::vsm_contact_shadow_thickness) -- the reach/
// width a contact shadow reads as is sensitive enough to scene depth scale
// that it needs to be tunable without a rebuild, unlike the others. Same
// starting values SS_Shadow.sig documents as its own defaults.
static const float VSM_SS_BILINEAR_THRESHOLD = 0.02;
static const float VSM_SS_SHADOW_CONTRAST = 4.0;

struct Result
{
	float2 pixel_pos;
	// Continuous lit amount (1 = fully lit, 0 = fully shadowed), same
	// convention as this file's consumer (vsm_shadow_resolve.hlsl's
	// min(shadow, ...)) -- NOT a hard boolean. The original engine's own
	// workgraph-node usage of this same struct (dev/workgraph_test.hlsl)
	// collapses it to a bool itself (that pipeline is a classify+RTX-verify
	// hybrid, not meant to carry Bend's own soft falloff past that point);
	// this copy keeps the real value since a hard 0/1 read is exactly the
	// "no fade" bug this field's own comment used to cause.
	float shadow;
	bool good;
};

// VSM_BlockerSearch's own per-tile "still ambiguous" verdict -- zero means
// every pixel in this 16x16 tile already resolved confidently (lit_tiles/
// dark_tiles/confirmed_lit_tiles), nothing left for this march to add.
// Reversed-Z: raw_z == 0 is sky/no-geometry, excluded the same way every
// other VSM pass excludes it.
static bool EarlyOutPixel(VSMScreenSpaceShadowParams inParameters, int2 pixel_xy, float depth)
{
	if (inParameters.GetAmbiguous_mask().Load(int3(pixel_xy >> 4, 0)) < 0.5)
		return true;

	return depth <= 0.0;
}

// Gets the start pixel coordinates for the pixels in the wavefront.
// Also returns the delta to get to the next pixel after WAVE_COUNT pixels along the ray.
static void ComputeWavefrontExtents(VSMScreenSpaceShadowParams inParameters, int3 inGroupID, uint inGroupThreadID, out float2 outDeltaXY, out float2 outPixelXY, out float outPixelDistance, out bool outMajorAxisX)
{
	float4 light_coordinate = inParameters.GetLight_coordinate();
	int2 wave_offset = inParameters.GetWave_offset();
	int2 xy = inGroupID.yz * WAVE_SIZE + wave_offset;

	float2 light_xy = floor(light_coordinate.xy) + 0.5;
	float2 light_xy_fraction = light_coordinate.xy - light_xy;
	bool reverse_direction = light_coordinate.w > 0.0f;

	int2 sign_xy = sign(xy);
	bool horizontal = abs(xy.x + sign_xy.y) < abs(xy.y - sign_xy.x);

	int2 axis;
	axis.x = horizontal ? (+sign_xy.y) : (0);
	axis.y = horizontal ? (0) : (-sign_xy.x);

	xy = axis * (int)inGroupID.x + xy;
	float2 xy_f = (float2)xy;

	bool x_axis_major = abs(xy_f.x) > abs(xy_f.y);
	float major_axis = x_axis_major ? xy_f.x : xy_f.y;

	float major_axis_start = abs(major_axis);
	float major_axis_end = abs(major_axis) - (float)WAVE_SIZE;

	float ma_light_frac = x_axis_major ? light_xy_fraction.x : light_xy_fraction.y;
	ma_light_frac = major_axis > 0 ? -ma_light_frac : ma_light_frac;

	float2 start_xy = xy_f + light_xy;

	float2 end_xy = lerp(light_coordinate.xy, start_xy, (major_axis_end + ma_light_frac) / (major_axis_start + ma_light_frac));

	float2 xy_delta = (start_xy - end_xy);

	float thread_step = (float)(inGroupThreadID ^ (reverse_direction ? 0 : (WAVE_SIZE - 1)));

	float2 pixel_xy = lerp(start_xy, end_xy, thread_step / (float)WAVE_SIZE);
	float pixel_distance = major_axis_start - thread_step + ma_light_frac;

	outPixelXY = pixel_xy;
	outPixelDistance = pixel_distance;
	outDeltaXY = xy_delta;
	outMajorAxisX = x_axis_major;
}

#define READ_COUNT (SAMPLE_COUNT / WAVE_SIZE + 2)

groupshared float DepthData[READ_COUNT * WAVE_SIZE];
groupshared bool LdsEarlyOut;

Result WriteScreenSpaceShadow(VSMScreenSpaceShadowParams inParameters, int3 inGroupID, int inGroupThreadID)
{
	float2 xy_delta;
	float2 pixel_xy;
	float pixel_distance;
	bool x_axis_major;

	ComputeWavefrontExtents(inParameters, (int3)inGroupID, inGroupThreadID.x, xy_delta, pixel_xy, pixel_distance, x_axis_major);

	float sampling_depth[READ_COUNT];
	float shadowing_depth[READ_COUNT];
	float depth_thickness_scale[READ_COUNT];
	float sample_distance[READ_COUNT];

	float4 light_coordinate = inParameters.GetLight_coordinate();
	const float direction = -light_coordinate.w;
	const float far_depth_value = inParameters.GetFar_depth_value();
	const float near_depth_value = inParameters.GetNear_depth_value();
	const float z_sign = near_depth_value > far_depth_value ? -1 : +1;

	Texture2D<float> depth_tex = inParameters.GetGbuffer().GetDepth();
	float2 dims;
	depth_tex.GetDimensions(dims.x, dims.y);
	float2 inv_dims = 1.0 / dims;

	int i;
	bool is_edge = false;
	bool skip_pixel = false;
	float2 write_xy = floor(pixel_xy);

	Result resultv;
	resultv.pixel_pos = write_xy;
	resultv.shadow = 0.0;
	resultv.good = false;
	[unroll] for (i = 0; i < READ_COUNT; i++)
	{
		float2 read_xy = floor(pixel_xy);
		float minor_axis = x_axis_major ? pixel_xy.y : pixel_xy.x;

		const float edge_skip = 1e20;

		float2 depths;
		float bilinear = frac(minor_axis) - 0.5;

#if USE_HALF_PIXEL_OFFSET
		read_xy += 0.5;
#endif

#if USE_UV_PIXEL_BIAS
		float bias = bilinear > 0 ? 1 : -1;
		float2 offset_xy = float2(x_axis_major ? 0 : bias, x_axis_major ? bias : 0);

		depths.x = depth_tex.SampleLevel(pointBorderSampler, read_xy * inv_dims, 0);
		depths.y = depth_tex.SampleLevel(pointBorderSampler, (read_xy + offset_xy) * inv_dims, 0);
#else
		int bias = bilinear > 0 ? 1 : -1;
		int2 offset_xy = int2(x_axis_major ? 0 : bias, x_axis_major ? bias : 0);

		depths.x = depth_tex.SampleLevel(pointBorderSampler, read_xy * inv_dims, 0);
		depths.y = depth_tex.SampleLevel(pointBorderSampler, read_xy * inv_dims, 0, offset_xy);
#endif

		depth_thickness_scale[i] = abs(far_depth_value - depths.x);

		bool use_point_filter = abs(depths.x - depths.y) > depth_thickness_scale[i] * VSM_SS_BILINEAR_THRESHOLD;

		if (i == 0) is_edge = use_point_filter;

		// Sampling points stay pixel-centered (BilinearSamplingOffsetMode ==
		// false, hardcoded -- see this file's own top comment); shadow
		// depths bias away by the depth gradient across the bilinear
		// sample.
		sampling_depth[i] = depths.x;
		float edge_depth = depths.x;
		float shadow_depth = depths.x + abs(depths.x - depths.y) * z_sign;
		shadowing_depth[i] = use_point_filter ? edge_depth : shadow_depth;

		sample_distance[i] = pixel_distance + (WAVE_SIZE * i) * direction;

		pixel_xy += xy_delta * direction;
	}

	// UseEarlyOut is always on for this copy -- the whole point of
	// EarlyOutPixel reading VSM_AmbiguousMask.
	{
		skip_pixel = EarlyOutPixel(inParameters, (int2)write_xy, sampling_depth[0]);

		bool early_out = WaveActiveAnyTrue(!skip_pixel) == false;

		if (WaveGetLaneCount() == WAVE_SIZE)
		{
			if (early_out == true)
				return resultv;
		}
		else
		{
			LdsEarlyOut = true;

			GroupMemoryBarrierWithGroupSync();

			[branch] if (early_out == false)
				LdsEarlyOut = false;

			GroupMemoryBarrierWithGroupSync();

			[branch] if (LdsEarlyOut)
				return resultv;
		}
	}

	[unroll] for (i = 0; i < READ_COUNT; i++)
	{
		float stored_depth = (shadowing_depth[i] - light_coordinate.z) / sample_distance[i];

		if (i != 0)
			stored_depth = sample_distance[i] > 0 ? stored_depth : 1e10;

		int idx = (i * WAVE_SIZE) + inGroupThreadID.x;
		DepthData[idx] = stored_depth;
	}

	GroupMemoryBarrierWithGroupSync();

	if (skip_pixel)
		return resultv;

	float start_depth = sampling_depth[0];

	start_depth = (start_depth - light_coordinate.z) / sample_distance[0];

	int sample_index = inGroupThreadID.x + 1;

	float4 shadow_value = 1;
	float hard_shadow = 1;

	float depth_scale = min(sample_distance[0] + direction, 1.0 / inParameters.GetSurface_thickness()) * sample_distance[0] / depth_thickness_scale[0];

	start_depth = start_depth * depth_scale - z_sign;

	[unroll] for (i = 0; i < HARD_SHADOW_SAMPLES; i++)
	{
		float depth_delta = abs(start_depth - DepthData[sample_index + i] * depth_scale);
		hard_shadow = min(hard_shadow, depth_delta);
	}

	[unroll] for (i = HARD_SHADOW_SAMPLES; i < SAMPLE_COUNT - FADE_OUT_SAMPLES; i++)
	{
		float depth_delta = abs(start_depth - DepthData[sample_index + i] * depth_scale);
		shadow_value[i & 3] = min(shadow_value[i & 3], depth_delta);
	}

	[unroll] for (i = SAMPLE_COUNT - FADE_OUT_SAMPLES; i < SAMPLE_COUNT; i++)
	{
		float depth_delta = abs(start_depth - DepthData[sample_index + i] * depth_scale);
		const float fade_out = (float)(i + 1 - (SAMPLE_COUNT - FADE_OUT_SAMPLES)) / (float)(FADE_OUT_SAMPLES + 1) * 0.75;
		shadow_value[i & 3] = min(shadow_value[i & 3], depth_delta + fade_out);
	}

	shadow_value = saturate(shadow_value * VSM_SS_SHADOW_CONTRAST + (1 - VSM_SS_SHADOW_CONTRAST));
	hard_shadow = saturate(hard_shadow * VSM_SS_SHADOW_CONTRAST + (1 - VSM_SS_SHADOW_CONTRAST));

	float result = dot(shadow_value, 0.25);
	result = min(hard_shadow, result);

	resultv.good = true;
	resultv.shadow = result;

	return resultv;
}

[numthreads(WAVE_SIZE, 1, 1)]
void CS(uint3 LocalThreadId : SV_GroupThreadID, uint3 WorkGroupId : SV_GroupID)
{
	VSMScreenSpaceShadowParams params = GetVSMScreenSpaceShadowParams();
	Result v = WriteScreenSpaceShadow(params, WorkGroupId, LocalThreadId);
	if (v.good)
		params.GetOutput()[uint2(v.pixel_pos)] = v.shadow;
}
