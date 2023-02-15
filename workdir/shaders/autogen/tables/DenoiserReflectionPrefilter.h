#pragma once
struct DenoiserReflectionPrefilter
{
	uint g_depth_buffer; // Texture2D<float>
	uint g_normal; // Texture2D<float4>
	uint g_average_radiance; // Texture2D<float3>
	uint g_in_radiance; // Texture2D<float4>
	uint g_in_variance; // Texture2D<float>
	uint g_in_sample_count; // Texture2D<float>
	uint g_denoiser_tile_list; // Buffer<uint>
	uint g_out_radiance; // RWTexture2D<float4>
	uint g_out_variance; // RWTexture2D<float>
	uint g_out_sample_count; // RWTexture2D<float>
	Texture2D<float> GetG_depth_buffer() { return ResourceDescriptorHeap[g_depth_buffer]; }
	Texture2D<float4> GetG_normal() { return ResourceDescriptorHeap[g_normal]; }
	Texture2D<float3> GetG_average_radiance() { return ResourceDescriptorHeap[g_average_radiance]; }
	Texture2D<float4> GetG_in_radiance() { return ResourceDescriptorHeap[g_in_radiance]; }
	Texture2D<float> GetG_in_variance() { return ResourceDescriptorHeap[g_in_variance]; }
	Texture2D<float> GetG_in_sample_count() { return ResourceDescriptorHeap[g_in_sample_count]; }
	RWTexture2D<float4> GetG_out_radiance() { return ResourceDescriptorHeap[g_out_radiance]; }
	RWTexture2D<float> GetG_out_variance() { return ResourceDescriptorHeap[g_out_variance]; }
	RWTexture2D<float> GetG_out_sample_count() { return ResourceDescriptorHeap[g_out_sample_count]; }
	Buffer<uint> GetG_denoiser_tile_list() { return ResourceDescriptorHeap[g_denoiser_tile_list]; }
};
