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
    HAL::virtual_gpu_buffer<Table::Meshes::MeshCommandData>::ptr mesh_infos;// (MAX_COMMANDS_SIZE)
    HAL::virtual_gpu_buffer<HAL::InstanceDesc>::ptr raytrace;

	my_unique_vector<UINT> command_ids[10];

	// Derived from the scene contents, which change far less often than once a
	// frame -- rebuilt on demand rather than from scratch every update().
	std::set<MaterialAsset*> mats;
	std::map<size_t, materials::Pipeline::ptr> pipelines;

	// Must be called by anything that adds/removes an object. Material and
	// pipeline changes are picked up via universal_material::pipeline_epoch.
	void invalidate_scene_caches();

	Slots::Frame::SceneData::Compiled compiledScene;

    Slots::Meshes::GatherPipelineGlobal::Compiled compiledGather[8];

    // A BVH over every mesh part of the scene (command_ids[ALL]), for the GPU to cull the whole scene without
    // testing each part (SceneBvhCuller). Built from the parts' world AABBs on the CPU, split into
    // "treelets" of at most Bvh::TreeletLeaves parts: the GPU tests one thread per treelet and walks the
    // treelet's own tree, so the levels above the treelets are never stored. Static and dynamic parts are
    // built separately, so a moving object only rebuilds the dynamic one.
    struct Bvh
    {
        static constexpr uint TreeletLeaves = 32;

        // Eight dwords per node: min.xyz, a, max.xyz, b. A leaf has b == 0 and a = the mesh id; an inner node
        // has the node indices of its children in a and b.
        std::vector<uint32_t> nodes;
        // The root node of each treelet.
        std::vector<uint32_t> treelets;
        // The first treelets, [0, static_treelets), hold the static parts and the rest the dynamic ones.
        uint static_treelets = 0;
        // Changes whenever the arrays do, so a renderer knows to upload them again.
        uint64_t version = 0;
    } bvh;

    // Rebuilds what moved or was added or removed since the last call; call before reading `bvh`.
    void update_bvh();


	Slots::GI::Voxel::VoxelInfo::Compiled voxels_compiled;
   
	Slots::GI::Voxel::VoxelInfo voxel_info;


   bool init_ras(CommandList::ptr& list);

private:
    // Passes record on several threads and each calls update_bvh().
    std::mutex bvh_mutex;
    bool     bvh_static_dirty     = true;
    bool     bvh_dynamic_dirty    = true;
    // The two parts' own trees (node indices relative to the part), combined into `bvh`.
    struct BvhPart
    {
        std::vector<uint32_t> nodes;
        std::vector<uint32_t> treelets;
    } bvh_static, bvh_dynamic;

    bool     scene_caches_dirty   = true;
    uint64_t cached_material_epoch = 0;

    void rebuild_scene_caches();

public:

    void update(HAL::FrameResources& frame);


    void iterate_meshes(MESH_TYPE mesh_type, std::function<void(scene_object::ptr)> f);




};

// Culls a Scene's mesh parts against a convex volume through Scene::Bvh, for any pass that draws the scene: a camera's
// frustum, a light's orthographic box, an AABB (six inward-pointing planes in every case). One per consumer, since it
// owns the GPU copy of the BVH, uploaded when Scene::Bvh::version changes, and the result of the last cull.
export class SceneBvhCuller
{
public:
    SceneBvhCuller();

    // Fills `gather` and `args` with the parts of `type` (STATIC, DYNAMIC or ALL) that touch `volume`. The parts that are
    // kept are a superset of the ones inside it. False when there is nothing to cull through (no BVH, or a scene larger
    // than the buffers): the caller scans the whole list instead.
    bool cull(HAL::CommandList& list, Scene& scene, MESH_TYPE type, const Table::Frame::Frustum& volume);

    // The result in the shape the gathers take as their input list, and the indirect dispatch sized for it.
    Slots::Meshes::GatherPipelineGlobal::Compiled gather;
    HAL::StructuredBufferView<DispatchArguments> args;

    // The buffer holding, in its first dword, how many parts the last cull kept.
    HAL::Buffer::ptr count_buffer() { return ids->buffer.get_counter_buffer(); }

private:
    HAL::virtual_gpu_buffer<UINT>::ptr nodes;
    HAL::virtual_gpu_buffer<UINT>::ptr treelets;
    HAL::virtual_gpu_buffer<UINT>::ptr ids;
    uint64_t uploaded = 0;
};