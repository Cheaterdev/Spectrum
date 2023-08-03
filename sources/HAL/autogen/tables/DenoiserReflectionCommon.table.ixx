export module HAL:Autogen.Tables.DenoiserReflectionCommon;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserReflectionCommon
	{
		static constexpr SlotID ID = SlotID::DenoiserReflectionCommon;
		float4x4 g_inv_view_proj;
		float4x4 g_proj;
		float4x4 g_inv_proj;
		float4x4 g_view;
		float4x4 g_inv_view;
		float4x4 g_prev_view_proj;
		uint2 g_buffer_dimensions;
		float2 g_inv_buffer_dimensions;
		float g_temporal_stability_factor;
		float g_depth_buffer_thickness;
		float g_roughness_threshold;
		float g_temporal_variance_threshold;
		uint g_frame_index;
		uint g_max_traversal_intersections;
		uint g_min_traversal_occupancy;
		uint g_most_detailed_mip;
		uint g_samples_per_quad;
		uint g_temporal_variance_guided_tracing_enabled;
		float4x4& GetG_inv_view_proj() { return g_inv_view_proj; }
		float4x4& GetG_proj() { return g_proj; }
		float4x4& GetG_inv_proj() { return g_inv_proj; }
		float4x4& GetG_view() { return g_view; }
		float4x4& GetG_inv_view() { return g_inv_view; }
		float4x4& GetG_prev_view_proj() { return g_prev_view_proj; }
		uint2& GetG_buffer_dimensions() { return g_buffer_dimensions; }
		float2& GetG_inv_buffer_dimensions() { return g_inv_buffer_dimensions; }
		float& GetG_temporal_stability_factor() { return g_temporal_stability_factor; }
		float& GetG_depth_buffer_thickness() { return g_depth_buffer_thickness; }
		float& GetG_roughness_threshold() { return g_roughness_threshold; }
		float& GetG_temporal_variance_threshold() { return g_temporal_variance_threshold; }
		uint& GetG_frame_index() { return g_frame_index; }
		uint& GetG_max_traversal_intersections() { return g_max_traversal_intersections; }
		uint& GetG_min_traversal_occupancy() { return g_min_traversal_occupancy; }
		uint& GetG_most_detailed_mip() { return g_most_detailed_mip; }
		uint& GetG_samples_per_quad() { return g_samples_per_quad; }
		uint& GetG_temporal_variance_guided_tracing_enabled() { return g_temporal_variance_guided_tracing_enabled; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(g_inv_view_proj);
			compiler.compile(g_proj);
			compiler.compile(g_inv_proj);
			compiler.compile(g_view);
			compiler.compile(g_inv_view);
			compiler.compile(g_prev_view_proj);
			compiler.compile(g_buffer_dimensions);
			compiler.compile(g_inv_buffer_dimensions);
			compiler.compile(g_temporal_stability_factor);
			compiler.compile(g_depth_buffer_thickness);
			compiler.compile(g_roughness_threshold);
			compiler.compile(g_temporal_variance_threshold);
			compiler.compile(g_frame_index);
			compiler.compile(g_max_traversal_intersections);
			compiler.compile(g_min_traversal_occupancy);
			compiler.compile(g_most_detailed_mip);
			compiler.compile(g_samples_per_quad);
			compiler.compile(g_temporal_variance_guided_tracing_enabled);
		}
		using Compiled = DenoiserReflectionCommon;
		};
		#pragma pack(pop)
	}
