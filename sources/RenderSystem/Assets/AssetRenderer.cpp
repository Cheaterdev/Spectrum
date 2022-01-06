#include "pch_render.h"

#include "Assets/EngineAssets.h"
#include "Effects/Sky.h"
#include "Lighting/PSSM.h"

#include "AssetRenderer.h"
#include "Helpers/MipMapGeneration.h"

import Queue;

using namespace FrameGraph;
import HAL.Types;
using namespace HAL;
class SceneRenderWorkflow
{

	
	PSSM pssm;
	SkyRender sky;

public:
	float time = 0;

	main_renderer::ptr scene_renderer;

	void render(Graph& graph)
	{
		graph.scene->update(*graph.builder.current_frame);
		
		{

			CommandList::ptr command_list = Render::Device::get().get_queue(CommandListType::DIRECT)->get_free_list();

			command_list->begin("pre");
			{
				auto scene = graph.scene->get_ptr<Scene>().get();

				SceneFrameManager::get().prepare(command_list, *scene);
			}

			command_list->end();

			command_list->execute();
		}



		auto size = graph.frame_size;

		{

			
			struct GBufferData
			{
				GBufferViewDesc gbuffer;

				Handlers::Texture H(GBuffer_HiZ);
				Handlers::Texture H(GBuffer_HiZ_UAV);
			};

			graph.add_pass<GBufferData>("GBUFFER", [this, size](GBufferData& data, TaskBuilder& builder) {
				data.gbuffer.create(size, builder);
				//	data.gbuffer.create_mips(size, builder);
				//	data.gbuffer.create_quality(size, builder);


				 builder.create(data.GBuffer_HiZ, { ivec3(size / 8, 1 ), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, 1},ResourceFlags::DepthStencil);
				 builder.create(data.GBuffer_HiZ_UAV, { ivec3(size / 8, 1), DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,1 }, ResourceFlags::UnorderedAccess);

				}, [this,&graph](GBufferData& data, FrameContext& _context) {

					auto& command_list = _context.get_list();
		
					//graph.scene->init_ras();

					command_list->get_graphics().set_layout(Layouts::DefaultLayout);
					command_list->get_compute().set_layout(Layouts::DefaultLayout);

					MeshRenderContext::ptr context(new MeshRenderContext());
					context->current_time = (size_t)time;
					context->priority = TaskPriority::HIGH;
					context->list = command_list;

					context->cam = graph.cam;

					GBuffer gbuffer = data.gbuffer.actualize(_context);

					gbuffer.rtv_table = RenderTargetTable(context->list->get_graphics(), { gbuffer.albedo, gbuffer.normals, gbuffer.specular, gbuffer.speed }, gbuffer.depth);

					gbuffer.HalfBuffer.hiZ_depth = *data.GBuffer_HiZ;
					gbuffer.HalfBuffer.hiZ_table = RenderTargetTable(context->list->get_graphics(), {  }, gbuffer.HalfBuffer.hiZ_depth);
					gbuffer.HalfBuffer.hiZ_depth_uav = *data.GBuffer_HiZ_UAV;
				
					context->g_buffer = &gbuffer;

					gbuffer.rtv_table.set(context, true, true);
					gbuffer.rtv_table.set_window(context->list->get_graphics());

					graph.set_slot(SlotID::FrameInfo, command_list->get_graphics());
					graph.set_slot(SlotID::FrameInfo, command_list->get_compute());


					scene_renderer->render(context, graph.scene->get_ptr<Scene>());


					//	MipMapGenerator::get().copy_texture_2d_slow(command_list->get_graphics(), texture.texture, gbuffer.albedo);
				});
		}

	

		pssm.generate(graph);
		sky.generate(graph);
		sky.generate_sky(graph);

		struct no
		{
			Handlers::Texture H(ResultTexture);
		};
		graph.add_pass<no>("mip", [this, &graph](no& data, TaskBuilder& builder) {
			builder.need(data.ResultTexture, ResourceFlags::UnorderedAccess);
			}, [](no& data, FrameContext& _context) {
			
			
				MipMapGenerator::get().generate(_context.get_list()->get_compute(), *data.ResultTexture);
			});


		graph.add_slot_generator([this](Graph& graph) {

			PROFILE(L"FrameInfo");
			Slots::FrameInfo frameInfo;
			//// hack zone
			auto &sky = graph.builder.alloc_resources["sky_cubemap_filtered"];
			if (sky.resource)
				frameInfo.GetSky() = sky.get_handler<Handlers::Texture>()->textureÑube;
			/////////
			frameInfo.GetSunDir().xyz = graph.sunDir;
			frameInfo.GetTime() = { graph.time ,graph.totalTime,0,0 };


			frameInfo.MapCamera() = graph.cam->camera_cb.current;
			frameInfo.MapPrevCamera() = graph.cam->camera_cb.prev;

			frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d()->texture2D;

			auto compiled = frameInfo.compile(*graph.builder.current_frame);
			graph.register_slot_setter(compiled);
		});

		graph.add_slot_generator([this](Graph& graph) {
			graph.register_slot_setter(graph.scene->compiledScene);
			});
		
//		graph.add_pass([])
	}

};



void AssetRenderer::draw(Scene::ptr scene, Render::Texture::ptr result)
{

	

   return;
 
	graph.start_new_frame();
	if (!vr_context)
	{
		vr_context = std::make_shared<Render::OVRContext>();
	}
	

    scene->update_transforms();

	auto mn = scene->get_min();
	auto mx = scene->get_max();
	scene->add_child(mesh_plane);
	scene->update(*graph.builder.current_frame);

	float x = mx.x - mn.x;
	float y = mx.y - mn.y;


	mesh_plane->local_transform.scaling(std::max(x, y) );

	mesh_plane->local_transform.a42 = mn.y;
	scene->update_transforms();

    cam.target = (mn + mx) / 2;
    cam.position = cam.target + (vec3(30, 10, 30).normalize()) * ((mx - mn).length());
    cam.set_projection_params(Math::pi / 4, 1, 1, 100 + (mn - mx).length());
    cam.update();


	graph.sunDir = float3(1,1,1).normalize();

	graph.builder.pass_texture("ResultTexture", result, ResourceFlags::Required);
	graph.frame_size = result->get_size();
	graph.scene = scene.get();
	graph.renderer = scene_renderer.get();
	graph.cam = &cam;

	rendering->render(graph);
	graph.setup();
	graph.compile(frame++);
	graph.render();
	graph.commit_command_lists();
	graph.reset();
	mesh_plane->remove_from_parent();
}

void AssetRenderer::draw(MaterialAsset::ptr mat, Render::Texture::ptr result)
{
	std::lock_guard<std::mutex> g(lock);

	scene->add_child(material_tester);
	material_tester->override_material(1, mat);

	draw(scene,result);
	material_tester->remove_from_parent();
}

void AssetRenderer::draw(scene_object::ptr obj, Render::Texture::ptr result)
{
	std::lock_guard<std::mutex> g(lock);

	scene->add_child(obj);
	draw(scene, result);
	obj->remove_from_parent();
}

AssetRenderer::AssetRenderer()
{
	std::lock_guard<std::mutex> g(lock);

    scene_renderer = std::make_shared<main_renderer>();
    scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
    cam.position = vec3(0, 5, -30);

	mesh_plane.reset(new MeshAssetInstance(EngineAssets::plane.get_asset()));
    material_tester.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

	scene = std::make_shared<Scene>();
	
	
	rendering = std::make_shared<SceneRenderWorkflow>();
	rendering->scene_renderer = scene_renderer;
//	ssgi = std::make_shared<SSGI>(*gbuffer);

}
