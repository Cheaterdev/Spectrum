

class renderer
{

    public:
        virtual ~renderer()
        {
        }

        using ptr = s_ptr<renderer>;

        virtual  bool add_object(Render::renderable* obj) = 0;
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


        void register_renderer(renderer::ptr r)
        {
            renderers.insert(r);
        }

        template<class T>
        void register_renderer(T r)
        {
            renderers.insert(std::static_pointer_cast<renderer>(r));
        }
        void render(MeshRenderContext::ptr c, Scene::ptr obj)
        {
            /*  obj->iterate([&](scene_object * node)
              {
                  //  bool node_inside = node->is_node_inside(*c->cam);
                  //    if (!node_inside)
                  //        return false;
                  Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

                  if (render_object && node->is_inside(*c->cam))
                  {
                      for (auto && r : renderers)
                      {
                          if (r->add_object(render_object))
                              break;
                      }
                  }

                  return true;
              });
            */
            for (auto && r : renderers)
                r->render(c, obj);
        }


		void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)>f)
		{
			for (auto && r : renderers)
				r->iterate(mesh_type, f);
		}
};

class mesh_renderer : public renderer, public Events::prop_handler
{
      
        Render::vertex_shader::ptr shader;
        Render::geometry_shader::ptr voxel_geometry_shader;

        IndirectCommand indirect_command_signature;

        IndirectCommand boxes_command;
	

		TextureAsset::ptr best_fit_normals;

        //    MeshRenderContext::ptr mesh_render_context;
        virtual bool add_object(Render::renderable* obj) override
        {
            auto instance = dynamic_cast<MeshAssetInstance*>(obj);
            //   if (instance)
            //    meshes.push_back(instance);
            /*  if (node)
                  nodes.push_back(node);
              else
              {
                  shared_mesh_object* shared_mesh = dynamic_cast<shared_mesh_object*>(obj);
                  instanced_meshes[shared_mesh->mesh].push_back(shared_mesh);
              }*/
            return true;
        }

        virtual void render(MeshRenderContext::ptr mesh_render_context, Scene::ptr obj) override;

		void iterate(MESH_TYPE mesh_type,  std::function<void(scene_object::ptr&)> f) override;
		void  render_meshes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, std::map<size_t, materials::Pipeline::ptr>& pipelines, Slots::GatherPipelineGlobal::Compiled& gatherData, Slots::FrameInfo::Compiled& compiledFrame, bool needCulling);
		void  draw_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::FrameInfo::Compiled& compiledFrame);
		void  generate_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling);
		void  gather_rendered_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::FrameInfo::Compiled& compiledFrame, bool invisibleToo);
		void  init_dispatch(MeshRenderContext::ptr mesh_render_context, Slots::GatherPipelineGlobal::Compiled & from);


		virtual_gpu_buffer<Table::BoxInfo>::ptr commands_boxes;
		virtual_gpu_buffer<Table::CommandData>::ptr commands_buffer[8];


		Render::StructuredBuffer<DispatchArguments>::ptr dispatch_buffer;
        Render::StructuredBuffer<DispatchArguments>::ptr dispatch_buffer111;

        IndirectCommand dispatch_command;

	
		Render::StructuredBuffer<DrawIndexedArguments>::ptr draw_boxes_first;
		Render::StructuredBuffer<vec4>::ptr vertex_buffer;
		Render::IndexBuffer::ptr index_buffer;
		virtual_gpu_buffer<UINT>::ptr visible_boxes;
		virtual_gpu_buffer<UINT>::ptr meshes_ids;
		virtual_gpu_buffer<UINT>::ptr meshes_invisible_ids;

		Slots::GatherPipelineGlobal::Compiled gather_visible;
		Slots::GatherPipelineGlobal::Compiled gather_invisible;
		Slots::GatherPipelineGlobal::Compiled gather_boxes_commands;

		Slots::InitDispatch::Compiled init_dispatch_compiled;
		Slots::GatherMeshesBoxes::Compiled gather_neshes_boxes_compiled;

		Slots::DrawBoxes::Compiled draw_boxes_compiled;
		Slots::GatherBoxes::Compiled gather_boxes_compiled;
    public:
        unsigned int rendered_simple;
        unsigned int rendered_instanced;
        unsigned int instances_count;

		Variable<bool> use_parrallel = Variable<bool>(false,"use_parrallel");
		Variable<bool> use_cpu_culling = Variable<bool>(false, "use_cpu_culling");
		Variable<bool> use_gpu_culling = Variable<bool>(false, "use_gpu_culling");
		Variable<bool> clear_depth = Variable<bool>(true, "clear_depth");
	
       // Render::RootSignature::ptr my_signature;

        using ptr = s_ptr<mesh_renderer>;
        mesh_renderer();

};
