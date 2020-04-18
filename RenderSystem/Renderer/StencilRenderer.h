#pragma once

class stencil_renderer : public renderer, public GUI::base, public FrameGraphGenerator
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
        PlacedAllocator allocator;
        camera axis_cam;

        //  bool need_update = false;
        int selected_axis = -1;
        // float3 added = 0;
        float3 center_pos;
        float3 pivot_pos;
        std::pair<MeshAssetInstance::ptr, int> current_selected;
        ////////////////////////////

        void select_current();
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos)override;

        virtual     bool on_mouse_move(vec2 pos) override;

        float3 mouse_pos;
        float3 prev_mouse_pos;
        int mouse_on_axis = -1;

        float3 get_axis(int axis);

        float3 get_current_pos();
        float3 get_normal(int axis);
        std::pair<MeshAssetInstance::ptr, int> mouse_on_object;



        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;
		virtual void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2)override;
        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override;

		Plane current_plane;

    public:
		virtual void generate(FrameGraph& graph) override;
		void generate_after(FrameGraph& graph);

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