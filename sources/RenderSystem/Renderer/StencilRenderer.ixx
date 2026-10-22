export module Graphics:StencilRenderer;

import :Camera;
import :MeshAsset;
import :Scene;
import :FrameGraphContext;

import FrameGraph;
import HAL;
import GUI;
import Core;   // Plane; MSVC leaks it through the other partitions' imports

export class stencil_renderer : public GUI::base, public Events::Runner, public VariableContext
{
        camera cam;

        MeshAssetInstance::ptr axis;
        camera axis_cam;
        camera mask_cam;
        float2 main_jitter = { 0, 0 };

        int selected_axis = -1;
        float3 center_pos;
        float3 pivot_pos;

        void select_current();
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        virtual bool on_mouse_move(vec2 pos) override;
        virtual void on_key_action(key_action action, long key, key_mods mods) override; // camera move

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
        void draw_mask(HAL::GraphicsContext& graphics, Passes::Editor::stencil_renderer::Context& data,
                       const std::pair<MeshAssetInstance::ptr, int>& selection);

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

        // Width is in output pixels; the overlays are drawn after tonemapping.
        Variable<float> outline_width     = { 3.0f, "Outline width", this, 1.0f, 6.0f };
        Variable<float> overlay_intensity = { 1.0f, "Overlay intensity", this, 0.5f, 4.0f };
        // Gain on the outline's antialiased edge strength: above 1 the line gets more opaque.
        Variable<float> outline_opacity = { 3.0f, "Outline opacity", this, 0.5f, 8.0f };
        // Weight of last frame's outline: higher = steadier edges, more lag on motion.
        Variable<float> outline_history_weight = { 0.85f, "Outline history", this, 0.0f, 0.97f };

        // Outline accumulation validity (see m_render_outline).
        uint     outline_last_id    = 0;
        uint64_t outline_last_frame = ~0ull;
        ivec2    outline_last_size  = { 0, 0 };

        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;

        // Setup is generated (stenciler.prism) -- render only.
        Passes::Editor::stencil_renderer::render_func_type m_render;
        Passes::Editor::stencil_gizmo_resolve::render_func_type m_render_resolve;
        Passes::Editor::stencil_outline_temporal::render_func_type m_render_outline;

        stencil_renderer();

        // Per-frame CPU work (task drain, gizmo ring sizing, camera setup).
        // Called once per frame before graph.setup().
        void update_frame(FrameGraph::Graph& graph);

        template<typename TPipeline>
        explicit stencil_renderer(TPipeline& pipeline) : stencil_renderer()
        {
            pipeline.stencil_renderer.render_func = m_render;
            pipeline.stencil_gizmo_resolve.render_func = m_render_resolve;
            pipeline.stencil_outline_temporal.render_func = m_render_outline;
        }
};
