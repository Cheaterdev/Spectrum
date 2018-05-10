#pragma once

class stencil_renderer : public renderer, public GUI::base
{
        RenderTargetTable table;
        Render::StructuredBuffer<UINT>::ptr id_buffer;
        Render::PipelineState::ptr draw_state;
        Render::PipelineState::ptr draw_selected_state;

        camera cam;
        camera axis_intersect_cam;

        std::vector<std::pair<MeshAssetInstance::ptr, int>> all[2];
        int current_frame = 0;


        Render::Texture::ptr depth_tex;

        RenderTargetTable render_color_table;


        /////////////////////////////
        Render::PipelineState::ptr  last_render_state;
        Render::PipelineState::ptr  axis_render_state;
        Render::StructuredBuffer<UINT>::ptr axis_id_buffer;

        MeshAssetInstance::ptr axis;

        camera axis_cam;

        //  bool need_update = false;
        int selected_axis = -1;
        // float3 added = 0;
        float3 center_pos;
        float3 pivot_pos;
        std::pair<MeshAssetInstance::ptr, int> current_selected;
        ////////////////////////////

        void select_current()
        {
            selected.clear();

            if (mouse_on_object.first)
            {
                selected.push_back(mouse_on_object);
                auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
                pivot_pos = -(draw.mesh->primitive->get_min() + draw.mesh->primitive->get_max()) / 2;
              //  center_pos = mouse_on_object.first->nodes[draw.node_index]->mesh_matrix[3] - pivot_pos
				center_pos = vec3(mouse_on_object.first->local_transform[3].xyz) - pivot_pos;
            }
        }
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos)override;

        virtual     bool on_mouse_move(vec2 pos) override;

        float3 mouse_pos;
        float3 prev_mouse_pos;
        int mouse_on_axis = -1;

        float3 get_axis(int axis)
        {
            return float3(axis == 0, axis == 1, axis == 2);
        }

        float3 get_current_pos()
        {
        //    Plane p(get_normal(selected_axis), center_pos);
            Ray r(cam.position, direction);
            vec3 res;
            r.intersect(current_plane, res);
            return res;
        }
        float3 get_normal(int axis)
        {
            auto n1 = float3(axis == 1, (axis == 0 || axis == 2), 0);
            auto n2 = float3(axis == 2, 0, axis == 1 || axis == 0);
            return abs(vec3::dot(n1, direction)) > abs(vec3::dot(n2, direction)) ? n1 : n2;
        }
        std::pair<MeshAssetInstance::ptr, int> mouse_on_object;



        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override
        {
            if (p->name != "asset") return false;

            return true;
        }
        void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2)override
        {
            auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();

            if (!item)
                return ;

            if (item->asset->get_type() == Asset_Type::MATERIAL)
                select_current();
        }

        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override
        {
            auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();

            if (!item)
                return false;

            run_on_ui([this, item]()
            {
                auto asset = item->asset->get_asset();

                if (!asset) return;

				if (asset->get_type() == Asset_Type::MESH)
				{
					auto mesh = std::static_pointer_cast<MeshAsset>(item->asset->get_asset());

					user_ui->message_box("static?", "static?", [this, mesh](bool v) {
						MeshAssetInstance::ptr m(new MeshAssetInstance(mesh));

					/*	m->iterate([](scene_object*obj) {
							obj->resizes_parent = false;

							return true;
						
						});*/
						m->type = v?MESH_TYPE::STATIC: MESH_TYPE::DYNAMIC;
						run_on_ui([this, mesh,m]()
						{
							scene->add_child(m);
						});
					
					
					});


                }

                if (asset->get_type() == Asset_Type::MATERIAL)
                {
                    auto material = item->asset->get_asset()->get_ptr<MaterialAsset>();
                    run_on_ui([this, material]()
                    {
                        //	MeshAssetInstance::ptr m(new MeshAssetInstance(mesh));

                        //scene->add_child(m);
                        if (mouse_on_object.first)
                        {
                            auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
                            mouse_on_object.first->override_material(draw.mesh->material, material);
                        }
                    });
                }
            });
            return true;
            //throw std::exception("The method or operation is not implemented.");
        }

		Plane current_plane;

    public:

        scene_object::ptr scene;
        camera* player_cam;
        void update()
        {
            //       need_update = true;
        }
        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;
        stencil_renderer();
        Render::Texture::ptr color_tex;

      
        virtual  bool add_object(Render::renderable* obj) override { return true; }
        virtual void render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj) override;

        void draw_after(MeshRenderContext::ptr mesh_render_context, G_Buffer& buffer);

		virtual void iterate(MESH_TYPE mesh_type,  std::function<void(scene_object::ptr &) > f) override;
		virtual void on_bounds_changed(const rect& r) override;

};