#include "pch.h"

class SceneRenderWorkflow
{

	
	PSSM pssm;
	SkyRender sky;

public:
	float time = 0;

	main_renderer::ptr scene_renderer;

	void render(FrameGraph& graph)
	{

		{

			CommandList::ptr command_list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();

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

				ResourceHandler* hiz;
				ResourceHandler* hiz_uav;
			};

			graph.add_pass<GBufferData>("GBUFFER", [this, size](GBufferData& data, TaskBuilder& builder) {
				data.gbuffer.create(size, builder);
			//	data.gbuffer.create_mips(size, builder);
			//	data.gbuffer.create_quality(size, builder);


				data.hiz = builder.create_texture("GBuffer_HiZ", size / 8, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil);
				data.hiz_uav = builder.create_texture("GBuffer_HiZ_UAV", size / 8, 1, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, ResourceFlags::UnorderedAccess);

				}, [this,&graph](GBufferData& data, FrameContext& _context) {

					auto& command_list = _context.get_list();
		
					//graph.scene->init_ras();

					MeshRenderContext::ptr context(new MeshRenderContext());
					context->current_time = (size_t)time;
					context->priority = TaskPriority::HIGH;
					context->list = command_list;

					context->cam = graph.cam;

					GBuffer gbuffer = data.gbuffer.actualize(_context);

					gbuffer.rtv_table = RenderTargetTable(context->list->get_graphics(), { gbuffer.albedo, gbuffer.normals, gbuffer.specular, gbuffer.speed }, gbuffer.depth);

					gbuffer.HalfBuffer.hiZ_depth = _context.get_texture(data.hiz);
					gbuffer.HalfBuffer.hiZ_table = RenderTargetTable(context->list->get_graphics(), {  }, gbuffer.HalfBuffer.hiZ_depth);
					gbuffer.HalfBuffer.hiZ_depth_uav = _context.get_texture(data.hiz_uav);
				
					context->g_buffer = &gbuffer;

					gbuffer.rtv_table.set(context, true, true);
					gbuffer.rtv_table.set_window(context->list->get_graphics());

					scene_renderer->render(context, graph.scene->get_ptr<Scene>());


					//	MipMapGenerator::get().copy_texture_2d_slow(command_list->get_graphics(), texture.texture, gbuffer.albedo);
				});
		}
		pssm.generate(graph);
		sky.generate(graph);


		struct no
		{
			ResourceHandler* result;
		};
		graph.add_pass<no>("mip", [this, &graph](no& data, TaskBuilder& builder) {
			data.result = graph.builder.need_texture("ResultTexture", ResourceFlags::UnorderedAccess);
			}, [](no& data, FrameContext& _context) {
			
			
				auto result = _context.get_texture(data.result);


				MipMapGenerator::get().generate(_context.get_list()->get_compute(), result);
			});

//		graph.add_pass([])
	}

};



void AssetRenderer::draw(Scene::ptr scene, Render::Texture::ptr result)
{

	//return;
 //  return;
 
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
    cam.set_projection_params(pi / 4, 1, 1, 100 + (mn - mx).length());
    cam.update();




	graph.builder.pass_texture("ResultTexture", result, ResourceFlags::Required);
	graph.frame_size = result->get_size();
	graph.scene = scene.get();
	graph.renderer = scene_renderer.get();
	graph.cam = &cam;

	rendering->render(graph);
	graph.setup();
	graph.compile(frame++);
	graph.render();
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
