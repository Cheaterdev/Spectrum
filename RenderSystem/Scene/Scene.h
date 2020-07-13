#pragma once

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

    //		std::set<MaterialAsset*> materials;
    Scene() 
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

        mesh_infos = std::make_shared< virtual_gpu_buffer<mesh_info_part>>(1024 * 1024 * 64);
    }

    Events::Event<scene_object*> on_element_add;
    Events::Event<scene_object*> on_element_remove;
    Events::Event<scene_object*> on_moved;
    Events::Event<scene_object*> on_changed;

    
    std::set<MeshAssetInstance*> static_objects;
	std::set<MeshAssetInstance*> dynamic_objects;


 
	static const int MAX_COMMANDS_SIZE = 1024 * 1024 * 64;
    virtual_gpu_buffer<mesh_info_part>::ptr mesh_infos;// (MAX_COMMANDS_SIZE)

    my_unique_vector<UINT> command_ids;
	std::set<materials::universal_material*> mats;
	std::map<size_t, materials::Pipeline::ptr> pipelines;

	Slots::SceneData::Compiled compiledScene;
	Slots::FrameInfo::Compiled compiledFrame;
    Slots::GatherPipelineGlobal::Compiled compiledGather;

    void update(FrameResources& frame)
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


			auto info = frame.place_raw(command_ids);
			auto srv = info.resource->create_view<FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(frame, info.offset, info.size).get_srv();

			{
				auto timer = Profiler::get().start(L"SceneData");
				Slots::SceneData sceneData;
				sceneData.GetNodes() = universal_nodes_manager::get().buffer->get_srv()[0];
				sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
				sceneData.GetVertexes() = universal_vertex_manager::get().buffer->get_srv()[0];
			    sceneData.GetMaterials() = universal_material_info_part_manager::get().buffer->get_srv()[0];
				sceneData.GetMeshes() = scene->mesh_infos->buffer->get_srv()[0];

				compiledScene = sceneData.compile(frame);


			}


            {
                //	auto timer = list.start(L"GatherMat");
                Slots::GatherPipelineGlobal gather_global;
                gather_global.GetMeshes_count() = command_ids.size();
                gather_global.GetCommands() = srv;

                compiledGather = gather_global.compile(frame);

            }


    }
  //  PipelineHolderManager pipelines;


    /*	virtual void on_element_add(scene_object* node)
        {
            if(node->)
        }

        virtual void on_element_remove(scene_object* node) {

        }*/
        /*
          virtual base_tree* get_root() override
          {
              return this;
          }

          virtual scene_object* get_child(scene_object* node, int i) override;

          virtual int get_child_count(scene_object* node) override;
          virtual void add_element(scene_object* parent, scene_object* node)
          {
              auto p = node->get_ptr();
              parent->add_child(p);
          }

          virtual std::string get_name(scene_object*node){ return node->name; }*/

};