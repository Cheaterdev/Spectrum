export module Graphics:PSSM;
import :Materials.ShaderMaterial;
import :Camera;
import :Scene;
import :EngineAssets;
import :FrameGraphContext;
  import :BRDF;

import FrameGraph;
import HAL;

export class PSSM
{
	ivec2 size = { 1024, 1024 };
	float2 pixel_size = float2(1, 1) / float2(1024, 1024);

	float scaler = 1;

	static const int renders_size = 5;

	std::mutex pos_mutex;
	float3 position;
	size_t counter = 0;
public:

	float3 get_position();
	void set_position(float3 p);

	PSSM();

	template<typename TPipeline>
	explicit PSSM(TPipeline& pipeline) : PSSM()
	{
		pipeline.pSSM_Global.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& sceneinfo = builder.graph->get_context<SceneInfo>();
			auto& caminfo   = builder.graph->get_context<CameraInfo>();

			auto scene = sceneinfo.scene;
			auto cam   = caminfo.cam;

			scaler = cam->z_far / (exp((float)renders_size));

			builder.create(data.global_depth,  { ivec3(1024, 1024, 0), HAL::Format::R32_TYPELESS, 1, 1 }, FrameGraph::ResourceFlags::DepthStencil);
			builder.create(data.global_camera, { 1 },                                                      FrameGraph::ResourceFlags::GenCPU);
			return true;
		};

		pipeline.pSSM_Global.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto& graphics     = command_list->get_graphics();
			auto& compute      = command_list->get_compute();

			auto& sceneinfo = context.graph->get_context<SceneInfo>();
			auto& caminfo   = context.graph->get_context<CameraInfo>();

			auto scene      = sceneinfo.scene;
			auto cam        = caminfo.cam;
			auto min        = scene->get_min();
			auto max        = scene->get_max();
			auto points_all = cam->get_points(min, max);

			camera light_cam;
			light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
			light_cam.position = get_position();
			light_cam.up = (float3(0.01, 1, 0.023)).normalize();
			light_cam.update();

			auto bounds_all = points_all.get_bounds_in(light_cam.get_view());

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			mesh_ctx->priority    = TaskPriority::HIGH;
			mesh_ctx->list        = command_list;
			mesh_ctx->cam         = &light_cam;
			mesh_ctx->render_type = RENDER_TYPE::DEPTH;

			auto renderer = sceneinfo.renderer;

			box bounds = points_all.get_bounds_in(light_cam.get_view());
			light_cam.set_projection_params(
				bounds.left - 1, bounds.right + 1,
				bounds.top  - 1, bounds.bottom + 1,
				std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);
			light_cam.update();

			data.global_camera->write(0, &light_cam.camera_cb.current, 1);

			{
				RT::DepthOnly rt;
				rt.GetDepth() = data.global_depth->depthStencil;
				command_list->get_graphics().set_rtv(rt, RTOptions::Default | RTOptions::ClearDepth);
			}

			{
				Slots::FrameInfo frameInfo;
				frameInfo.GetBrdf()   = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
				frameInfo.GetCamera() = light_cam.camera_cb.current;
				graphics.set(frameInfo);
				compute.set(frameInfo);
			}

			renderer->render(mesh_ctx, scene->get_ptr<Scene>());
		};

		for (int i = 0; i < renders_size; i++)
		{
			pipeline.pSSM_Cascade.setup_funcs[i] = [this, i](auto& data, FrameGraph::TaskBuilder& builder) -> bool
			{
				if (i == 0)
				{
					builder.create(data.PSSM_Depths,  { ivec3(size, 0), HAL::Format::R32_TYPELESS, renders_size, 1 }, FrameGraph::ResourceFlags::DepthStencil);
					builder.create(data.PSSM_Cameras, { renders_size },                                               FrameGraph::ResourceFlags::GenCPU);
				}
				else
				{
					builder.need(data.PSSM_Cameras, FrameGraph::ResourceFlags::GenCPU);
					builder.need(data.PSSM_Depths,  FrameGraph::ResourceFlags::DepthStencil);
				}
				return true;
			};

			pipeline.pSSM_Cascade.render_funcs[i] = [this, i](auto& data, FrameGraph::FrameContext& context)
			{
				auto& command_list = context.get_list();
				auto& graphics     = command_list->get_graphics();
				auto& compute      = command_list->get_compute();

				auto& sceneinfo = context.graph->get_context<SceneInfo>();
				auto& caminfo   = context.graph->get_context<CameraInfo>();
				auto  cam       = caminfo.cam;

				graphics.set_signature(Layouts::DefaultLayout);
				compute.set_signature(Layouts::DefaultLayout);

				camera light_cam;
				light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
				light_cam.position = get_position();
				light_cam.up = (float3(0.01, 1, 0.023)).normalize();
				light_cam.update();

				auto scene      = sceneinfo.scene;
				auto min        = scene->get_min();
				auto max        = scene->get_max();
				auto points_all = cam->get_points(min, max);
				auto bounds_all = points_all.get_bounds_in(light_cam.get_view());

				float znear = (i == 0) ? cam->z_near : cam->z_near + exp((float)i)       * scaler;
				float zfar  =            cam->z_near + exp((float)(i + 1)) * scaler;

				MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
				mesh_ctx->priority = TaskPriority::HIGH;
				mesh_ctx->list     = command_list;
				mesh_ctx->cam      = &light_cam;

				auto depth_tex = data.PSSM_Depths->create_2d_slice(i, *command_list);
				auto renderer  = sceneinfo.renderer;

				auto points = cam->get_points(znear, zfar);
				box  bounds = points.get_bounds_in(light_cam.get_view());
				light_cam.set_projection_params(
					bounds.left - 1, bounds.right + 1,
					bounds.top  - 1, bounds.bottom + 1,
					std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);
				light_cam.update();

				data.PSSM_Cameras->write(i * sizeof(camera::shader_params),
					reinterpret_cast<Table::Camera*>(&light_cam.camera_cb.current), 1);

				{
					RT::DepthOnly rt;
					rt.GetDepth() = depth_tex.depthStencil;
					command_list->get_graphics().set_rtv(rt, RTOptions::Default | RTOptions::ClearDepth);
				}

				{
					Slots::FrameInfo frameInfo;
					frameInfo.GetBrdf()   = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
					frameInfo.GetCamera() = light_cam.camera_cb.current;
					graphics.set(frameInfo);
					compute.set(frameInfo);
				}

				mesh_ctx->render_type = RENDER_TYPE::DEPTH;
				renderer->render(mesh_ctx, scene->get_ptr<Scene>());
			};
		}

		pipeline.pSSM_GenerateMask.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& frame = builder.graph->get_context<ViewportInfo>();
			builder.create(data.LightMask, { ivec3(frame.frame_size, 0), HAL::Format::R8_UNORM, 1, 1 }, FrameGraph::ResourceFlags::RenderTarget);
			GBufferViewDesc::need(builder, data.gbuffer);
			builder.need(data.PSSM_Depths,  FrameGraph::ResourceFlags::PixelRead);
			builder.need(data.PSSM_Cameras, FrameGraph::ResourceFlags::None);
			return true;
		};

		pipeline.pSSM_GenerateMask.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

			auto& list     = *context.get_list();
			auto& graphics = list.get_graphics();
			auto& compute  = list.get_compute();

			context.graph->set_slot(SlotID::FrameInfo, graphics);
			context.graph->set_slot(SlotID::FrameInfo, compute);

			{
				Slots::PSSMLighting lighting;
				gbuffer.SetTable(lighting.GetGbuffer());
				graphics.set(lighting);
			}

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			graphics.set_viewport(data.LightMask->get_viewport());
			graphics.set_scissor(data.LightMask->get_scissor());

			{
				RT::SingleColor rt;
				rt.GetColor() = data.LightMask->renderTarget;
				graphics.set_rtv(rt);
			}

			graphics.set_pipeline<PSOS::PSSMMask>();

			{
				Slots::PSSMData pssmdata;
				pssmdata.GetLight_buffer()  = data.PSSM_Depths->texture2DArray;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				graphics.set(pssmdata);
			}

			for (int i = renders_size - 1; i >= 0; i--)
			{
				{
					Slots::PSSMConstants constants;
					constants.GetLevel() = i;
					constants.GetTime()  = 0;
					graphics.set(constants);
				}
				graphics.draw(4);
			}
		};

		pipeline.pSSM_Combine.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			GBufferViewDesc::need(builder, data.gbuffer);
			builder.need(data.ResultTexture, FrameGraph::ResourceFlags::RenderTarget);
			builder.need(data.PSSM_Cameras,  FrameGraph::ResourceFlags::None);

			if (builder.exists(data.RTXDebug))
				builder.need(data.RTXDebug,  FrameGraph::ResourceFlags::PixelRead);
			else
				builder.need(data.LightMask, FrameGraph::ResourceFlags::PixelRead);

			return true;
		};

		pipeline.pSSM_Combine.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

			auto& list     = *context.get_list();
			auto& graphics = list.get_graphics();
			auto& compute  = list.get_compute();

			context.graph->set_slot(SlotID::FrameInfo, graphics);
			context.graph->set_slot(SlotID::FrameInfo, compute);

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());

			{
				Slots::PSSMData pssmdata;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				graphics.set(pssmdata);
			}

			{
				Slots::PSSMLighting lighting;
				gbuffer.SetTable(lighting.GetGbuffer());
				if (data.RTXDebug)
					lighting.GetLight_mask() = data.RTXDebug->texture2D;
				else
					lighting.GetLight_mask() = data.LightMask->texture2D;
				graphics.set(lighting);
			}

			{
				RT::SingleColor rt;
				rt.GetColor() = data.ResultTexture->renderTarget;
				graphics.set_rtv(rt);
			}

			graphics.set_pipeline<PSOS::PSSMApply>();
			graphics.draw(4);
		};
	}
};
