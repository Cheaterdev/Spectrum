#pragma once

#include "Assets/MeshAsset.h"

import Graphics;


using namespace HAL;
/*
class PipelineHolder
{
public:
    virtual_gpu_buffer<command>::ptr commands;

    std::vector<materials::render_pass> passes;
};

class PipelineHolderManager
{
	std::map<size_t, PipelineHolder> pipelines;
    
public:
    PipelineHolder get_for(materials::universal_material* mat)
    {
        auto id = mat->get_pipeline_id();

        auto pip = pipelines.find(id);
        if (pip == pipelines.end())
        {

            auto&& res = pipelines[id];

            res.passes = mat->passes;
        }

        return pip->second;
    }
};
*/
class Scene : public scene_object, Events::prop_handler
{


    //	std::set<scene_object*> static_objects;
    //	std::set<scene_object*> dynamic_objects;
public:
    using ptr = s_ptr<Scene>;
    using wptr = w_ptr<Scene>;

    virtual ~Scene();
    HAL::RaytracingAccelerationStructure::ptr raytrace_scene;
    Scene();

    Events::Event<scene_object*> on_element_add;
    Events::Event<scene_object*> on_element_remove;
    Events::Event<scene_object*> on_moved;
    Events::Event<scene_object*> on_changed;

    
    std::set<MeshAssetInstance*> static_objects;
	std::set<MeshAssetInstance*> dynamic_objects;


 
	static const int MAX_COMMANDS_SIZE = 1024 * 1024 * 64;
    HAL::virtual_gpu_buffer<Table::MeshCommandData>::ptr mesh_infos;// (MAX_COMMANDS_SIZE)
    HAL::virtual_gpu_buffer<D3D12_RAYTRACING_INSTANCE_DESC>::ptr raytrace;// (MAX_COMMANDS_SIZE)

	my_unique_vector<UINT> command_ids[10];

	std::set<MaterialAsset*> mats;
	std::map<size_t, materials::Pipeline::ptr> pipelines;

	Slots::SceneData::Compiled compiledScene;

    Slots::GatherPipelineGlobal::Compiled compiledGather[8];


	Slots::VoxelInfo::Compiled voxels_compiled;
   
	Slots::VoxelInfo voxel_info;


    bool init_ras(CommandList::ptr& list);

    void update(HAL::FrameResources& frame);


    void iterate_meshes(MESH_TYPE mesh_type, std::function<void(scene_object::ptr)> f)
	{


		if (mesh_type & MESH_TYPE::STATIC)
			for (auto& instance : static_objects)
				f(instance->get_ptr<scene_object>());

		if (mesh_type & MESH_TYPE::DYNAMIC)
			for (auto& instance : dynamic_objects)
				f(instance->get_ptr<scene_object>());
	}


   

};