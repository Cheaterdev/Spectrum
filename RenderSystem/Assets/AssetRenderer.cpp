#include "pch.h"


void AssetRenderer::draw(scene_object::ptr scene, Render::Texture::ptr result)
{
 //  return;
    std::lock_guard<std::mutex> g(lock);

	if (!vr_context)
	{
		vr_context = std::make_shared<Render::OVRContext>();
	}
    Render::CommandList::ptr list = frames.start_frame("AssetRenderer");
	list->get_graphics().set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

    MeshRenderContext::ptr context(new MeshRenderContext());
    context->list = list;
	context->g_buffer = gbuffer.get();
	//context->eye_context = std::make_shared<Render::OVRContext>();

	context->eye_context = vr_context;
	context->eye_context->eyes.resize(1);
	context->eye_context->eyes[0].dir = quat();
	context->eye_context->eyes[0].color_buffer = result;
	context->eye_context->eyes[0].offset = vec3(0, 0, 0);
	context->eye_context->eyes[0].cam = &cam;
	context->eye_context->eyes[0].g_buffer = gbuffer.get();
	gbuffer->reset(context);


    gbuffer->set(context);
	
    scene->update_transforms();

	auto mn = scene->get_min();
	auto mx = scene->get_max();
	scene->add_child(mesh_plane);


	float x = mx.x - mn.x;
	float y = mx.y - mn.y;


	mesh_plane->local_transform.scaling(std::max(x, y) );

	mesh_plane->local_transform.a42 = mn.y;
	scene->update_transforms();

    cam.target = (mn + mx) / 2;
    cam.position = cam.target + (vec3(30, 10, 30).normalize()) * ((mx - mn).length());
    cam.set_projection_params(pi / 4, 1, 1, 100 + (mn - mx).length());
    cam.update();
    context->cam = &cam;
    scene_renderer->render(context, scene);
    gbuffer->end(context);


	if (!sky) sky = std::make_shared<SkyRender>();


	context->sky_dir = float3(-1, 1, 0).normalized();

	sky->update_cubemap(context);

  lighting->process(context, *gbuffer, scene_renderer, scene);

	ssgi->process(context, *gbuffer, sky->cubemap->array_cubemap(),true,1);
	sky->process(context);

    context->list->copy_resource(result.get(), gbuffer->result_tex.first().get());
    context->list->transition(gbuffer->result_tex.first(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
    MipMapGenerator::get().generate(context->list->get_compute(), result);
	context->list->transition(result, Render::ResourceState::PIXEL_SHADER_RESOURCE);

    list->end();
    list->execute_and_wait();

	mesh_plane->remove_from_parent();
}

void AssetRenderer::draw(MaterialAsset::ptr mat, Render::Texture::ptr result)
{
 // return;
  //  std::lock_guard<std::mutex> g(lock);

	
	material_tester->overrided_material[1] = material_tester->register_asset(mat);

	draw(material_scene,result);
	/*
    Render::CommandList::ptr list =  frames.start_frame("AssetRenderer");

	list->get_graphics().set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

    MeshRenderContext::ptr context(new MeshRenderContext());
    context->list = list;
	gbuffer->reset(context);


    gbuffer->set(context);
   
 	auto mn = material_tester->get_min();
	auto mx = material_tester->get_max();
    cam.target = (mn + mx) / 2;
    cam.position = cam.target + (vec3(30, 10, 30).normalize()) * ((mx - mn).length());
    cam.set_projection_params(pi / 4, 1, 1, 100 + (mn - mx).length());
    cam.update();
    context->cam = &cam;
    scene_renderer->render(context, material_tester);
    gbuffer->end(context);
    lighting->process(context, *gbuffer, scene_renderer, material_tester);
    context->list->copy_resource(result, gbuffer->result_tex.first());
    MipMapGenerator::get().generate(list->get_compute(), result, Render::ResourceState::PIXEL_SHADER_RESOURCE);
    list->end();
    list->execute_and_wait();*/
}

AssetRenderer::AssetRenderer()
{
	std::lock_guard<std::mutex> g(lock);

    scene_renderer = std::make_shared<main_renderer>();
    scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
    cam.position = vec3(0, 5, -30);
    gbuffer.reset(new G_Buffer());
	gbuffer->size = {256,256};
    lighting.reset(new LightSystem());
	lighting->pssm.resize(gbuffer->size);
	lighting->pssm.optimize_far = false;

	mesh_plane.reset(new MeshAssetInstance(EngineAssets::plane.get_asset()));
    material_tester.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

	material_scene = std::make_shared<Scene>();

	material_scene->add_child(material_tester);
	material_scene->update_transforms();

	ssgi = std::make_shared<SSGI>(*gbuffer);
}
