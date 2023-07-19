module Graphics:ShadowDenoiser;

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

constexpr uint32_t k_tileSizeX = 8;
constexpr uint32_t k_tileSizeY = 4;

ShadowDenoiser::ShadowDenoiser()
{

}



void ShadowDenoiser::generate(Graph& graph)
{

	auto& frame = graph.get_context<ViewportInfo>();
	auto size = frame.frame_size;
	uint32_t const xTiles = DivRoundUp(size.x, k_tileSizeX);
	uint32_t const yTiles = DivRoundUp(size.y, k_tileSizeY);

	uint32_t const tileCount = xTiles * yTiles;

	//size/=2;

	{
		struct ShadowDenoiser_PrepareData
		{
			Handlers::Texture H(RTXDebug);
			Handlers::StructuredBuffer<uint32_t> H(ShadowDenoiser_TileBuffer);
		};

		graph.pass<ShadowDenoiser_PrepareData>("ShadowDenoiser_Prepare", [this, &graph, tileCount](ShadowDenoiser_PrepareData& data, TaskBuilder& builder) {
			builder.need(data.RTXDebug, ResourceFlags::ComputeRead);
			builder.create(data.ShadowDenoiser_TileBuffer, { tileCount }, ResourceFlags::UnorderedAccess);

			return true;
			}, [this, &graph, size](ShadowDenoiser_PrepareData& data, FrameContext& _context) {
				auto& list = *_context.get_list();
				auto& compute = list.get_compute();

				compute.set_pipeline<PSOS::DenoiserShadow_Prepare>();
				{
					Slots::DenoiserShadow_Prepare prepare_data;

					prepare_data.GetBufferDimensions() = size;
					prepare_data.GetT2d_hitMaskResults() = data.RTXDebug->texture2D;
					prepare_data.GetRwsb_shadowMask() = data.ShadowDenoiser_TileBuffer->rwStructuredBuffer;
					compute.set(prepare_data);
				}
	//			if(!GetAsyncKeyState('5'))
				compute.dispatch(uint3(size, 1), uint3(k_tileSizeX * 4, k_tileSizeY * 4, 1));
			}/*, PassFlags::Compute*/);
	}


	{
		struct ShadowDenoiser_TileClassificationData
		{
			Handlers::StructuredBuffer<uint32_t> H(ShadowDenoiser_TileBuffer);
			Handlers::Texture H(GBuffer_DepthPrev);
			Handlers::Texture H(GBuffer_Depth);
			Handlers::Texture H(GBuffer_Normals);
			Handlers::Texture H(GBuffer_Speed);


			Handlers::StructuredBuffer<uint32_t> H(ShadowDenoiser_TileMetaBuffer);
			Handlers::Texture H(ShadowDenoiser_Moments);
			Handlers::Texture H(ShadowDenoiser_MomentsPrev);
			Handlers::Texture H(ShadowDenoiser_Scratch);
			Handlers::Texture H(ShadowDenoiser_Scratch2);
		};

		graph.pass<ShadowDenoiser_TileClassificationData>("ShadowDenoiser_TileClassification", [this, &graph, tileCount, size](ShadowDenoiser_TileClassificationData& data, TaskBuilder& builder) {
			builder.need(data.ShadowDenoiser_TileBuffer, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_DepthPrev, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Depth, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Normals, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Speed, ResourceFlags::ComputeRead);

			builder.create(data.ShadowDenoiser_TileMetaBuffer, { tileCount }, ResourceFlags::UnorderedAccess);
			builder.create(data.ShadowDenoiser_Moments, { ivec3(size, 0), HAL::Format::R11G11B10_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ShadowDenoiser_MomentsPrev, { ivec3(size, 0), HAL::Format::R11G11B10_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ShadowDenoiser_Scratch, { ivec3(size, 0), HAL::Format::R16G16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ShadowDenoiser_Scratch2, { ivec3(size, 0), HAL::Format::R16G16_FLOAT, 1,1 }, ResourceFlags::UnorderedAccess );

			return true;
			}, [this, &graph, size](ShadowDenoiser_TileClassificationData& data, FrameContext& _context) {
				auto& list = *_context.get_list();

				auto& compute = list.get_compute();
				auto& cam = graph.get_context<CameraInfo>();

				bool first = false;
				if (data.ShadowDenoiser_MomentsPrev.is_new())
				{
					first = true;
					list.clear_uav(data.ShadowDenoiser_MomentsPrev->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ShadowDenoiser_Scratch->rwTexture2D, vec4(0, 0, 0, 0));
					list.clear_uav(data.ShadowDenoiser_Scratch2->rwTexture2D, vec4(0, 0, 0, 0));

				}
				//list.clear_uav(data.ShadowDenoiser_TileMetaBuffer->rwRAW, vec4(0, 0, 0, 0));


				compute.set_pipeline<PSOS::DenoiserShadow_TileClassification>();

				Slots::DenoiserShadow_TileClassification classification_data;
				classification_data.Eye = cam.cam->camera_cb.current.GetPosition().xyz;//dc.camPos.getX(), dc.camPos.getY(), dc.camPos.getZ(),
				classification_data.FirstFrame = first;
				classification_data.BufferDimensions = size;
				classification_data.InvBufferDimensions = float2(1.0f, 1.0f) / size;
				classification_data.ProjectionInverse = cam.cam->camera_cb.current.GetInvProj();
				classification_data.ReprojectionMatrix = cam.cam->camera_cb.current.GetReprojectionProj();
				classification_data.ViewProjectionInverse = cam.cam->camera_cb.current.GetInvViewProj();

				//classification_data.ProjectionInverse.transpose();
				//classification_data.ReprojectionMatrix.transpose();
				//classification_data.ViewProjectionInverse.transpose();

				classification_data.t2d_depth = data.GBuffer_Depth->texture2D;
				classification_data.t2d_velocity = data.GBuffer_Speed->texture2D;
				classification_data.t2d_normal = data.GBuffer_Normals->texture2D;
				classification_data.t2d_history = data.ShadowDenoiser_Scratch->texture2D;
				classification_data.t2d_previousDepth = data.GBuffer_DepthPrev->texture2D;
				classification_data.sb_raytracerResult = data.ShadowDenoiser_TileBuffer->structuredBuffer;


				classification_data.rwsb_tileMetaData = data.ShadowDenoiser_TileMetaBuffer->rwStructuredBuffer;
				classification_data.rwt2d_reprojectionResults = data.ShadowDenoiser_Scratch2->rwTexture2D;

				classification_data.t2d_previousMoments = data.ShadowDenoiser_MomentsPrev->texture2D;
				classification_data.rwt2d_momentsBuffer = data.ShadowDenoiser_Moments->rwTexture2D;


				compute.set(classification_data);


	//			if(!GetAsyncKeyState('6'))
				compute.dispatch(uint3(size, 1));


				list.get_copy().copy_texture(data.ShadowDenoiser_MomentsPrev->resource->get_ptr(), 0, data.ShadowDenoiser_Moments->resource->get_ptr(), 0);

			}/*, PassFlags::Compute*/);
	}
	{
		struct ShadowDenoiser_FilterData
		{
			Handlers::StructuredBuffer<uint32_t> H(ShadowDenoiser_TileMetaBuffer);
			Handlers::Texture H(RTXDebug);
			Handlers::Texture H(GBuffer_Depth);
			Handlers::Texture H(GBuffer_Normals);
			Handlers::Texture H(ShadowDenoiser_Scratch);
			Handlers::Texture H(ShadowDenoiser_Scratch2);
		};

		graph.pass<ShadowDenoiser_FilterData>("ShadowDenoiser_Filter", [this, &graph, tileCount, size](ShadowDenoiser_FilterData& data, TaskBuilder& builder) {
		//	builder.need(data.ShadowDenoiser_TileBuffer, ResourceFlags::ComputeRead);
			builder.need(data.RTXDebug, ResourceFlags::UnorderedAccess);
			builder.need(data.GBuffer_Depth, ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Normals, ResourceFlags::ComputeRead);

				builder.need(data.ShadowDenoiser_TileMetaBuffer, ResourceFlags::ComputeRead);
				builder.need(data.ShadowDenoiser_Scratch, ResourceFlags::UnorderedAccess);
			builder.need(data.ShadowDenoiser_Scratch2,ResourceFlags::UnorderedAccess);

			return true;
			}, [this, &graph,size](ShadowDenoiser_FilterData& data, FrameContext& _context) {
				auto& list = *_context.get_list();

				auto& compute = list.get_compute();
					auto& cam = graph.get_context<CameraInfo>();

				{

					Slots::DenoiserShadow_Filter filter_data;

					filter_data.BufferDimensions = size;
					filter_data.InvBufferDimensions = float2(1.0f, 1.0f) / size;
					filter_data.ProjectionInverse = cam.cam->camera_cb.current.GetInvProj();
					filter_data.DepthSimilaritySigma = 0.01f;
				//	filter_data.ProjectionInverse.transpose();
			
					filter_data.t2d_DepthBuffer = data.GBuffer_Depth->texture2D;
					filter_data.t2d_NormalBuffer = data.GBuffer_Normals->texture2D;

					filter_data.sb_tileMetaData = data.ShadowDenoiser_TileMetaBuffer->structuredBuffer;


					compute.set(filter_data);
				}

				{
					compute.set_pipeline<PSOS::DenoiserShadow_Filter>(PSOS::DenoiserShadow_Filter::Pass(0));

					Slots::DenoiserShadow_FilterLocal filter_data;
					filter_data.GetRqt2d_input() = data.ShadowDenoiser_Scratch2->texture2D;
					filter_data.GetRwt2d_output() = data.ShadowDenoiser_Scratch->rwTexture2D;
					compute.set(filter_data);

					compute.dispatch(uint3(size, 1));
				}


				{
					compute.set_pipeline<PSOS::DenoiserShadow_Filter>(PSOS::DenoiserShadow_Filter::Pass(1));

					Slots::DenoiserShadow_FilterLocal filter_data;
					filter_data.GetRqt2d_input() = data.ShadowDenoiser_Scratch->texture2D;
					filter_data.GetRwt2d_output() = data.ShadowDenoiser_Scratch2->rwTexture2D;
					compute.set(filter_data);

					compute.dispatch(uint3(size, 1));
				}

				{
					compute.set_pipeline<PSOS::DenoiserShadow_Filter>(PSOS::DenoiserShadow_Filter::Pass(2));

					Slots::DenoiserShadow_FilterLast filter_data;
					filter_data.GetRqt2d_input() = data.ShadowDenoiser_Scratch2->texture2D;
					filter_data.GetRwt2d_output() = data.RTXDebug->rwTexture2D;
					compute.set(filter_data);

					compute.dispatch(uint3(size, 1));

				}


					list.get_copy().copy_texture(data.ShadowDenoiser_Scratch->resource->get_ptr(), 0, data.ShadowDenoiser_Scratch2->resource->get_ptr(), 0);



			}/*, PassFlags::Compute*/);
	 }
}

