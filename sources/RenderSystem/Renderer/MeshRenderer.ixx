export module Graphics:MeshRenderer;

import Core;
import HAL;
import :Context;
import :Scene;
import :SceneObject;
import :TextureAsset;
import :MeshAsset;

using namespace HAL;
export
{

class renderer
{

    public:
        virtual ~renderer();

        using ptr = s_ptr<renderer>;

        virtual  bool add_object(Graphics::renderable* obj) = 0;
        virtual void render(MeshRenderContext::ptr c, Scene::ptr obj) = 0;;

		virtual void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)=0;
};
struct mat_info
{
	int index;
	int draw_count;
};


class main_renderer
{
        std::set<renderer::ptr> renderers;

    public:
        using ptr = s_ptr<main_renderer>;


        void register_renderer(renderer::ptr r);

        template<class T>
        void register_renderer(T r)
        {
            renderers.insert(std::static_pointer_cast<renderer>(r));
        }
        void render(MeshRenderContext::ptr c, Scene::ptr obj);

		void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f);
};

class mesh_renderer : public renderer, public Events::prop_handler, VariableContext
{
		TextureAsset::ptr best_fit_normals;

        //    MeshRenderContext::ptr mesh_render_context;
        virtual bool add_object(Graphics::renderable* obj) override;

        virtual void render(MeshRenderContext::ptr mesh_render_context, Scene::ptr obj) override;

		void iterate(MESH_TYPE mesh_type,  std::function<void(scene_object::ptr&)> f) override;
		// gatherData supplies the shader-side bound check; dispatch_args (when
		// non-null) drives an indirect dispatch, otherwise direct_count is used
		// for a CPU-sized direct dispatch (count known on CPU — no indirect).
		void  render_meshes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, std::map<size_t, materials::Pipeline::ptr>& pipelines, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling, HAL::StructuredBufferView<DispatchArguments>* dispatch_args, UINT direct_count);
		void  draw_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene);
		void  generate_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::GatherPipelineGlobal::Compiled& gatherData, HAL::StructuredBufferView<DispatchArguments>* dispatch_args, UINT direct_count);
		void  gather_rendered_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, bool invisibleToo);


		virtual_gpu_buffer<Table::BoxInfo>::ptr commands_boxes;
		virtual_gpu_buffer<Table::CommandData>::ptr commands_buffer[8];


		// Indirect args written by the producer shaders (InterlockedMax) and
		// zeroed per stage via clear_uav (CLEAR sync = real barrier before the
		// COMPUTE producers) — no InitDispatch round trips.
		HAL::StructuredBufferView<DispatchArguments> gather_meshes_args; // sized by commands_boxes count
		HAL::StructuredBufferView<DispatchArguments> render_args;        // sized by meshes_ids count
		HAL::StructuredBufferView<DispatchArguments> retest_args;        // sized by meshes_invisible_ids count


		HAL::StructuredBufferView<DrawIndexedArguments> draw_boxes_first;
		HAL::StructuredBufferView<vec4> vertex_buffer;
		HAL::IndexBuffer index_buffer;
		virtual_gpu_buffer<UINT>::ptr visible_boxes;
		virtual_gpu_buffer<UINT>::ptr meshes_ids;
		virtual_gpu_buffer<UINT>::ptr meshes_invisible_ids;

		Slots::GatherPipelineGlobal::Compiled gather_visible;
		Slots::GatherPipelineGlobal::Compiled gather_invisible;
		Slots::GatherPipelineGlobal::Compiled gather_boxes_commands;

		Slots::GatherMeshesBoxes::Compiled gather_neshes_boxes_compiled;

		Slots::DrawBoxes::Compiled draw_boxes_compiled;
		Slots::GatherBoxes::Compiled gather_boxes_compiled;
    public:
        unsigned int rendered_simple;
        unsigned int rendered_instanced;
        unsigned int instances_count;

        Variable<bool> use_gpu_occlusion = { true,"GPU culling", this };
	/*	Variable<bool> use_cpu_culling = Variable<bool>(false, "use_cpu_culling", this);
		Variable<bool> use_gpu_culling = Variable<bool>(false, "use_gpu_culling", this);
		Variable<bool> clear_depth = Variable<bool>(true, "clear_depth", this);*/
	
       // HAL::RootSignature::ptr my_signature;

        using ptr = s_ptr<mesh_renderer>;
        mesh_renderer();

};
}