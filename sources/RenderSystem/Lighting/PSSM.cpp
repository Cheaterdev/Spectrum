#include "pch.h"

float jit = 0;

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
	mat.reset(new ShaderMaterial("",""));
	position = float3(200, 400, 200);
}



void PSSM::generate(FrameGraph& graph)
{



	auto scene = graph.scene;
	auto min = scene->get_min();
	auto max = scene->get_max();
	auto cam = graph.cam;


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
		ResourceHandler* global_depth;
		ResourceHandler* global_camera;

	};

	graph.add_pass<PSSMDataGlobal>("PSSM_Global", [this, &graph](PSSMDataGlobal& data, TaskBuilder& builder) {
		data.global_depth = builder.create_texture("global_depth", ivec2(1024, 1024), 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil);
		data.global_camera = builder.create_buffer("global_camera", sizeof(camera::shader_params) * 1, ResourceFlags::GenCPU);


		}, [this, &graph, cam, points_all](PSSMDataGlobal& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			std::vector<sizer_long> scissor;
			std::vector<Render::Viewport> viewport;
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

			auto global_depth = _context.get_texture(data.global_depth);
			auto global_camera = _context.get_buffer(data.global_camera);

			auto scene = graph.scene;
			auto renderer = graph.renderer;


			//_context.register_subview(depth_tex);

			auto table = RenderTargetTable(command_list->get_graphics(), {}, global_depth);


			box bounds = points_all.get_bounds_in(light_cam.get_view());
			light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);

			light_cam.update();

			//	auto ptr = reinterpret_cast<camera::shader_params*>(shadow_cameras->get_data() )+ i;// *sizeof(camera::shader_params), (i + 1) * sizeof(camera::shader_params));
			//	*ptr = light_cam.get_const_buffer().data().current;

			global_camera.write(0, &light_cam.camera_cb.current, 1);


			viewport.resize(1);
			viewport[0].MinDepth = 0.0f;
			viewport[0].MaxDepth = 1.0f;
			viewport[0].Width = static_cast<float>(size.x);
			viewport[0].Height = static_cast<float>(size.y);
			viewport[0].TopLeftX = 0;
			viewport[0].TopLeftY = 0;

			scissor.resize(1);
			scissor[0] = { 0, 0, size.x, size.y };

			context->overrided_pipeline = mat->get_pipeline();
			//context->use_materials = false;
			command_list->get_graphics().set_viewports(viewport);
			command_list->get_graphics().set_scissors(scissor[0]);
			context->pipeline.blend.render_target[0].enabled = false;
			context->pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;

			

			table.set(context, false, true);

			renderer->render(context, scene->get_ptr<Scene>());


});



	struct PSSMData
	{
		GBufferViewDesc gbuffer;

		ResourceHandler* screen_light_mask;
		ResourceHandler* full_scene_depth;
		ResourceHandler* shadow_depths;
		ResourceHandler* shadow_cameras;

		ResourceHandler* target_tex;



	};

	graph.add_pass<PSSMData>("PSSM_TexGenerator", [this, &graph](PSSMData& data, TaskBuilder& builder) {
		data.shadow_depths = builder.create_texture("PSSM_Depths", size, renders_size, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::None);
		data.shadow_cameras = builder.create_buffer("PSSM_Cameras", sizeof(camera::shader_params) * renders_size, ResourceFlags::GenCPU);
		}, [](PSSMData& data, FrameContext& _context) {});



	auto position = get_position();
	for (int i = 0; i < renders_size; i++)
	{
		zfar = cam->z_near + (exp(float(i + 1))) * scaler;



		graph.add_pass<PSSMData>(std::string("PSSM_Cascade_") + std::to_string(i), [this, &graph](PSSMData& data, TaskBuilder& builder) {
			data.shadow_cameras = builder.need_buffer("PSSM_Cameras", ResourceFlags::None);
			data.shadow_depths = builder.need_texture("PSSM_Depths", ResourceFlags::DepthStencil);
			}, [this, &graph, i, znear, zfar, cam, points_all, position](PSSMData& data, FrameContext& _context) {

				auto& command_list = _context.get_list();


				std::vector<sizer_long> scissor;
				std::vector<Render::Viewport> viewport;
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

				auto depth_tex = _context.get_texture(data.shadow_depths).create_2d_slice(i, *graph.builder.current_frame);
				auto shadow_cameras = _context.get_buffer(data.shadow_cameras);

				auto scene = graph.scene;
				auto renderer = graph.renderer;


				//_context.register_subview(depth_tex);

				auto table = RenderTargetTable(command_list->get_graphics(), {}, depth_tex);



				auto points = cam->get_points(znear, zfar);


				box bounds = points.get_bounds_in(light_cam.get_view());
				light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);

				light_cam.update();

				//	auto ptr = reinterpret_cast<camera::shader_params*>(shadow_cameras->get_data() )+ i;// *sizeof(camera::shader_params), (i + 1) * sizeof(camera::shader_params));
				//	*ptr = light_cam.get_const_buffer().data().current;

				shadow_cameras.write(i * sizeof(camera::shader_params), &light_cam.camera_cb.current, 1);
				viewport.resize(1);
				viewport[0].MinDepth = 0.0f;
				viewport[0].MaxDepth = 1.0f;
				viewport[0].Width = static_cast<float>(size.x);
				viewport[0].Height = static_cast<float>(size.y);
				viewport[0].TopLeftX = 0;
				viewport[0].TopLeftY = 0;

				scissor.resize(1);
				scissor[0] = { 0, 0, size.x, size.y };

				context->overrided_pipeline = mat->get_pipeline();
				//context->use_materials = false;
				command_list->get_graphics().set_viewports(viewport);
				command_list->get_graphics().set_scissors(scissor[0]);
				context->pipeline.blend.render_target[0].enabled = false;
				context->pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;


				table.set(context, false, true);

				renderer->render(context, scene->get_ptr<Scene>());
			}
			);
		znear = zfar;
	}




	// relight pass
	graph.add_pass<PSSMData>("PSSM_Process", [this, &graph](PSSMData& data, TaskBuilder& builder) {
		data.shadow_depths = builder.need_texture("PSSM_Depths", ResourceFlags::PixelRead);

		data.gbuffer.need(builder);
		
		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
		data.screen_light_mask = builder.create_texture("LightMask", graph.frame_size, 1, DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, ResourceFlags::RenderTarget);
		data.shadow_cameras = builder.need_buffer("PSSM_Cameras", ResourceFlags::PixelRead);


		}, [this, &graph](PSSMData& data, FrameContext& _context) {

			GBuffer gbuffer = data.gbuffer.actualize(_context);

			auto screen_light_mask = _context.get_texture(data.screen_light_mask);
			auto depth_tex = _context.get_texture(data.shadow_depths);

			auto target_tex = _context.get_texture(data.target_tex);
			auto shadow_cameras = _context.get_buffer(data.shadow_cameras);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			//list.set_my_heap();// set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_csu());
			graphics.set_layout(Layouts::DefaultLayout);


			{
				Slots::FrameInfo frameInfo;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();
				auto camera = frameInfo.MapCamera();
			//	memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
				camera.cb = graph.cam->camera_cb.current;
				frameInfo.set(graphics);
			}

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.MapGbuffer());
			
				//lighting.GetLight_mask() = screen_light_mask.get_srv();

				lighting.set(graphics);
			}

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


			graphics.set_viewport(screen_light_mask.get_viewport());
			graphics.set_scissor(screen_light_mask.get_scissor());

			graphics.set_rtv(1, screen_light_mask.get_rtv(), Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::PSSMMask>());


			auto buffer_view = shadow_cameras.resource->create_view<StructuredBufferView<camera::shader_params>>(*graph.builder.current_frame);
			{
				Slots::PSSMData pssmdata;
				pssmdata.GetLight_buffer() = depth_tex.get_srv();
				pssmdata.GetLight_cameras() = buffer_view.get_srv();
				pssmdata.set(graphics);
			}

			for (int i = renders_size - 1; i >= 0; i--) //rangeees
			{
				auto timer = list.start((std::wstring(L"renders") + std::to_wstring(i)).c_str());
				{
					Slots::PSSMConstants constants;
					constants.GetLevel() = i;
					constants.GetTime() = 0;
					constants.set(graphics);
				}
				graphics.draw(4);
			}
			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.MapGbuffer());

				lighting.GetLight_mask() = screen_light_mask.get_srv();

				lighting.set(graphics);
			}

			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::PSSMApply>());

			graphics.draw(4);

		});

}