#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserReflectionReproject
	{
		static constexpr SlotID ID = SlotID::DenoiserReflectionReproject;
		HLSL::Texture2D<float> g_depth_buffer;
		HLSL::Texture2D<float4> g_normal;
		HLSL::Texture2D<float> g_depth_buffer_history;
		HLSL::Texture2D<float4> g_normal_history;
		HLSL::Texture2D<float4> g_in_radiance;
		HLSL::Texture2D<float4> g_radiance_history;
		HLSL::Texture2D<float2> g_motion_vector;
		HLSL::Texture2D<float3> g_average_radiance_history;
		HLSL::Texture2D<float> g_variance_history;
		HLSL::Texture2D<float> g_sample_count_history;
		HLSL::Texture2D<float2> g_blue_noise_texture;
		HLSL::Buffer<uint> g_denoiser_tile_list;
		HLSL::RWTexture2D<float3> g_out_reprojected_radiance;
		HLSL::RWTexture2D<float3> g_out_average_radiance;
		HLSL::RWTexture2D<float> g_out_variance;
		HLSL::RWTexture2D<float> g_out_sample_count;
		HLSL::Texture2D<float>& GetG_depth_buffer() { return g_depth_buffer; }
		HLSL::Texture2D<float4>& GetG_normal() { return g_normal; }
		HLSL::Texture2D<float>& GetG_depth_buffer_history() { return g_depth_buffer_history; }
		HLSL::Texture2D<float4>& GetG_normal_history() { return g_normal_history; }
		HLSL::Texture2D<float4>& GetG_in_radiance() { return g_in_radiance; }
		HLSL::Texture2D<float4>& GetG_radiance_history() { return g_radiance_history; }
		HLSL::Texture2D<float2>& GetG_motion_vector() { return g_motion_vector; }
		HLSL::Texture2D<float3>& GetG_average_radiance_history() { return g_average_radiance_history; }
		HLSL::Texture2D<float>& GetG_variance_history() { return g_variance_history; }
		HLSL::Texture2D<float>& GetG_sample_count_history() { return g_sample_count_history; }
		HLSL::Texture2D<float2>& GetG_blue_noise_texture() { return g_blue_noise_texture; }
		HLSL::RWTexture2D<float3>& GetG_out_reprojected_radiance() { return g_out_reprojected_radiance; }
		HLSL::RWTexture2D<float3>& GetG_out_average_radiance() { return g_out_average_radiance; }
		HLSL::RWTexture2D<float>& GetG_out_variance() { return g_out_variance; }
		HLSL::RWTexture2D<float>& GetG_out_sample_count() { return g_out_sample_count; }
		HLSL::Buffer<uint>& GetG_denoiser_tile_list() { return g_denoiser_tile_list; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(g_depth_buffer);
			compiler.compile(g_normal);
			compiler.compile(g_depth_buffer_history);
			compiler.compile(g_normal_history);
			compiler.compile(g_in_radiance);
			compiler.compile(g_radiance_history);
			compiler.compile(g_motion_vector);
			compiler.compile(g_average_radiance_history);
			compiler.compile(g_variance_history);
			compiler.compile(g_sample_count_history);
			compiler.compile(g_blue_noise_texture);
			compiler.compile(g_denoiser_tile_list);
			compiler.compile(g_out_reprojected_radiance);
			compiler.compile(g_out_average_radiance);
			compiler.compile(g_out_variance);
			compiler.compile(g_out_sample_count);
		}
		struct Compiled
		{
			uint g_depth_buffer; // Texture2D<float>
			uint g_normal; // Texture2D<float4>
			uint g_depth_buffer_history; // Texture2D<float>
			uint g_normal_history; // Texture2D<float4>
			uint g_in_radiance; // Texture2D<float4>
			uint g_radiance_history; // Texture2D<float4>
			uint g_motion_vector; // Texture2D<float2>
			uint g_average_radiance_history; // Texture2D<float3>
			uint g_variance_history; // Texture2D<float>
			uint g_sample_count_history; // Texture2D<float>
			uint g_blue_noise_texture; // Texture2D<float2>
			uint g_denoiser_tile_list; // Buffer<uint>
			uint g_out_reprojected_radiance; // RWTexture2D<float3>
			uint g_out_average_radiance; // RWTexture2D<float3>
			uint g_out_variance; // RWTexture2D<float>
			uint g_out_sample_count; // RWTexture2D<float>
		};
	};
	#pragma pack(pop)
}
