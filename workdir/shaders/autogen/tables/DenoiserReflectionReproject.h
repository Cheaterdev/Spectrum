#pragma once
struct DenoiserReflectionReproject
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
	Texture2D<float> GetG_depth_buffer() { return ResourceDescriptorHeap[g_depth_buffer]; }
	Texture2D<float4> GetG_normal() { return ResourceDescriptorHeap[g_normal]; }
	Texture2D<float> GetG_depth_buffer_history() { return ResourceDescriptorHeap[g_depth_buffer_history]; }
	Texture2D<float4> GetG_normal_history() { return ResourceDescriptorHeap[g_normal_history]; }
	Texture2D<float4> GetG_in_radiance() { return ResourceDescriptorHeap[g_in_radiance]; }
	Texture2D<float4> GetG_radiance_history() { return ResourceDescriptorHeap[g_radiance_history]; }
	Texture2D<float2> GetG_motion_vector() { return ResourceDescriptorHeap[g_motion_vector]; }
	Texture2D<float3> GetG_average_radiance_history() { return ResourceDescriptorHeap[g_average_radiance_history]; }
	Texture2D<float> GetG_variance_history() { return ResourceDescriptorHeap[g_variance_history]; }
	Texture2D<float> GetG_sample_count_history() { return ResourceDescriptorHeap[g_sample_count_history]; }
	Texture2D<float2> GetG_blue_noise_texture() { return ResourceDescriptorHeap[g_blue_noise_texture]; }
	RWTexture2D<float3> GetG_out_reprojected_radiance() { return ResourceDescriptorHeap[g_out_reprojected_radiance]; }
	RWTexture2D<float3> GetG_out_average_radiance() { return ResourceDescriptorHeap[g_out_average_radiance]; }
	RWTexture2D<float> GetG_out_variance() { return ResourceDescriptorHeap[g_out_variance]; }
	RWTexture2D<float> GetG_out_sample_count() { return ResourceDescriptorHeap[g_out_sample_count]; }
	Buffer<uint> GetG_denoiser_tile_list() { return ResourceDescriptorHeap[g_denoiser_tile_list]; }
};
