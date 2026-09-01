export module Graphics:Scene;

import :SceneObject;
import :MaterialAsset;
import :MeshAsset;

import HAL;

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
export class Scene : public scene_object, Events::prop_handler
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

    
    std::set<scene_object*> static_objects;
	std::set<scene_object*> dynamic_objects;

	// Contiguous, already-typed view of the two sets above. The sets only ever
	// receive MeshAssetInstance (see the on_element_add handler), so iterating
	// this needs no per-object dynamic_cast.
	std::vector<MeshAssetInstance*> mesh_objects;


 
	static const int MAX_COMMANDS_SIZE = 1024 * 1024 * 64;
    HAL::virtual_gpu_buffer<Table::MeshCommandData>::ptr mesh_infos;// (MAX_COMMANDS_SIZE)
    HAL::virtual_gpu_buffer<HAL::InstanceDesc>::ptr raytrace;

	my_unique_vector<UINT> command_ids[10];

	// Derived from the scene contents, which change far less often than once a
	// frame -- rebuilt on demand rather than from scratch every update().
	std::set<MaterialAsset*> mats;
	std::map<size_t, materials::Pipeline::ptr> pipelines;

	// Must be called by anything that adds/removes an object. Material and
	// pipeline changes are picked up via universal_material::pipeline_epoch.
	void invalidate_scene_caches();

	Slots::SceneData::Compiled compiledScene;

    Slots::GatherPipelineGlobal::Compiled compiledGather[8];


	Slots::VoxelInfo::Compiled voxels_compiled;
   
	Slots::VoxelInfo voxel_info;


   bool init_ras(CommandList::ptr& list);

private:
    bool     scene_caches_dirty   = true;
    uint64_t cached_material_epoch = 0;

    void rebuild_scene_caches();

public:

    void update(HAL::FrameResources& frame);


    void iterate_meshes(MESH_TYPE mesh_type, std::function<void(scene_object::ptr)> f);


   

};