#include "pch_render.h"
#include "Assets/EngineAssets.h"
#include "PSSM.h"
#include "Renderer/Renderer.h"

#include "Camera/Camera.h"
using namespace FrameGraph;
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



void PSSM::generate(Graph& graph)
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
		Handlers::Texture H(global_depth);
		Handlers::StructuredBuffer<Table::Camera::CB> H(global_camera);
	};

	graph.add_pass<PSSMDataGlobal>("PSSM_Global", [this, &graph](PSSMDataGlobal& data, TaskBuilder& builder) {
		builder.create(data.global_depth, { ivec3(1024, 1024,1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, 1 ,1}, ResourceFlags::DepthStencil);
		builder.create(data.global_camera, { 1 }, ResourceFlags::GenCPU);
		}, [this, &graph, cam, points_all](PSSMDataGlobal& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto& graphics = command_list->get_graphics();
			auto& compute = command_list->get_compute();

			graphics.set_layout(Layouts::DefaultLayout);
			compute.set_layout(Layouts::DefaultLayout);


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

			auto scene = graph.scene;
			auto renderer = graph.renderer;

			auto table = RenderTargetTable(command_list->get_graphics(), {}, *data.global_depth);


			box bounds = points_all.get_bounds_in(light_cam.get_view());
			light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar);

			light_cam.update();

			//	auto ptr = reinterpret_cast<camera::shader_params*>(shadow_cameras->get_data() )+ i;// *sizeof(camera::shader_params), (i + 1) * sizeof(camera::shader_params));
			//	*ptr = light_cam.get_const_buffer().data().current;

			data.global_camera->write(0, &light_cam.camera_cb.current, 1);


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



			{
				Slots::FrameInfo frameInfo;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
				auto camera = frameInfo.MapCamera();
				//	memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
				camera.cb = light_cam.camera_cb.current;
				frameInfo.set(graphics);
				frameInfo.set(compute);
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
		builder.create(data.PSSM_Depths, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,renders_size ,1}, ResourceFlags::None);
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

				graphics.set_layout(Layouts::DefaultLayout);
				compute.set_layout(Layouts::DefaultLayout);


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

				auto depth_tex = data.PSSM_Depths->create_2d_slice(i, *graph.builder.current_frame);
			//	auto shadow_cameras = _context.get_buffer(data.shadow_cameras);

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

				data.PSSM_Cameras->write(i * sizeof(camera::shader_params), reinterpret_cast<Table::Camera*>(&light_cam.camera_cb.current), 1);
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

				{
					Slots::FrameInfo frameInfo;

					frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
					auto camera = frameInfo.MapCamera();
					//	memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
					camera.cb = light_cam.camera_cb.current;
					frameInfo.set(graphics);
					frameInfo.set(compute);
				}

				renderer->render(context, scene->get_ptr<Scene>());
			}
			);
		znear = zfar;
	}


	// relight pass
	graph.add_pass<PSSMData>("PSSM_GenerateMask", [this, &graph](PSSMData& data, TaskBuilder& builder) {

		builder.create(data.LightMask, { ivec3(graph.frame_size,1), DXGI_FORMAT::DXGI_FORMAT_R8_UNORM,1,1 }, ResourceFlags::RenderTarget);
		data.gbuffer.need(builder);
		builder.need(data.PSSM_Depths, ResourceFlags::PixelRead);
		builder.need(data.PSSM_Cameras, ResourceFlags::None);
		
		}, [this, &graph](PSSMData& data, FrameContext& _context) {

			GBuffer gbuffer = data.gbuffer.actualize(_context);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();
			auto& compute = list.get_compute();

			//list.set_my_heap();// set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_csu());
			graphics.set_layout(Layouts::DefaultLayout);
			compute.set_layout(Layouts::DefaultLayout);

			graph.set_slot(SlotID::FrameInfo, graphics);
			graph.set_slot(SlotID::FrameInfo, compute);

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.MapGbuffer());

				//lighting.GetLight_mask() = screen_light_mask.get_srv();

				lighting.set(graphics);
			}

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


			graphics.set_viewport(data.LightMask->get_viewport());
			graphics.set_scissor(data.LightMask->get_scissor());

			graphics.set_rtv(1, data.LightMask->renderTarget, Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::PSSMMask>());


			{
				Slots::PSSMData pssmdata;
				pssmdata.GetLight_buffer() = data.PSSM_Depths->texture2DArray;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				pssmdata.set(graphics);
			}

			for (int i = renders_size - 1; i >= 0; i--) //rangeees
			{
				//		PROFILE_GPU((std::wstring(L"renders") + std::to_wstring(i)).c_str());
				{
					Slots::PSSMConstants constants;
					constants.GetLevel() = i;
					constants.GetTime() = 0;
					constants.set(graphics);
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
		}else
		{
			builder.need(data.LightMask, ResourceFlags::PixelRead);

		}
		}, [this, &graph](PSSMData& data, FrameContext& _context) {

			GBuffer gbuffer = data.gbuffer.actualize(_context);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();
			auto& compute = list.get_compute();

			//list.set_my_heap();// set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_csu());
			graphics.set_layout(Layouts::DefaultLayout);
			compute.set_layout(Layouts::DefaultLayout);

			graph.set_slot(SlotID::FrameInfo, graphics);
			graph.set_slot(SlotID::FrameInfo, compute);

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.MapGbuffer());
				lighting.set(graphics);
			}

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			
			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());

			{
				Slots::PSSMData pssmdata;
			//	pssmdata.GetLight_buffer() = data.PSSM_Depths->texture2DArray;
				pssmdata.GetLight_cameras() = data.PSSM_Cameras->structuredBuffer;
				pssmdata.set(graphics);
			}

			{
				Slots::PSSMLighting lighting;

				gbuffer.SetTable(lighting.MapGbuffer());

				if(data.RTXDebug)
				lighting.GetLight_mask() = data.RTXDebug->texture2D;//data.LightMask->texture2D;
				else
					lighting.GetLight_mask() = data.LightMask->texture2D;

				lighting.set(graphics);
			}

			graphics.set_rtv(1, data.ResultTexture->renderTarget, Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::PSSMApply>());

			graphics.draw(4);

		});

}