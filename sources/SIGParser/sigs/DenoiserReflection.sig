    

[Bind = DefaultLayout::Instance0]
struct DenoiserReflectionCommon
{
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
}

[Bind = DefaultLayout::Instance1]
struct DenoiserReflectionReproject
{
	Texture2D<float> g_depth_buffer;
	#Texture2D<float> g_roughness;
	Texture2D<float4> g_normal;
	Texture2D<float> g_depth_buffer_history;
	#Texture2D<float> g_roughness_history;
	Texture2D<float4> g_normal_history;

	Texture2D<float4> g_in_radiance;
	Texture2D<float4> g_radiance_history;
	Texture2D<float2> g_motion_vector;

	Texture2D<float3> g_average_radiance_history;
	Texture2D<float> g_variance_history;
	Texture2D<float> g_sample_count_history;
	Texture2D<float2> g_blue_noise_texture;

	RWTexture2D<float3> g_out_reprojected_radiance;
	RWTexture2D<float3> g_out_average_radiance;
	RWTexture2D<float> g_out_variance;
	RWTexture2D<float> g_out_sample_count;

	Buffer<uint> g_denoiser_tile_list;
}

[Bind = DefaultLayout::Instance1]
struct DenoiserReflectionPrefilter
{
	Texture2D<float> g_depth_buffer;
	#Texture2D<float> g_roughness;
	Texture2D<float4> g_normal;
	Texture2D<float3> g_average_radiance;
	Texture2D<float4> g_in_radiance;
	Texture2D<float> g_in_variance;
	Texture2D<float> g_in_sample_count;

	RWTexture2D<float4> g_out_radiance;
	RWTexture2D<float> g_out_variance;
	RWTexture2D<float> g_out_sample_count;

	Buffer<uint> g_denoiser_tile_list;
}


[Bind = DefaultLayout::Instance1]
struct DenoiserReflectionResolve
{
	Texture2D<float4> g_normal;
	Texture2D<float3> g_average_radiance;
	Texture2D<float4> g_in_radiance;
	Texture2D<float4> g_in_reprojected_radiance;
	Texture2D<float> g_in_variance;
	Texture2D<float> g_in_sample_count;

	RWTexture2D<float4> g_out_radiance;
	RWTexture2D<float> g_out_variance;
	RWTexture2D<float> g_out_sample_count;

	Buffer<uint> g_denoiser_tile_list;
}

ComputePSO DenoiserReflectionReproject
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = denoiser/reflection_reproject;
}


ComputePSO DenoiserReflectionPrefilter
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = denoiser/reflection_prefilter;
}


ComputePSO DenoiserReflectionResolve
{
	root = DefaultLayout;

	[EntryPoint = main]
	[Enable16bits]
	compute = denoiser/reflection_resolve;
}
