#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserReflectionResolve
	{
		static constexpr SlotID ID = SlotID::DenoiserReflectionResolve;
		HLSL::Texture2D<float4> g_normal;
		HLSL::Texture2D<float3> g_average_radiance;
		HLSL::Texture2D<float4> g_in_radiance;
		HLSL::Texture2D<float4> g_in_reprojected_radiance;
		HLSL::Texture2D<float> g_in_variance;
		HLSL::Texture2D<float> g_in_sample_count;
		HLSL::Buffer<uint> g_denoiser_tile_list;
		HLSL::RWTexture2D<float4> g_out_radiance;
		HLSL::RWTexture2D<float> g_out_variance;
		HLSL::RWTexture2D<float> g_out_sample_count;
		HLSL::Texture2D<float4>& GetG_normal() { return g_normal; }
		HLSL::Texture2D<float3>& GetG_average_radiance() { return g_average_radiance; }
		HLSL::Texture2D<float4>& GetG_in_radiance() { return g_in_radiance; }
		HLSL::Texture2D<float4>& GetG_in_reprojected_radiance() { return g_in_reprojected_radiance; }
		HLSL::Texture2D<float>& GetG_in_variance() { return g_in_variance; }
		HLSL::Texture2D<float>& GetG_in_sample_count() { return g_in_sample_count; }
		HLSL::RWTexture2D<float4>& GetG_out_radiance() { return g_out_radiance; }
		HLSL::RWTexture2D<float>& GetG_out_variance() { return g_out_variance; }
		HLSL::RWTexture2D<float>& GetG_out_sample_count() { return g_out_sample_count; }
		HLSL::Buffer<uint>& GetG_denoiser_tile_list() { return g_denoiser_tile_list; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(g_normal);
			compiler.compile(g_average_radiance);
			compiler.compile(g_in_radiance);
			compiler.compile(g_in_reprojected_radiance);
			compiler.compile(g_in_variance);
			compiler.compile(g_in_sample_count);
			compiler.compile(g_denoiser_tile_list);
			compiler.compile(g_out_radiance);
			compiler.compile(g_out_variance);
			compiler.compile(g_out_sample_count);
		}
		struct Compiled
		{
			uint g_normal; // Texture2D<float4>
			uint g_average_radiance; // Texture2D<float3>
			uint g_in_radiance; // Texture2D<float4>
			uint g_in_reprojected_radiance; // Texture2D<float4>
			uint g_in_variance; // Texture2D<float>
			uint g_in_sample_count; // Texture2D<float>
			uint g_denoiser_tile_list; // Buffer<uint>
			uint g_out_radiance; // RWTexture2D<float4>
			uint g_out_variance; // RWTexture2D<float>
			uint g_out_sample_count; // RWTexture2D<float>
		};
	};
	#pragma pack(pop)
}
