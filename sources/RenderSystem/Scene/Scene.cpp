module Graphics:Scene;
import RenderSystem;

import :Materials.UniversalMaterial;
import :MeshAsset;

import HAL;
using namespace HAL;

void Scene::iterate_meshes(MESH_TYPE mesh_type, std::function<void(scene_object::ptr)> f)
{
    if (mesh_type & MESH_TYPE::STATIC)
        for (auto& instance : static_objects)
            f(instance->get_ptr<scene_object>());

    if (mesh_type & MESH_TYPE::DYNAMIC)
        for (auto& instance : dynamic_objects)
            f(instance->get_ptr<scene_object>());
}

Scene::~Scene()
{
	remove_all();
}

Scene::Scene()
{
	scene = this;
	// controller = this;
	on_element_add.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			static_objects.insert(render_object);

		if (render_object->type == MESH_TYPE::DYNAMIC)
			dynamic_objects.insert(render_object);

		invalidate_scene_caches();
		});

	on_element_remove.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			static_objects.erase(render_object);

		if (render_object->type == MESH_TYPE::DYNAMIC)
			dynamic_objects.erase(render_object);

		invalidate_scene_caches();
		});

	mesh_infos = std::make_shared< virtual_gpu_buffer<Table::MeshCommandData>>(RenderSystem::get().device(), 1024 * 1024);
	raytrace = std::make_shared<virtual_gpu_buffer<HAL::InstanceDesc>>(RenderSystem::get().device(), 1024 * 1024);


	if (RenderSystem::get().device().is_rtx_supported())
	{
		std::vector<InstanceDesc>  desc;
		raytrace_scene = std::make_shared<RaytracingAccelerationStructure>(RenderSystem::get().device(), desc);
	}
}

void Scene::invalidate_scene_caches()
{
	scene_caches_dirty = true;
}

void Scene::rebuild_scene_caches()
{
	PROFILE(L"scene_rebuild_caches");

	mesh_objects.clear();
	mats.clear();
	pipelines.clear();

	mesh_objects.reserve(static_objects.size() + dynamic_objects.size());

	{
		PROFILE(L"scene_collect_objects");

		// static_cast, not dynamic_cast: the on_element_add handler already
		// filtered these, so every entry is a MeshAssetInstance by construction.
		for (auto o : static_objects)
			mesh_objects.push_back(static_cast<MeshAssetInstance*>(o));
		for (auto o : dynamic_objects)
			mesh_objects.push_back(static_cast<MeshAssetInstance*>(o));
	}

	{
		PROFILE(L"scene_collect_materials");
		for (auto m : mesh_objects)
			for (auto& r : m->rendering)
				mats.insert(r.material);
	}

	{
		// Once per UNIQUE material. Doing this per rendering entry meant a mesh
		// whose submeshes share a material paid a map write for each of them.
		PROFILE(L"scene_collect_pipelines");
		for (auto mat : mats)
		{
			auto um = static_cast<materials::universal_material*>(mat);
			pipelines[um->get_id()] = um->get_pipeline();
		}
	}

	scene_caches_dirty    = false;
	cached_material_epoch = materials::universal_material::pipeline_epoch.load();
}

bool Scene::init_ras(HAL::CommandList::ptr& list)
{
	if (scene_caches_dirty)
		rebuild_scene_caches();

	bool res = false;

	for (auto m : mesh_objects)
		res |= m->init_ras(list);

	return res;
}

void Scene::update(HAL::FrameResources& frame)
{

	  	PROFILE(L"Scene::update");

	{
		PROFILE(L"scene_collect");

		if (scene_caches_dirty
			|| cached_material_epoch != materials::universal_material::pipeline_epoch.load())
		{
			rebuild_scene_caches();
		}
	}

	{
		PROFILE(L"scene_material_update");
		for (auto mat : mats)
		{
			mat->update();
		}
	}


	{
		PROFILE(L"SceneData");
		Slots::SceneData sceneData;
		sceneData.GetNodes() = universal_nodes_manager::get().buffer;
		sceneData.GetMaterials() = universal_material_info_part_manager::get().buffer;
		sceneData.GetMeshes() = scene->mesh_infos->buffer;
sceneData.GetRaytraceInstanceInfo() = universal_rtx_manager::get().buffer;
//sceneData.GetScene() = raytrace_scene->raytracing_handle;
		compiledScene = sceneData.compile(frame);
	}


	auto build = [&](my_unique_vector<UINT>& data, Slots::GatherPipelineGlobal::Compiled& target) {



		{
			//	auto timer = list.start(L"GatherMat");
			Slots::GatherPipelineGlobal gather_global;
			{
				PROFILE(L"gather_count_view");
				//	gather_global.GetMeshes_count() = data.size();


				auto info = frame.place_raw(UINT(data.size()));
				auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(frame, StructuredBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size,counterType::NONE }).structuredBuffer;
				gather_global.GetMeshes_count() = srv;
			}


			if (data.size()) {
				PROFILE(L"gather_commands_view");
				auto info = frame.place_raw(data);
				auto srv = info.resource->create_view<HAL::FormattedBufferView<UINT, HAL::Format::R32_UINT>>(frame, FormattedBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size }).buffer;
				gather_global.GetCommands() = srv;
			}

			{
				PROFILE(L"gather_compile");
				target = gather_global.compile(frame);
			}

		}


	};

	{
		PROFILE(L"scene_gather_static");
		build(command_ids[(int)MESH_TYPE::STATIC], compiledGather[(int)MESH_TYPE::STATIC]);
	}
	{
		PROFILE(L"scene_gather_dynamic");
		build(command_ids[(int)MESH_TYPE::DYNAMIC], compiledGather[(int)MESH_TYPE::DYNAMIC]);
	}
	{
		PROFILE(L"scene_gather_all");
		build(command_ids[(int)MESH_TYPE::ALL], compiledGather[(int)MESH_TYPE::ALL]);
	}


}
