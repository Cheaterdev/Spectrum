export module Graphics:StencilRenderer;
import :Camera;
import :MeshAsset;
import :Scene;

import GUI;

export class stencil_renderer : public GUI::base, public FrameGraph::GraphGenerator, public Events::Runner, public VariableContext
{
        camera cam;
        camera axis_intersect_cam;

        MeshAssetInstance::ptr axis;
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

		HAL::StructureBuffer<vec4>::ptr vertex_buffer;
		HAL::IndexBuffer::ptr index_buffer;

        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;
		virtual void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2)override;
        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override;

		Plane current_plane;

    public:
		virtual void generate(FrameGraph::Graph& graph) override;
		void generate_after(FrameGraph::Graph& graph);
		Scene::ptr debug_scene;

        Scene::ptr scene;
        camera* player_cam;

        Variable<bool> draw_aabb = { false,"Draw AABB", this };


        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;
        stencil_renderer();
};