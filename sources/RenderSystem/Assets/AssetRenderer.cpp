
module  Graphics:AssetRenderer;

import HAL;
import :MeshRenderer;
import :Sky;
import :PSSM;
import :FrameGraphContext;
import :MipMapGenerator;
import :EngineAssets;
import :BRDF;

using namespace FrameGraph;
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
		auto& sceneinfo = graph.get_context<SceneInfo>();
		auto scene = sceneinfo.scene->get_ptr<Scene>().get();

		scene->update(*graph.builder.current_frame);

		{

			CommandList::ptr command_list = (HAL::Device::get().get_queue(CommandListType::DIRECT)->get_free_list());

			command_list->begin("pre");
			{
		
				SceneFrameManager::get().prepare(command_list, *scene);
			}

			command_list->execute();
		}

		auto& frame = graph.get_context<ViewportInfo>();


		auto size = frame.frame_size;

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


				builder.create(data.GBuffer_HiZ, { ivec3(size / 8, 0), HAL::Format::R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
				builder.create(data.GBuffer_HiZ_UAV, { ivec3(size / 8, 0), HAL::Format::R32_FLOAT,1 }, ResourceFlags::UnorderedAccess);

				}, [this, &graph](GBufferData& data, FrameContext& _context) {

					auto& command_list = _context.get_list();
					auto& frame = graph.get_context<ViewportInfo>();
					auto& scene = graph.get_context<SceneInfo>();
					auto& cam = graph.get_context<CameraInfo>();

					//graph.scene->init_ras();

					command_list->get_graphics().set_signature(Layouts::DefaultLayout);
					command_list->get_compute().set_signature(Layouts::DefaultLayout);

					MeshRenderContext::ptr context(new MeshRenderContext());
					context->current_time = (size_t)time;
					context->priority = TaskPriority::HIGH;
					context->list = command_list;

					context->cam = cam.cam;

					GBuffer gbuffer = data.gbuffer.actualize(_context);
					gbuffer.HalfBuffer.hiZ_depth = *data.GBuffer_HiZ;
					gbuffer.HalfBuffer.hiZ_depth_uav = *data.GBuffer_HiZ_UAV;

					{
						RT::GBuffer rtv;
						rtv.GetAlbedo() = gbuffer.albedo.renderTarget;
						rtv.GetNormals() = gbuffer.normals.renderTarget;
						rtv.GetSpecular() = gbuffer.specular.renderTarget;

						rtv.GetMotion() = gbuffer.speed.renderTarget;
						rtv.GetDepth() = gbuffer.depth.depthStencil;
						gbuffer.compiled = rtv.compile(*command_list);
					}

					{
						RT::DepthOnly rtv;

						rtv.GetDepth() = gbuffer.HalfBuffer.hiZ_depth.depthStencil;

							gbuffer.HalfBuffer.compiled = rtv.compile(*command_list);
					}
					
					context->g_buffer = &gbuffer;


					gbuffer.compiled.set(context->list->get_graphics(), RTOptions::Default| RTOptions::ClearAll);

					graph.set_slot(SlotID::FrameInfo, command_list->get_graphics());
					graph.set_slot(SlotID::FrameInfo, command_list->get_compute());


					scene_renderer->render(context, scene.scene->get_ptr<Scene>());


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
			auto& time = graph.get_context<TimeInfo>();
			auto& skyinfo = graph.get_context<SkyInfo>();
			auto& cam = graph.get_context<CameraInfo>();
			PROFILE(L"FrameInfo");
			Slots::FrameInfo frameInfo;
			//// hack zone
			auto sky = graph.builder.get("sky_cubemap_filtered");
			if (sky && sky->resource)
				frameInfo.GetSky() = sky->get_handler<Handlers::Cube>()->textureCube;
			/////////
			frameInfo.GetSunDir().xyz = skyinfo.sunDir;
			frameInfo.GetTime() = { time.time ,time.totalTime,0,0 };


			frameInfo.GetCamera() = cam.cam->camera_cb.current;
			frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;

			frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;

			auto compiled = frameInfo.compile(*graph.builder.current_frame);
			graph.register_slot_setter(compiled);
			});

		graph.add_slot_generator([this](Graph& graph) {
			auto& scene = graph.get_context<SceneInfo>();

			graph.register_slot_setter(scene.scene->compiledScene);
			});

		//		graph.add_pass([])
	}

};



void AssetRenderer::draw(Scene::ptr scene, HAL::Texture::ptr result)
{

	graph.start_new_frame();


	scene->update_transforms();

	auto mn = scene->get_min();
	auto mx = scene->get_max();
	scene->add_child(mesh_plane);
	scene->update(*graph.builder.current_frame);

	float x = mx.x - mn.x;
	float y = mx.y - mn.y;


	mesh_plane->local_transform.scaling(std::max(x, y));

	mesh_plane->local_transform.a42 = mn.y;
	scene->update_transforms();


	auto& time = graph.get_context<TimeInfo>();
	auto& skyinfo = graph.get_context<SkyInfo>();
	auto& caminfo = graph.get_context<CameraInfo>();
	auto& sceneinfo = graph.get_context<SceneInfo>();
	auto& vp = graph.get_context<ViewportInfo>();


	cam.target = (mn + mx) / 2;
	cam.position = cam.target + (vec3(30, 10, 30).normalize()) * ((mx - mn).length());
	cam.set_projection_params(Math::pi / 4, 1, 1, 100 + (mn - mx).length());
	cam.update();


	skyinfo.sunDir = float3(1, 1, 1).normalize();

	graph.builder.pass_texture("ResultTexture", result, ResourceFlags::Required);
	vp.frame_size = result->get_size().xy;
	sceneinfo.scene = scene.get();
	sceneinfo.renderer = scene_renderer.get();
	caminfo.cam = &cam;

	rendering->render(graph);
	graph.setup();
	graph.compile(frame++);
	graph.render();
	graph.commit_command_lists();
	graph.reset();
	mesh_plane->remove_from_parent();
}

void AssetRenderer::draw(MaterialAsset::ptr mat, HAL::Texture::ptr result)
{
	std::lock_guard<std::mutex> g(lock);

	scene->add_child(material_tester);
	material_tester->override_material(1, mat);

	draw(scene, result);
	material_tester->remove_from_parent();
}

void AssetRenderer::draw(scene_object::ptr obj, HAL::Texture::ptr result)
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
