#pragma once
struct DenoiserReflectionCommon
{
	float4x4 g_inv_view_proj; // float4x4
	float4x4 g_proj; // float4x4
	float4x4 g_inv_proj; // float4x4
	float4x4 g_view; // float4x4
	float4x4 g_inv_view; // float4x4
	float4x4 g_prev_view_proj; // float4x4
	uint2 g_buffer_dimensions; // uint2
	float2 g_inv_buffer_dimensions; // float2
	float g_temporal_stability_factor; // float
	float g_depth_buffer_thickness; // float
	float g_roughness_threshold; // float
	float g_temporal_variance_threshold; // float
	uint g_frame_index; // uint
	uint g_max_traversal_intersections; // uint
	uint g_min_traversal_occupancy; // uint
	uint g_most_detailed_mip; // uint
	uint g_samples_per_quad; // uint
	uint g_temporal_variance_guided_tracing_enabled; // uint
	float4x4 GetG_inv_view_proj() { return g_inv_view_proj; }
	float4x4 GetG_proj() { return g_proj; }
	float4x4 GetG_inv_proj() { return g_inv_proj; }
	float4x4 GetG_view() { return g_view; }
	float4x4 GetG_inv_view() { return g_inv_view; }
	float4x4 GetG_prev_view_proj() { return g_prev_view_proj; }
	uint2 GetG_buffer_dimensions() { return g_buffer_dimensions; }
	float2 GetG_inv_buffer_dimensions() { return g_inv_buffer_dimensions; }
	float GetG_temporal_stability_factor() { return g_temporal_stability_factor; }
	float GetG_depth_buffer_thickness() { return g_depth_buffer_thickness; }
	float GetG_roughness_threshold() { return g_roughness_threshold; }
	float GetG_temporal_variance_threshold() { return g_temporal_variance_threshold; }
	uint GetG_frame_index() { return g_frame_index; }
	uint GetG_max_traversal_intersections() { return g_max_traversal_intersections; }
	uint GetG_min_traversal_occupancy() { return g_min_traversal_occupancy; }
	uint GetG_most_detailed_mip() { return g_most_detailed_mip; }
	uint GetG_samples_per_quad() { return g_samples_per_quad; }
	uint GetG_temporal_variance_guided_tracing_enabled() { return g_temporal_variance_guided_tracing_enabled; }
};
