module Graphics:ReflectionDenoiser;

import <RenderSystem.h>;

import :FrameGraphContext;
import :MipMapGenerator;
import HAL;


using namespace FrameGraph;
using namespace HAL;
namespace
{
	constexpr uint32_t DivRoundUp(uint32_t a, uint32_t b)
	{
		return (a + b - 1) / b;
	}
}

ReflectionDenoiser::ReflectionDenoiser()
{

}



void ReflectionDenoiser::generate(Graph& graph)
{

	auto& frame = graph.get_context<ViewportInfo>();
	auto size = frame.frame_size;
	uint2 small_size = float2(DivRoundUp(size.x, 8), DivRoundUp(size.y, 8));
	{
		struct ReflectionDenoiser_ReprojectData
		{
			Handlers::Texture H(GBuffer_DepthPrev);
			Handlers::Texture H(GBuffer_NormalsPrev);
			//Handlers::Texture H(GBuffer_SpecularPrev);

			Handlers::Texture H(GBuffer_Depth);
			Handlers::Texture H(GBuffer_Normals);
			//	Handlers::Texture H(GBuffer_Specular);

			Handlers::Texture H(GBuffer_Speed);

			Handlers::Texture H(VoxelReflectionNoise);
			Handlers::Texture H(ReflectionDenoiser_RadiancePrev);

			Handlers::Texture H(ReflectionDenoiser_AverageRadiance);
			Handlers::Texture H(ReflectionDenoiser_AverageRadiancePrev);

			Handlers::Texture H(ReflectionDenoiser_Variance);
			Handlers::Texture H(ReflectionDenoiser_VariancePrev);

			Handlers::Texture H(ReflectionDenoiser_SampleCount);
			Handlers::Texture H(ReflectionDenoiser_SampleCountPrev);

			Handlers::Texture H(ReflectionDenoiser_ReprojectedRadiance);
				Handlers::Texture H(BlueNoise);
		};

		graph.pass<ReflectionDenoiser_ReprojectData>("ReflectionDenoiser_Reproject", [this, &graph, size, small_size](ReflectionDenoiser_ReprojectData& data, TaskBuilder& builder) {
			builder.need(data.GBuffer_DepthPrev, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_NormalsPrev, ResourceFlags::ComputeRead);
			//	builder.need(data.GBuffer_SpecularPrev, ResourceFlags::ComputeRead);

			builder.need(data.GBuffer_Depth, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Normals, ResourceFlags::ComputeRead);
			//builder.need(data.GBuffer_Specular, ResourceFlags::ComputeRead);

			builder.need(data.GBuffer_Speed, ResourceFlags::ComputeRead);

				builder.need(data.BlueNoise, ResourceFlags::ComputeRead);


			builder.need(data.VoxelReflectionNoise, ResourceFlags::UnorderedAccess);

			//	builder.create(data.ReflectionDenoiser_Radiance, { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 }, ResourceFlags::UnorderedAccess|ResourceFlags::Required);
			builder.create(data.ReflectionDenoiser_RadiancePrev, { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 ,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_AverageRadiance, { ivec3(small_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_AverageRadiancePrev, { ivec3(small_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_Variance, { ivec3(size, 0), HAL::Format::R16_FLOAT, 1 ,1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_VariancePrev, { ivec3(size, 0), HAL::Format::R16_FLOAT, 1 ,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_SampleCount, { ivec3(size, 0), HAL::Format::R16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_SampleCountPrev, { ivec3(size, 0), HAL::Format::R16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_ReprojectedRadiance, { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess );


			return true;
			}, [this, &graph, size](ReflectionDenoiser_ReprojectData& data, FrameContext& _context) {
				auto& list = *_context.get_list();
				auto& compute = list.get_compute();
				auto& cam = graph.get_context<CameraInfo>();
				auto& copy = _context.get_list()->get_copy();


				if (data.ReflectionDenoiser_RadiancePrev.is_new())
				{
					list.clear_uav(data.ReflectionDenoiser_RadiancePrev->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_AverageRadiance->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_AverageRadiancePrev->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_Variance->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_VariancePrev->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_SampleCount->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_SampleCountPrev->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ReflectionDenoiser_ReprojectedRadiance->rwTexture2D, vec4(0, 0, 0, 0));
				}



			

				{
					Slots::DenoiserReflectionCommon common_data;

					common_data.g_buffer_dimensions = size;
					common_data.g_inv_buffer_dimensions = float2(1.0f, 1.0f) / size;

					common_data.g_view = cam.cam->camera_cb.current.GetView();
					common_data.g_inv_view = cam.cam->camera_cb.current.GetInvView();
					common_data.g_prev_view_proj = cam.cam->camera_cb.prev.GetViewProj();

					common_data.g_inv_proj = cam.cam->camera_cb.current.GetInvProj();
					common_data.g_proj = cam.cam->camera_cb.current.GetProj();
					common_data.g_inv_view_proj = cam.cam->camera_cb.current.GetInvViewProj();

					common_data.g_temporal_stability_factor = 0.7f;
					common_data.g_depth_buffer_thickness = 0.015f;
					common_data.g_roughness_threshold = 0.2f;
					common_data.g_temporal_variance_threshold = 0;
					common_data.g_frame_index = 100;
					// common_data.g_max_traversal_intersections;
					//  common_data.g_min_traversal_occupancy;
					common_data.g_most_detailed_mip = 0;
					common_data.g_samples_per_quad = 1;
					common_data.g_temporal_variance_guided_tracing_enabled = true;
					compute.set(common_data);
				}



				{
						compute.set_pipeline<PSOS::DenoiserReflectionReproject>();
					{
						Slots::DenoiserReflectionReproject reproject_data;

						reproject_data.g_depth_buffer = data.GBuffer_Depth->texture2D;
						//	reproject_data.g_roughness= data.GBuffer_Specular->texture2D;
						reproject_data.g_normal = data.GBuffer_Normals->texture2D;
						reproject_data.g_depth_buffer_history = data.GBuffer_DepthPrev->texture2D;
						//		reproject_data.g_roughness_history= data.GBuffer_SpecularPrev->texture2D;
						reproject_data.g_normal_history = data.GBuffer_NormalsPrev->texture2D;

						reproject_data.g_in_radiance = data.VoxelReflectionNoise->texture2D;
						reproject_data.g_radiance_history = data.ReflectionDenoiser_RadiancePrev->texture2D;
						reproject_data.g_motion_vector = data.GBuffer_Speed->texture2D;

						reproject_data.g_average_radiance_history = data.ReflectionDenoiser_AverageRadiancePrev->texture2D;
						reproject_data.g_variance_history = data.ReflectionDenoiser_VariancePrev->texture2D;
						reproject_data.g_sample_count_history = data.ReflectionDenoiser_SampleCountPrev->texture2D;
						reproject_data.g_blue_noise_texture= data.BlueNoise->texture2D;

						reproject_data.g_out_reprojected_radiance = data.ReflectionDenoiser_ReprojectedRadiance->rwTexture2D;
						reproject_data.g_out_average_radiance = data.ReflectionDenoiser_AverageRadiance->rwTexture2D;
						reproject_data.g_out_variance = data.ReflectionDenoiser_Variance->rwTexture2D;
						reproject_data.g_out_sample_count = data.ReflectionDenoiser_SampleCount->rwTexture2D;
						compute.set(reproject_data);
					}

					compute.dispatch(uint3(size, 1));

				}


				{
						compute.set_pipeline<PSOS::DenoiserReflectionPrefilter>();
					{
						Slots::DenoiserReflectionPrefilter prefilter_data;
						
						prefilter_data.g_depth_buffer = data.GBuffer_Depth->texture2D;
						prefilter_data.g_normal = data.GBuffer_Normals->texture2D;
						prefilter_data.g_average_radiance = data.ReflectionDenoiser_AverageRadiance->texture2D;
						
						prefilter_data.g_in_radiance = data.VoxelReflectionNoise->texture2D; //?????
						prefilter_data.g_in_variance = data.ReflectionDenoiser_Variance->texture2D;
				
						prefilter_data.g_in_sample_count = data.ReflectionDenoiser_SampleCount->texture2D;
				




						prefilter_data.g_out_radiance = data.ReflectionDenoiser_RadiancePrev->rwTexture2D;
						prefilter_data.g_out_sample_count = data.ReflectionDenoiser_SampleCountPrev->rwTexture2D;
						prefilter_data.g_out_variance = data.ReflectionDenoiser_VariancePrev->rwTexture2D;
						compute.set(prefilter_data);
					}

					compute.dispatch(uint3(size, 1));

				}



				
				{
						compute.set_pipeline<PSOS::DenoiserReflectionResolve>();
					{
						Slots::DenoiserReflectionResolve resolve_data;
						



						resolve_data.g_normal = data.GBuffer_Normals->texture2D;
						resolve_data.g_average_radiance = data.ReflectionDenoiser_AverageRadiance->texture2D;
						
						resolve_data.g_in_radiance = data.VoxelReflectionNoise->texture2D;
						resolve_data.g_in_reprojected_radiance = data.ReflectionDenoiser_ReprojectedRadiance->texture2D; 


						resolve_data.g_in_variance = data.ReflectionDenoiser_VariancePrev->texture2D;
				
						resolve_data.g_in_sample_count = data.ReflectionDenoiser_SampleCountPrev->texture2D;
				




						resolve_data.g_out_radiance =data.VoxelReflectionNoise->rwTexture2D;
						resolve_data.g_out_sample_count = data.ReflectionDenoiser_SampleCount->rwTexture2D;
						resolve_data.g_out_variance = data.ReflectionDenoiser_Variance->rwTexture2D;
						compute.set(resolve_data);
					}

					compute.dispatch(uint3(size, 1));

				}









				copy.copy_resource(data.ReflectionDenoiser_SampleCountPrev->resource, data.ReflectionDenoiser_SampleCount->resource);
				copy.copy_resource(data.ReflectionDenoiser_VariancePrev->resource, data.ReflectionDenoiser_Variance->resource);
				copy.copy_resource(data.ReflectionDenoiser_RadiancePrev->resource, data.VoxelReflectionNoise->resource);
				copy.copy_resource(data.ReflectionDenoiser_AverageRadiancePrev->resource, data.ReflectionDenoiser_AverageRadiance->resource);


			}, PassFlags::Compute);


	}
}

