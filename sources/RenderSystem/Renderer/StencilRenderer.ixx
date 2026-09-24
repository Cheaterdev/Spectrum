export module Graphics:StencilRenderer;

import :Camera;
import :MeshAsset;
import :Scene;
import :FrameGraphContext;

import FrameGraph;
import HAL;
import GUI;

export class stencil_renderer : public GUI::base, public Events::Runner, public VariableContext
{
        camera cam;

        MeshAssetInstance::ptr axis;
        camera axis_cam;

        int selected_axis = -1;
        float3 center_pos;
        float3 pivot_pos;

        void select_current();
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        virtual bool on_mouse_move(vec2 pos) override;
        virtual void on_key_action(key_action action, long key) override; // camera move

        // Camera fly-controls (active only while this viewport is focused).
        bool camera_keys[256] = {};
        bool looking          = false; // RMB held -> mouse-look
        bool rmb_dragged      = false; // distinguishes look-drag from a menu click
        vec2 look_last;
        void update_move_input();

        float3 mouse_pos;
        float3 prev_mouse_pos;
        int mouse_on_axis = -1;

        float3 get_axis(int axis);
        float3 get_current_pos();
        float3 get_normal(int axis);
        std::pair<MeshAssetInstance::ptr, int> mouse_on_object;

        // Mouse position in [0,1] viewport space; outside that range = not over the viewport.
        vec2 mouse_uv = { -1, -1 };
        std::pair<MeshAssetInstance::ptr, int> find_object(uint object_id);
        static uint object_id_of(const std::pair<MeshAssetInstance::ptr, int>& obj);

        void draw_gizmo(HAL::GraphicsContext& graphics);

        HAL::StructuredBufferView<vec4> vertex_buffer;
        HAL::IndexBuffer index_buffer;

        // Rotation rings (procedural): one flat annulus per axis, at the gizmo origin.
        HAL::StructuredBufferView<vec4> ring_vertex_buffer;
        HAL::IndexBuffer                ring_index_buffer;
        struct ring_range { UINT offset; UINT count; };
        ring_range ring_ranges[3] = {};
        bool       rings_sized     = false; // built once axis bounds are known
        void build_rings(float radius, float thickness);

        // Viewport height (px) at which the gizmo has its base size. The camera
        // distance scales with the actual height so the gizmo stays a constant
        // pixel size instead of a fixed fraction of the ResultTexture. Lower =
        // larger gizmo on screen.
        float gizmo_ref_height = 300.0f;

        // Rotation drag state (selected_axis 0..2 = translate, 3..5 = rotate).
        vec3 rot_prev;
        bool is_rotate() const { return selected_axis >= 3; }

        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;
        virtual void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2) override;
        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override;

        Plane current_plane;

    public:
        Scene::ptr debug_scene;
        Scene::ptr scene;
        first_person_camera* player_cam;

        Variable<bool> draw_aabb = { false, "Draw AABB", this };

        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;

        // Setup is generated (stenciler.prism) -- render only.
        Passes::Editor::stencil_renderer::render_func_type m_render;

        stencil_renderer();

        // Per-frame CPU work (task drain, gizmo ring sizing, camera setup).
        // Called once per frame before graph.setup().
        void update_frame(FrameGraph::Graph& graph);

        template<typename TPipeline>
        explicit stencil_renderer(TPipeline& pipeline) : stencil_renderer()
        {
            pipeline.stencil_renderer.render_func = m_render;
        }
};
