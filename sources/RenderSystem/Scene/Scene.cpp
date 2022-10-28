#include "pch_render.h"
#include "Scene.h"
#include "Materials/universal_material.h"
#include "Assets/MeshAsset.h"

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
		});

	on_element_remove.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			static_objects.erase(render_object);

		if (render_object->type == MESH_TYPE::DYNAMIC)
			dynamic_objects.erase(render_object);
		});

	mesh_infos = std::make_shared< virtual_gpu_buffer<Table::MeshCommandData>>(1024 * 1024);
	raytrace = std::make_shared< virtual_gpu_buffer<D3D12_RAYTRACING_INSTANCE_DESC>>(1024 * 1024);



	if (HAL::Device::get().is_rtx_supported())
	{
		std::vector<InstanceDesc>  desc;
		raytrace_scene = std::make_shared<RaytracingAccelerationStructure>(desc);
	}
}

bool Scene::init_ras(Graphics::CommandList::ptr& list)
{
	bool res = false;
	auto mesh_func = [&](MeshAssetInstance* l)
	{
		res |= l->init_ras(list);

	};

	for (auto m : static_objects)
		mesh_func(m);
	for (auto m : dynamic_objects)
		mesh_func(m);

	return res;
}

void Scene::update(Graphics::FrameResources& frame)
{
	mats.clear();
	pipelines.clear();

	auto mesh_func = [&](MeshAssetInstance* l)
	{
		for (auto r : l->rendering)
		{
			auto mat = static_cast<materials::universal_material*>(r.material);
			mats.insert(mat);

			pipelines[mat->get_id()] = mat->get_pipeline();
		}


	};

	for (auto m : static_objects)
		mesh_func(m);
	for (auto m : dynamic_objects)
		mesh_func(m);


	for (auto mat : mats)
	{
		mat->update();
	}


	{
		PROFILE(L"SceneData");
		Slots::SceneData sceneData;
		sceneData.GetNodes() = universal_nodes_manager::get().buffer->structuredBuffer;
		sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
		sceneData.GetVertexes() = universal_vertex_manager::get().buffer->structuredBuffer;
		sceneData.GetMaterials() = universal_material_info_part_manager::get().buffer->structuredBuffer;
		sceneData.GetMeshes() = scene->mesh_infos->buffer->structuredBuffer;
		sceneData.GetMeshInstances() = universal_mesh_instance_manager::get().buffer->structuredBuffer;
		sceneData.GetMeshlets() = universal_meshlet_manager::get().buffer->structuredBuffer;
		sceneData.GetIndices() = universal_index_manager::get().buffer->structuredBuffer;
		sceneData.GetMeshletCullData() = universal_meshletculldata_manager::get().buffer->structuredBuffer;

		compiledScene = sceneData.compile(frame);
	}


	auto build = [&](my_unique_vector<UINT>& data, Slots::GatherPipelineGlobal::Compiled& target) {



		{
			//	auto timer = list.start(L"GatherMat");
			Slots::GatherPipelineGlobal gather_global;
			{
				//	gather_global.GetMeshes_count() = data.size();


				auto info = frame.place_raw(UINT(data.size()));
				auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(frame, StructuredBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size,false }).structuredBuffer;
				gather_global.GetMeshes_count() = srv;
			}


			if (data.size()) {
				auto info = frame.place_raw(data);
				auto srv = info.resource->create_view<HAL::FormattedBufferView<UINT, Graphics::Format::R32_UINT>>(frame, FormattedBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size }).buffer;
				gather_global.GetCommands() = srv;
			}

			target = gather_global.compile(frame);

		}


	};

	build(command_ids[(int)MESH_TYPE::STATIC], compiledGather[(int)MESH_TYPE::STATIC]);
	build(command_ids[(int)MESH_TYPE::DYNAMIC], compiledGather[(int)MESH_TYPE::DYNAMIC]);
	build(command_ids[(int)MESH_TYPE::ALL], compiledGather[(int)MESH_TYPE::ALL]);


}
