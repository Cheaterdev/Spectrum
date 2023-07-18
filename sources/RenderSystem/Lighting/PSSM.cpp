module Graphics:PSSM;
import <RenderSystem.h>;

import :PSSM;
import :BRDF;
import :EngineAssets;

import HAL;


import Graphics;
using namespace FrameGraph;
float jit = 0;
import HAL;
using namespace HAL;
float3 PSSM::get_position()
{
	pos_mutex.lock();
	auto p = position;
	pos_mutex.unlock();
	return p;
}

void PSSM::set_position(float3 p)
{
	pos_mutex.lock();
	position = p;
	pos_mutex.unlock();
}

PSSM::PSSM()
{
	position = float3(200, 400, 200);
}



void PSSM::generate(Graph& graph)
{

	auto& sceneinfo = graph.get_context<SceneInfo>();
	auto& caminfo = graph.get_context<CameraInfo>();


	auto scene = sceneinfo.scene;
	auto min = scene->get_min();
	auto max = scene->get_max();
	auto cam = caminfo.cam;


	auto points_all = cam->get_points(min, max);
	float one_pixel_size = (max - min).max_element() / size.x;
	float3 center = (max + min) / 2;
	const float clearColor[] = { 0, 0, 0, 0 };

	scaler = cam->z_far / (exp((float)renders_size));

	static float t = 0;


	float znear = cam->z_near;
	float zfar = 10;




	struct PSSMDataGlobal
	{
		Handlers::Texture H(global_depth);
		Handlers::StructuredBuffer<Table::Camera> H(global_camera);
	};

	graph.add_pass<PSSMDataGlobal>("PSSM_Global", [this, &graph](PSSMDataGlobal& data, TaskBuilder& builder) {
		builder.create(data.global_depth, { ivec3(1024, 1024,0), HAL::Format::R32_TYPELESS, 1 ,1 }, ResourceFlags::DepthStencil);
		builder.create(data.global_camera, { 1 }, ResourceFlags::GenCPU);
		}, [this, &graph, cam, points_all](PSSMDataGlobal& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto& graphics = command_list->get_graphics();
			auto& compute = command_list->get_compute();

			auto& sceneinfo = graph.get_context<SceneInfo>();



			graphics.set_signature(Layouts::DefaultLayout);
			compute.set_signature(Layouts::DefaultLayout);

			camera light_cam;

			light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
			light_cam.position = get_position();
			light_cam.up = (float3(0.01, 1, 0.023)).normalize();
			light_cam.update();


			auto bounds_all = points_all.get_bounds_in(light_cam.get_view());



			MeshRenderContext::ptr context(new MeshRenderContext());

			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			context->cam = &light_cam;
			context->render_type = RENDER_TYPE::DEPTH;

			auto scene = sceneinfo.scene;
			auto renderer = sceneinfo.renderer;


			box bounds = points_all.get_bounds_in(light_cam.get_view());
			light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);

			light_cam.update();

			//	auto ptr = reinterpret_cast<camera::shader_params*>(shadow_cameras->get_data() )+ i;// *sizeof(camera::shader_params), (i + 1) * sizeof(camera::shader_params));
			//	*ptr = light_cam.get_const_buffer().data().current;

			data.global_camera->write(0, &light_cam.camera_cb.current, 1);

			//context->overrided_pipeline = mat->get_pipeline();
			//context->use_materials = false;
			//context->pipeline.blend.render_target[0].enabled = false;
			//context->pipeline.rasterizer.cull_mode = CullMode::Front;

			{
				RT::DepthOnly rt;
				rt.GetDepth() = data.global_depth->depthStencil;
				auto compiled = rt.set(command_list->get_graphics(), RTOptions::Default |  RTOptions::ClearDepth);
			}


			{
				Slots::FrameInfo frameInfo;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
				auto& camera = frameInfo.GetCamera();
				//	memcpy(&camera, &graph.cam->camera_cb.current, sizeof(camera));
				camera = light_cam.camera_cb.current;
				graphics.set(frameInfo);
				compute.set(frameInfo);
			}

			renderer->render(context, scene->get_ptr<Scene>());


		});



	struct PSSMData
	{
		GBufferViewDesc gbuffer;

		Handlers::Texture H(LightMask);
		Handlers::Texture H(PSSM_Depths);
		Handlers::Texture H(ResultTexture);
		Handlers::StructuredBuffer<Table::Camera> H(PSSM_Cameras);

		Handlers::Texture H(RTXDebug);
	};

	graph.add_pass<PSSMData>("PSSM_TexGenerator", [this, &graph](PSSMData& data, TaskBuilder& builder) {
		builder.create(data.PSSM_Depths, { ivec3(size,0), HAL::Format::R32_TYPELESS,renders_size ,1 }, ResourceFlags::DepthStencil);
		builder.create(data.PSSM_Cameras, { renders_size }, ResourceFlags::GenCPU);
		}, [](PSSMData& data, FrameContext& _context) {});



	auto position = get_position();
	for (int i = 0; i < renders_size; i++)
	{
		zfar = cam->z_near + (exp(float(i + 1))) * scaler;



		graph.add_pass<PSSMData>(std::string("PSSM_Cascade_") + std::to_string(i), [this, &graph](PSSMData& data, TaskBuilder& builder) {
			builder.need(data.PSSM_Cameras, ResourceFlags::None);
			builder.need(data.PSSM_Depths, ResourceFlags::DepthStencil);
			}, [this, &graph, i, znear, zfar, cam, points_all, position](PSSMData& data, FrameContext& _context) {

				auto& command_list = _context.get_list();
				auto& graphics = command_list->get_graphics();
				auto& compute = command_list->get_compute();

				auto& sceneinfo = graph.get_context<SceneInfo>();

				graphics.set_signature(Layouts::DefaultLayout);
				compute.set_signature(Layouts::DefaultLayout);

				camera light_cam;

				light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
				light_cam.position = get_position();
				light_cam.up = (float3(0.01, 1, 0.023)).normalize();
				light_cam.update();


				auto bounds_all = points_all.get_bounds_in(light_cam.get_view());



				MeshRenderContext::ptr context(new MeshRenderContext());

				context->priority = TaskPriority::HIGH;
				context->list = command_list;
				context->cam = &light_cam;

				auto depth_tex = data.PSSM_Depths->create_2d_slice(i, *command_list);
				//	auto shadow_cameras = _context.get_buffer(data.shadow_cameras);

				auto scene = sceneinfo.scene;
				auto renderer = sceneinfo.renderer;





				auto points = cam->get_points(znear, zfar);


				box bounds = points.get_bounds_in(light_cam.get_view());
				light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);

				light_cam.update();

				//	auto ptr = reinterpret_cast<camera::shader_params*>(shadow_cameras->get_data() )+ i;// *sizeof(camera::shader_params), (i + 1) * sizeof(camera::shader_params));
				//	*ptr = light_cam.get_const_buffer().data().current;

				data.PSSM_Cameras->write(i * sizeof(camera::shader_params), reinterpret_cast<Table::Camera*>(&light_cam.camera_cb.current), 1);

				{
					RT::DepthOnly rt;
					rt.GetDepth() = depth_tex.depthStencil;
					rt.set(command_list->get_graphics(), RTOptions::Default | RTOptions::ClearDepth);
				}

				{
					Slots::FrameInfo frameInfo;

					frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
					auto& camera = frameInfo.GetCamera();
					//	memcpy(&camera, &graph.cam->camera_cb.current, sizeof(camera));
					camera = light_cam.camera_cb.current;
					graphics.set(frameInfo);
					compute.set(frameInfo);
				}
				context->render_type = RENDER_TYPE::DEPTH;

				renderer->render(context, scene->get_ptr<Scene>());
			}
			);
		znear = zfar;
	}


	// relight pass
	graph.add_pass<PSSMData>("PSSM_GenerateMask", [this, &graph](PSSMData& data, TaskBuilder& builder) {
		auto& frame = graph.get_context<ViewportInfo>();

		builder.create(data.LightMask, { ivec3(frame.frame_size,0), HAL::Format::R8_UNORM,1,1 }, ResourceFlags::RenderTarget);
		data.gbuffer.need(builder);
		builder.need(data.PSSM_Depths, ResourceFlags::PixelRead);
		builder.need(data.PSSM_Cameras, ResourceFlags::None);

		}, [this, &graph](PSSMData& data, FrameContext& _context) {

			GBuffer gbuffer = data.gbuffer.actualize(_context);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();
			auto& compute = list.get_compute();

			//list.set_my_heap();// set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_csu());
			graphics.set_signature(Layouts::DefaultLayout);
			compute.set_signature(Layouts::DefaultLayout);

			graph.set_slot(SlotID::FrameInfo, graphics);
			graph.set_slot(SlotID::FrameInfo, compute);

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.GetGbuffer());

				//lighting.GetLight_mask() = screen_light_mask.get_srv();

				graphics.set(lighting);
			}

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);


			graphics.set_viewport(data.LightMask->get_viewport());
			graphics.set_scissor(data.LightMask->get_scissor());
			{
				RT::SingleColor rt;
				rt.GetColor() = data.LightMask->renderTarget;
				rt.set(graphics);
			}
			graphics.set_pipeline<PSOS::PSSMMask>();


			{
				Slots::PSSMData pssmdata;
				pssmdata.GetLight_buffer() = data.PSSM_Depths->texture2DArray;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				graphics.set(pssmdata);
			}

			for (int i = renders_size - 1; i >= 0; i--) //rangeees
			{
				//		PROFILE_GPU((std::wstring(L"renders") + std::to_wstring(i)).c_str());
				{
					Slots::PSSMConstants constants;
					constants.GetLevel() = i;
					constants.GetTime() = 0;
					graphics.set(constants);
				}
				graphics.draw(4);
			}

		});


	// relight pass
	graph.add_pass<PSSMData>("PSSM_Combine", [this, &graph](PSSMData& data, TaskBuilder& builder) {

		data.gbuffer.need(builder);

		//  builder.need(data.PSSM_Depths, ResourceFlags::PixelRead);
		builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
		builder.need(data.PSSM_Cameras, ResourceFlags::None);

		if (builder.exists(data.RTXDebug))
		{

			builder.need(data.RTXDebug, ResourceFlags::PixelRead);
		}
		else
		{
			builder.need(data.LightMask, ResourceFlags::PixelRead);

		}
		}, [this, &graph](PSSMData& data, FrameContext& _context) {

			GBuffer gbuffer = data.gbuffer.actualize(_context);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();
			auto& compute = list.get_compute();

			//list.set_my_heap();// set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_csu());
			graphics.set_signature(Layouts::DefaultLayout);
			compute.set_signature(Layouts::DefaultLayout);

			graph.set_slot(SlotID::FrameInfo, graphics);
			graph.set_slot(SlotID::FrameInfo, compute);


			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());

			{
				Slots::PSSMData pssmdata;
				//	pssmdata.GetLight_buffer() = data.PSSM_Depths->texture2DArray;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				graphics.set(pssmdata);
			}

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.GetGbuffer());

				if (data.RTXDebug)
					lighting.GetLight_mask() = data.RTXDebug->texture2D;//data.LightMask->texture2D;
				else
					lighting.GetLight_mask() = data.LightMask->texture2D;

				graphics.set(lighting);
			}

			{
				RT::SingleColor rt;
				rt.GetColor() = data.ResultTexture->renderTarget;
				rt.set(graphics);
			}

			graphics.set_pipeline<PSOS::PSSMApply>();

			graphics.draw(4);

		});

}