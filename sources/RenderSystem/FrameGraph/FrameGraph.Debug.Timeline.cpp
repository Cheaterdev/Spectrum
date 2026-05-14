module FrameGraphDebug;

import windows;
import Core;
import HAL;
import GUI;
import FrameGraph;
import Graphics;
import <HAL.h>;

using namespace GUI::Elements;

class FrameGraphTimelineCanvas : public dock_base
{
    // -----------------------------------------------------------------------
    //  Layout constants
    // -----------------------------------------------------------------------
    static constexpr float LABEL_W    = 162.0f;
    static constexpr float LANE_H     = 40.0f;
    static constexpr float LANE_COUNT = 2.0f;
    static constexpr float ROW_H      = 74.0f;
    static constexpr float COL_W_BASE = 140.0f;
    static constexpr float PASS_H     = 32.0f;
    static constexpr float CELL_H     = 64.0f;
    static constexpr float PAD        = 3.0f;
    static constexpr float LABEL_FONT = 10.0f;
    static constexpr float ICON_W     = 18.0f;  // resource-type icon size in left column

    // -----------------------------------------------------------------------
    //  Data model
    // -----------------------------------------------------------------------
    struct PassInfo
    {
        std::wstring         name;
        UINT                 call_id   = 0;
        HAL::CommandListType queue     = HAL::CommandListType::DIRECT;
        bool                 put_fence = false;
        FrameGraph::Pass*    wait_pass = nullptr;
    };

    struct ResourceCell
    {
        UINT  call_id     = 0;
        bool  is_write    = false;
        bool  has_barrier = false;
        bool  is_created  = false;
        bool  is_deleted  = false;
        image::ptr                     preview;
        FrameGraph::ResourceAllocInfo* alloc         = nullptr;
        Events::prop_helper*           debug_handler = nullptr;
    };

    struct ResourceTrack
    {
        std::string               name;
        std::vector<ResourceCell> cells;
    };

    FrameGraph::Graph&         graph;
    std::vector<PassInfo>      m_passes;
    std::vector<ResourceTrack> m_resources;
    UINT                       m_max_call_id = 0;
    bool                       m_ready       = false;
    bool                       m_compiled    = false;

    // -----------------------------------------------------------------------
    //  View state
    // -----------------------------------------------------------------------
    vec2  m_offset = { 0.0f, 0.0f };
    float m_zoom   = 1.0f;

    // Selection state — updated on thumbnail click.
    UINT m_sel_call_id = std::numeric_limits<UINT>::max();
    int  m_sel_ri      = -1;

    // -----------------------------------------------------------------------
    //  scroll_container with no scrollbars and no built-in drag.
    // -----------------------------------------------------------------------
    struct timeline_scroll : scroll_container
    {
        using ptr = std::shared_ptr<timeline_scroll>;
        timeline_scroll()
        {
            vert->visible        = false;
            hor->visible         = false;
            over_filled->visible = false;
            allow_overflow       = true;
            clickable            = false;
            contents->width_size  = GUI::size_type::FIXED;
            contents->height_size = GUI::size_type::FIXED;
        }
    };

    // -----------------------------------------------------------------------
    //  Inner canvas drawn inside m_grid_scroll.
    //  Added as the FIRST child of m_grid_scroll so that labels and preview
    //  images (added later in rebuild) render on top.
    // -----------------------------------------------------------------------
    struct grid_canvas : GUI::base
    {
        using ptr = std::shared_ptr<grid_canvas>;
        FrameGraphTimelineCanvas& owner;

        explicit grid_canvas(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking        = GUI::dock::NONE;
            width_size     = GUI::size_type::FIXED;
            height_size    = GUI::size_type::FIXED;
            clip_to_parent = GUI::ParentClip::ALL;
            clickable      = false;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) return;
            owner.draw_grid_content(c, *this);
        }
    };

    // -----------------------------------------------------------------------
    //  Center fill pane — draws lane/row backgrounds and owns pan/zoom input.
    //  Contains m_grid_scroll (FILL) and m_left_scroll (LEFT).
    // -----------------------------------------------------------------------
    struct center_pane : GUI::base
    {
        using ptr = std::shared_ptr<center_pane>;
        FrameGraphTimelineCanvas& owner;

        bool m_dragging      = false;
        vec2 m_drag_start;
        vec2 m_offset_at_drag;

        explicit center_pane(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking     = GUI::dock::FILL;
            width_size  = GUI::size_type::NONE;
            height_size = GUI::size_type::NONE;
            clickable   = true;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) { base::draw(c); return; }

            const rect  b  = get_render_bounds();
            const float bx = b.x, by = b.y, bw = b.w, bh = b.h;
            const float oy = by + owner.m_offset.y;

            owner.dr(c, C_DIRECT_LANE,  bx, oy,                       bw, LANE_H);
            owner.dr(c, C_COMPUTE_LANE, bx, oy + LANE_H,              bw, LANE_H);
            owner.dr(c, C_SEPARATOR,    bx, oy + LANE_COUNT * LANE_H, bw, 2.0f);

            for (int ri = 0; ri < (int)owner.m_resources.size(); ri++)
            {
                float ry = oy + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
                if (ry + ROW_H < by || ry > by + bh) continue;
                owner.dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN, bx + LABEL_W, ry, bw - LABEL_W, ROW_H);
            }

            owner.dr(c, C_LABEL_BG, bx, oy,          LABEL_W, LANE_H);
            owner.dr(c, C_LABEL_BG, bx, oy + LANE_H, LABEL_W, LANE_H);
            for (int ri = 0; ri < (int)owner.m_resources.size(); ri++)
            {
                float ry = oy + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
                if (ry + ROW_H < by || ry > by + bh) continue;
                owner.dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN, bx, ry, LABEL_W, ROW_H);
            }
            owner.dr(c, C_SEPARATOR, bx + LABEL_W - 1.0f, by, 1.0f, bh);

            if (owner.m_sel_ri >= 0 && owner.m_sel_ri < (int)owner.m_resources.size())
            {
                float ry = oy + LANE_COUNT * LANE_H + 6.0f + owner.m_sel_ri * ROW_H;
                owner.dr(c, C_SEL_RESOURCE, bx, ry, bw, ROW_H);
            }

            base::draw(c);
        }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::RIGHT)
            {
                pressed = (action == mouse_action::DOWN);
                if (pressed)
                {
                    m_drag_start     = pos;
                    m_offset_at_drag = owner.m_offset;
                    m_dragging       = false;
                }
                else
                {
                    m_dragging = false;
                    set_movable(false);
                }
            }
            return false;
        }

        virtual bool on_mouse_move(vec2 pos) override
        {
            if (pressed && !m_dragging && (pos - m_drag_start).length() > 3.0f)
            {
                m_dragging = true;
                set_movable(true);
            }
            if (m_dragging)
            {
                owner.m_offset = m_offset_at_drag + (pos - m_drag_start);
                owner.apply_pan();
            }
            return m_dragging;
        }

        virtual bool on_wheel(mouse_wheel type, float value, vec2 wheel_pos) override
        {
            float prev_zoom   = owner.m_zoom;
            owner.m_zoom     *= 1.0f + value / 10.0f;
            owner.m_zoom      = Math::clamp(owner.m_zoom, 0.05f, 20.0f);
            float cx          = get_render_bounds().x;
            float pivot       = wheel_pos.x - cx - LABEL_W - owner.m_offset.x;
            owner.m_offset.x  = wheel_pos.x - cx - LABEL_W - pivot * (owner.m_zoom / prev_zoom);
            owner.apply_zoom();
            owner.apply_pan();
            return true;
        }
    };

    // -----------------------------------------------------------------------
    //  Image widget with an on_click callback.
    // -----------------------------------------------------------------------
    struct preview_image : image
    {
        using ptr = std::shared_ptr<preview_image>;
        std::function<void()> on_click;

        preview_image() { clickable = true; }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::LEFT && action == mouse_action::DOWN && on_click)
            {
                on_click();
                return true;
            }
            return image::on_mouse_action(action, button, pos);
        }
    };

    // -----------------------------------------------------------------------
    //  Content widget placed inside a GUI::Elements::window.
    //  Handles pan/zoom (right-drag + scroll) and drives the GPU preview.
    // -----------------------------------------------------------------------
    struct ResourcePreviewContent : GUI::base
    {
        using ptr = std::shared_ptr<ResourcePreviewContent>;

        image::ptr                                                 m_img;
        GUI::Elements::tree<member_item, object_tree_creator>::ptr m_buffer_tree;
        label::ptr                                                 m_type_lbl;
        label::ptr                                                 m_dim_lbl;
        FrameGraph::ResourceAllocInfo*                             m_alloc   = nullptr;
        UINT                                                       m_call_id = 0;
        Events::prop_helper*                                       m_handler = nullptr;

        // Texture produced on the render thread; handed to m_img via run_on_ui.
        std::shared_ptr<Texture> m_current_tex;

        // Render-thread-only flags (one render thread per call_id, no atomic needed).
        bool m_fit_done      = false;
        bool m_buffer_inited = false;

        // Cached image widget size: UI thread writes, render thread reads.
        // Plain int is naturally atomic on x86 for word-sized stores/loads.
        volatile int m_view_w = 64;
        volatile int m_view_h = 64;

        // Pan/zoom/camera state — written from UI thread, read from render thread.
        // All access must hold m_state_mutex.
        mutable Thread::Lockable::mutex m_state_mutex;
        float2              m_pan   = { 0.0f, 0.0f };
        float               m_scale = 1.0f;
        third_person_camera m_cam_3d;

        bool  m_dragging   = false;
        vec2  m_drag_start;
        vec2  m_prev;

        ResourcePreviewContent(FrameGraph::ResourceAllocInfo* alloc, UINT call_id,
                               const std::string& pass_name)
            : m_alloc(alloc), m_call_id(call_id)
        {
            docking     = GUI::dock::FILL;
            width_size  = GUI::size_type::MATCH_PARENT;
            height_size = GUI::size_type::MATCH_PARENT;
            clickable   = true;

            // Docker layout: preview fills the main area, info panel at the bottom.
            // clickable=false lets mouse events fall through to ResourcePreviewContent.
            auto docker       = std::make_shared<GUI::Elements::dock_base>();
            docker->docking   = GUI::dock::FILL;
            docker->clickable = false;
            add_child(docker);

            m_img              = std::make_shared<image>();
            m_img->docking     = GUI::dock::FILL;
            m_img->width_size  = GUI::size_type::NONE;
            m_img->height_size = GUI::size_type::NONE;
            m_img->clickable   = false;
            docker->add_child(m_img);

            m_buffer_tree          = std::make_shared<GUI::Elements::tree<member_item, object_tree_creator>>();
            m_buffer_tree->docking = GUI::dock::FILL;
            m_buffer_tree->visible = false;
            docker->add_child(m_buffer_tree);

            // Bottom dock — resource / pass information.
            auto bottom_dock  = docker->get_dock(GUI::dock::BOTTOM);
            bottom_dock->size = { 0.0f, 90.0f };

            auto info_panel     = std::make_shared<GUI::base>();
            info_panel->docking = GUI::dock::FILL;

            float info_y = 4.0f;
            auto add_row = [&](const std::string& text) -> label::ptr
            {
                auto lbl         = std::make_shared<label>();
                lbl->text        = text;
                lbl->font_size   = 10.0f;
                lbl->docking     = GUI::dock::NONE;
                lbl->width_size  = GUI::size_type::MATCH_PARENT;
                lbl->height_size = GUI::size_type::FIXED;
                lbl->size        = { 0.0f, 18.0f };
                lbl->pos         = { 6.0f, info_y };
                info_panel->add_child(lbl);
                info_y += 18.0f;
                return lbl;
            };

            add_row("Resource: " + (alloc ? alloc->name : "?"));
            add_row("Pass:     " + pass_name + "  (#" + std::to_string(call_id) + ")");
            m_type_lbl = add_row("Type:     ...");
            m_dim_lbl  = add_row("Size:     ...");

            bottom_dock->get_tabs()->add_page("Info", info_panel);

            m_handler = alloc->process_debug_resource.register_handler(this,
                [this](FrameGraph::Pass* pass, FrameGraph::FrameContext* context)
                {
                    if (pass->call_id != m_call_id) return;

                    if (m_alloc->resource->get_desc().is_texture())
                    {
                        // Read view size cached by update_layout on the UI thread.
                        uint2 SZ = { (UINT)m_view_w, (UINT)m_view_h };

                        // Create / resize render target — render-thread-owned via m_current_tex.
                        bool need_new = !m_current_tex ||
                            uint2(m_current_tex->get_desc().as_texture().Dimensions.xy) != SZ;
                        if (need_new)
                        {
                            HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(
                                HAL::Format::R8G8B8A8_UNORM, { SZ }, 1, 1,
                                HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
                            m_current_tex = std::make_shared<Texture>(desc);

                            if (!m_fit_done)
                            {
                                m_fit_done = true;
                                auto   d   = m_alloc->resource->get_desc().as_texture().Dimensions;
                                float2 src = { float(d.x), float(d.y) };
                                float2 dst = float2(SZ);
                                float  fit = std::min(dst.x / src.x, dst.y / src.y);
                                {
                                    Thread::Lockable::guard lk(m_state_mutex);
                                    m_scale = fit;
                                    m_pan   = (dst - src * fit) * 0.5f;
                                }

                                bool is_3d    = (d.z > 1);
                                std::string t = is_3d ? "Texture3D" : "Texture2D";
                                std::string s = std::to_string(d.x) + "x" + std::to_string(d.y)
                                              + (is_3d ? "x" + std::to_string(d.z) : "");
                                run_on_ui([this, t, s]()
                                {
                                    m_type_lbl->text = "Type:     " + t;
                                    m_dim_lbl->text  = "Size:     " + s;
                                });
                            }

                            // Hand the new texture to the image widget on the UI thread.
                            auto tex = m_current_tex;
                            run_on_ui([this, tex]()
                            {
                                m_img->texture.texture = tex;
                                m_img->visible         = true;
                                m_buffer_tree->visible = false;
                            });
                        }

                        if (!m_current_tex) return;

                        auto& compute = context->get_list()->get_compute();
                        {
                            Slots::FrameGraph_Debug_Common common;
                            common.GetTarget()     = m_current_tex->texture_2d().rwTexture2D;
                            common.GetTargetSize() = SZ;
                            compute.set(common);
                        }

                        if (auto* src = dynamic_cast<HAL::Texture2DView*>(m_alloc->view.get()))
                        {
                            float2 snap_pan;
                            float  snap_scale;
                            {
                                Thread::Lockable::guard lk(m_state_mutex);
                                snap_pan   = m_pan;
                                snap_scale = m_scale;
                            }
                            compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
                            Slots::FrameGraph_Debug_Texture2D tex2d;
                            tex2d.GetSource()     = *src;
                            tex2d.GetSourceSize() = m_alloc->resource->get_desc().as_texture().Dimensions.xy;
                            tex2d.GetOffset()     = snap_pan;
                            tex2d.GetScale()      = snap_scale;
                            compute.set(tex2d);
                        }
                        else if (auto* src = dynamic_cast<HAL::Texture3DView*>(m_alloc->view.get()))
                        {
                            auto snap_cam_cb = [&]() {
                                Thread::Lockable::guard lk(m_state_mutex);
                                m_cam_3d.set_projection_params(Math::pi / 4,
                                    float(SZ.x) / float(SZ.y), 1.0f, 1500.0f);
                                m_cam_3d.frame_move(0.1f);
                                m_cam_3d.update();
                                return m_cam_3d.camera_cb.current;
                            }();
                            compute.set_pipeline<PSOS::FrameGraph_Debug_Texture3D>();
                            Slots::FrameGraph_Debug_Texture3D tex3d;
                            tex3d.GetSource()     = *src;
                            tex3d.GetSourceSize() = m_alloc->resource->get_desc().as_texture().Dimensions;
                            tex3d.GetCamera()     = snap_cam_cb;
                            compute.set(tex3d);
                        }
                        else
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_NotImplemented>();
                        }
                        compute.dispatch(uint3(SZ, 1));
                    }
                    else
                    {
                        if (auto* src = dynamic_cast<HAL::StructuredBufferViewBase*>(m_alloc->view.get()))
                        {
                            if (!m_buffer_inited)
                            {
                                m_buffer_inited      = true;
                                size_t count     = src->get_count();
                                size_t elem_size = src->get_element_size();
                                context->get_list()->get_copy().read_buffer(
                                    src->resource.get(), 0, count * elem_size,
                                    [this, src, count, elem_size](std::span<std::byte> memory)
                                    {
                                        auto tree_data = src->describe(memory.data(), memory.size());
                                        run_on_ui([this, tree_data, count, elem_size]()
                                        {
                                            m_type_lbl->text = "Type:     Buffer";
                                            m_dim_lbl->text  = "Size:     "
                                                + std::to_string(count) + " x "
                                                + std::to_string(elem_size) + "B";
                                            m_buffer_tree->contents->remove_all();
                                            m_buffer_tree->init(tree_data.get());
                                        });
                                    });
                                run_on_ui([this]()
                                {
                                    m_img->visible         = false;
                                    m_buffer_tree->visible = true;
                                });
                            }
                        }
                    }
                });
        }

        ~ResourcePreviewContent()
        {
            if (m_handler) m_handler->unregister();
        }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::RIGHT)
            {
                pressed      = (action == mouse_action::DOWN);
                m_drag_start = pos;
                if (!pressed) { m_dragging = false; set_movable(false); }
            }
            return false;
        }

        virtual bool on_mouse_move(vec2 pos) override
        {
            if (pressed && !m_dragging && (pos - m_drag_start).length() > 3.0f)
            {
                m_dragging = true;
                m_prev     = pos;
                set_movable(true);
            }
            if (m_dragging)
            {
                vec2 delta = pos - m_prev;
                {
                    Thread::Lockable::guard lk(m_state_mutex);
                    m_pan    += float2(delta);
                    m_cam_3d.input(float2(delta) / 1000.0f);
                }
                m_prev = pos;
            }
            return m_dragging;
        }

        virtual bool on_wheel(mouse_wheel type, float value, vec2 wpos) override
        {
            vec2 pivot = wpos - m_img->get_render_bounds().pos;
            {
                Thread::Lockable::guard lk(m_state_mutex);
                float prev = m_scale;
                m_cam_3d.input(value / 100.0f);
                m_scale *= 1.0f + value / 10.0f;
                m_scale  = Math::clamp(m_scale, 0.01f, 50.0f);
                m_pan    = m_scale / prev * (m_pan - float2(pivot)) + float2(pivot);
            }
            return true;
        }

        virtual ::sizer update_layout(::sizer r, float scale) override
        {
            ::sizer result = base::update_layout(r, scale);
            const rect rb = m_img->get_render_bounds();
            m_view_w = std::max(64, (int)rb.w);
            m_view_h = std::max(64, (int)rb.h);
            return result;
        }
    };

    // -----------------------------------------------------------------------
    //  Widget containers
    // -----------------------------------------------------------------------
    center_pane::ptr     m_center;
    timeline_scroll::ptr m_grid_scroll;
    timeline_scroll::ptr m_left_scroll;
    grid_canvas::ptr     m_grid_canvas;
    GUI::base::ptr       m_preview_panel;

    // -----------------------------------------------------------------------
    //  Labels / images
    // -----------------------------------------------------------------------
    label::ptr              m_lbl_direct;
    label::ptr              m_lbl_compute;
    std::vector<label::ptr> m_resource_labels;
    std::vector<image::ptr> m_resource_icons;   // one per resource row, parallel to m_resource_labels

    struct PassLabel
    {
        label::ptr           lbl;
        UINT                 call_id;
        HAL::CommandListType queue;
    };
    std::vector<PassLabel> m_pass_labels;

    // -----------------------------------------------------------------------
    //  Palette
    // -----------------------------------------------------------------------
    static const float4 C_DIRECT_LANE;
    static const float4 C_COMPUTE_LANE;
    static const float4 C_PASS_DIRECT;
    static const float4 C_PASS_COMPUTE;
    static const float4 C_WRITE;
    static const float4 C_READ;
    static const float4 C_BARRIER;
    static const float4 C_CREATED;
    static const float4 C_DELETED;
    static const float4 C_FENCE;
    static const float4 C_FENCE_LINK;
    static const float4 C_LABEL_BG;
    static const float4 C_ROW_EVEN;
    static const float4 C_ROW_ODD;
    static const float4 C_GRID;
    static const float4 C_SEPARATOR;
    static const float4 C_LIFETIME;
    static const float4 C_TEXT_BRIGHT;
    static const float4 C_TEXT_DIM;
    static const float4 C_SEL_PASS;
    static const float4 C_SEL_RESOURCE;

    // -----------------------------------------------------------------------
    //  Helpers
    // -----------------------------------------------------------------------
    static label::ptr make_label(float w, float h, const std::string& text,
                                 float4 color, float font_size)
    {
        auto lbl         = std::make_shared<label>();
        lbl->text        = text;
        lbl->color       = color;
        lbl->font_size   = font_size;
        lbl->docking     = GUI::dock::NONE;
        lbl->width_size  = GUI::size_type::FIXED;
        lbl->height_size = GUI::size_type::FIXED;
        lbl->size        = { w, h };
        lbl->clip_to_parent = GUI::ParentClip::ALL;
        return lbl;
    }

    static std::string to_str(std::wstring_view ws)
    {
        std::string s;
        s.reserve(ws.size());
        for (wchar_t c : ws) s.push_back(static_cast<char>(c));
        return s;
    }

public:
    using ptr = std::shared_ptr<FrameGraphTimelineCanvas>;

    explicit FrameGraphTimelineCanvas(FrameGraph::Graph& g) : graph(g)
    {
        width_size  = GUI::size_type::MATCH_PARENT;
        height_size = GUI::size_type::MATCH_PARENT;

        // center_pane is the fill child of this dock_base; it owns draw + input.
        m_center = std::make_shared<center_pane>(*this);
        add_child(m_center);

        // Grid scroll fills the center pane.
        m_grid_scroll              = std::make_shared<timeline_scroll>();
        m_grid_scroll->docking     = GUI::dock::FILL;
        m_grid_scroll->width_size  = GUI::size_type::NONE;
        m_grid_scroll->height_size = GUI::size_type::NONE;
        m_center->add_child(m_grid_scroll);

        // Grid canvas — first child so labels/images render on top.
        m_grid_canvas = std::make_shared<grid_canvas>(*this);
        m_grid_scroll->add_child(m_grid_canvas);

        // Left-column scroll — fixed width, docked left inside center.
        m_left_scroll              = std::make_shared<timeline_scroll>();
        m_left_scroll->docking     = GUI::dock::LEFT;
        m_left_scroll->width_size  = GUI::size_type::FIXED;
        m_left_scroll->height_size = GUI::size_type::NONE;
        m_left_scroll->size        = { LABEL_W, 0.0f };
        m_center->add_child(m_left_scroll);

        // Lane labels at fixed positions in left-column contents.
        m_lbl_direct  = make_label(LABEL_W - 8.0f, LANE_H - 6.0f, "DIRECT",  C_TEXT_BRIGHT, LABEL_FONT);
        m_lbl_compute = make_label(LABEL_W - 8.0f, LANE_H - 6.0f, "COMPUTE", C_TEXT_BRIGHT, LABEL_FONT);
        m_lbl_direct->pos  = { 4.0f, (LANE_H - 14.0f) * 0.5f };
        m_lbl_compute->pos = { 4.0f, LANE_H + (LANE_H - 14.0f) * 0.5f };
        m_left_scroll->add_child(m_lbl_direct);
        m_left_scroll->add_child(m_lbl_compute);

        // Right dock on this (dock_base) — permanent preview panel.
        auto right_dock  = get_dock(GUI::dock::RIGHT);
        right_dock->size = { 400.0f, 0.0f };

        m_preview_panel              = std::make_shared<GUI::base>();
        m_preview_panel->docking     = GUI::dock::FILL;
        m_preview_panel->width_size  = GUI::size_type::MATCH_PARENT;
        m_preview_panel->height_size = GUI::size_type::MATCH_PARENT;
        right_dock->get_tabs()->add_page("Preview", m_preview_panel);

        graph.on_compile.register_handler(this,
            [this](const FrameGraph::Graph& cg) { rebuild(cg); });
    }

private:
    // -----------------------------------------------------------------------
    //  Rebuild from compiled graph
    // -----------------------------------------------------------------------
    void rebuild(const FrameGraph::Graph& g)
    {
        if (m_compiled) return;
        m_compiled = true;

        for (auto& tr : m_resources)
            for (auto& cell : tr.cells)
            {
                if (cell.preview)       m_grid_scroll->remove_child(cell.preview);
                if (cell.debug_handler) cell.debug_handler->unregister();
            }

        m_passes.clear();
        m_resources.clear();
        m_max_call_id = 0;

        for (auto* pass : g.builder.enabled_passes)
        {
            PassInfo info;
            info.name      = pass->name;
            info.call_id   = pass->call_id;
            info.queue     = pass->get_type();
            info.put_fence = pass->put_fence;
            info.wait_pass = pass->wait_pass;
            m_passes.push_back(info);
            if (pass->call_id > m_max_call_id)
                m_max_call_id = pass->call_id;
        }

        std::map<std::string, ResourceTrack> track_map;
        for (auto* pass : g.builder.enabled_passes)
        {
            for (auto& [alloc, flags] : pass->used.resource_flags)
            {
                auto& tr = track_map[alloc->name];
                tr.name  = alloc->name;

                ResourceCell cell;
                cell.call_id    = pass->call_id;
                cell.is_write   = check(flags & FrameGraph::WRITEABLE_FLAGS);
                cell.is_created = (pass->used.resource_creations.count(alloc) > 0);
                cell.is_deleted = (pass->used.resource_deletions_after.count(alloc) > 0);
                cell.alloc      = alloc;
                tr.cells.push_back(cell);
            }

            for (auto* alloc : pass->used.resource_deletions_before)
            {
                if (pass->used.resource_flags.count(alloc)) continue;

                auto& tr = track_map[alloc->name];
                tr.name  = alloc->name;

                ResourceCell cell;
                cell.call_id    = pass->call_id;
                cell.is_deleted = true;
                tr.cells.push_back(cell);
            }
        }
        for (auto& [name, tr] : track_map)
        {
            for (size_t i = 1; i < tr.cells.size(); ++i)
                if (tr.cells[i].is_write != tr.cells[i - 1].is_write)
                    tr.cells[i].has_barrier = true;
            m_resources.push_back(std::move(tr));
        }

        // Resource-name labels + type icons in left column.
        for (auto& lbl  : m_resource_labels) m_left_scroll->remove_child(lbl);
        for (auto& icon : m_resource_icons)  m_left_scroll->remove_child(icon);
        m_resource_labels.clear();
        m_resource_icons.clear();

        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float row_top = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;

            // Small resource-type indicator icon.
            auto icon            = std::make_shared<image>();
            icon->docking        = GUI::dock::NONE;
            icon->width_size     = GUI::size_type::FIXED;
            icon->height_size    = GUI::size_type::FIXED;
            icon->size           = { ICON_W, ICON_W };
            icon->pos            = { 4.0f, row_top + (ROW_H - ICON_W) * 0.5f };
            icon->clip_to_parent = GUI::ParentClip::ALL;
            icon->clickable      = false;
			icon->texture.texture = Skin::get().DefaultEditBox.Normal.texture;

            // TODO: set icon->texture.texture to an appropriate loaded texture, e.g.:
            //   Texture2D  -> load_icon("path/to/texture2d.png")
            //   Texture3D  -> load_icon("path/to/texture3d.png")
            //   Buffer     -> load_icon("path/to/buffer.png")
            // Icons are assigned later via run_on_ui once the resource type is known.
            m_left_scroll->add_child(icon);
            m_resource_icons.push_back(icon);

            // Resource name label — indented past the icon.
            float ly  = row_top + (ROW_H - 14.0f) * 0.5f;
            float lx  = 4.0f + ICON_W + 4.0f;
            auto lbl  = make_label(LABEL_W - lx - 4.0f, ROW_H - 4.0f,
                                   m_resources[ri].name, C_TEXT_BRIGHT, LABEL_FONT);
            lbl->pos  = { lx, ly };
            m_left_scroll->add_child(lbl);
            m_resource_labels.push_back(lbl);
        }

        // Write-cell preview images (added to grid scroll AFTER grid_canvas).
        auto* passes_ptr = &m_passes;
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float cy = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f;
            for (auto& cell : m_resources[ri].cells)
            {
                if (!cell.is_write) continue;
                auto img         = std::make_shared<preview_image>();
                img->docking     = GUI::dock::NONE;
                img->width_size  = GUI::size_type::FIXED;
                img->height_size = GUI::size_type::FIXED;
                img->size        = { 0.0f, CELL_H };
                img->pos         = { 0.0f, cy };

                img->on_click = [this,
                                  ri        = ri,
                                  cid       = cell.call_id,
                                  alloc     = cell.alloc,
                                  passes_ptr]()
                {
                    m_sel_call_id = cid;
                    m_sel_ri      = ri;

                    std::string pass_name_str;
                    for (auto& p : *passes_ptr)
                        if (p.call_id == cid) { pass_name_str = to_str(p.name); break; }

                    m_preview_panel->remove_all();
                    m_preview_panel->add_child(
                        std::make_shared<ResourcePreviewContent>(alloc, cid, pass_name_str));
                };

                m_grid_scroll->add_child(img);
                cell.preview = img;
            }
        }

        // process_debug_resource handlers.
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
            for (auto& cell : m_resources[ri].cells)
            {
                if (!cell.preview || !cell.alloc) continue;
                auto  img             = cell.preview;
                UINT  capture_call_id = cell.call_id;
                auto* info            = cell.alloc;
                auto  icon_img        = m_resource_icons[ri];

                // Per-cell camera (not shared) and render-thread-local texture holder.
                auto cam       = std::make_shared<third_person_camera>();
                auto thumb_tex = std::make_shared<std::shared_ptr<Texture>>();
                cell.debug_handler = info->process_debug_resource.register_handler(this,
                    [this, img, capture_call_id, info, cam, thumb_tex, icon_img,
                     icon_done = false]
                    (FrameGraph::Pass* pass, FrameGraph::FrameContext* context) mutable
                    {
                        if (pass->call_id != capture_call_id) return;

                        // Detect and display the resource type icon once per handler instance.
                        // Multiple cells for the same resource row each have their own bool,
                        // so the icon may be assigned more than once — that is idempotent.
                        if (!icon_done)
                        {
                            icon_done         = true;
                            const bool is_tex = info->resource->get_desc().is_texture();
                            const bool is_3d  = is_tex &&
                                info->resource->get_desc().as_texture().Dimensions.z > 1;
                            run_on_ui([icon_img, is_tex, is_3d]()
                            {
                                // TODO: assign icon_img->texture.texture to the appropriate icon:
                                // if      (is_3d)  icon_img->texture.texture = /* load("path/to/texture3d_icon.png") */;
                                // else if (is_tex) icon_img->texture.texture = /* load("path/to/texture2d_icon.png") */;
                                // else             icon_img->texture.texture = /* load("path/to/buffer_icon.png") */;
                                (void)icon_img; (void)is_tex; (void)is_3d;
                            });
                        }

                        if (!info->resource->get_desc().is_texture()) return;

                        const uint2 PREVIEW_SIZE = { 128, 64 };

                        if (!*thumb_tex)
                        {
                            HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(
                                HAL::Format::R8G8B8A8_UNORM, { PREVIEW_SIZE }, 1, 1,
                                HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
                            *thumb_tex = std::make_shared<Texture>(desc);
                            auto t = *thumb_tex;
                            run_on_ui([img, t]() { img->texture.texture = t; });
                        }

                        auto& compute = context->get_list()->get_compute();
                        {
                            Slots::FrameGraph_Debug_Common common;
                            common.GetTarget()     = (*thumb_tex)->texture_2d().rwTexture2D;
                            common.GetTargetSize() = PREVIEW_SIZE;
                            compute.set(common);
                        }

                        if (auto* source = dynamic_cast<HAL::Texture2DView*>(info->view.get()))
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
                            Slots::FrameGraph_Debug_Texture2D tex2d;
                            uint2  src_dim   = info->resource->get_desc().as_texture().Dimensions.xy;
                            float2 src_size  = float2(src_dim);
                            float2 dst_size  = float2(PREVIEW_SIZE);
                            float  fit_scale = std::min(dst_size.x / src_size.x, dst_size.y / src_size.y);
                            float2 offset    = (dst_size - src_size * fit_scale) * 0.5f;
                            tex2d.GetSource()     = *source;
                            tex2d.GetSourceSize() = src_dim;
                            tex2d.GetOffset()     = offset;
                            tex2d.GetScale()      = fit_scale;
                            compute.set(tex2d);
                        }
                        else if (auto* source = dynamic_cast<HAL::Texture3DView*>(info->view.get()))
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_Texture3D>();
                            cam->set_projection_params(Math::pi / 4,
                                float(PREVIEW_SIZE.x) / float(PREVIEW_SIZE.y), 1.0f, 1500.0f);
                            cam->frame_move(0.1f);
                            cam->update();
                            Slots::FrameGraph_Debug_Texture3D tex3d;
                            tex3d.GetSource()     = *source;
                            tex3d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions;
                            tex3d.GetCamera()     = cam->camera_cb.current;
                            compute.set(tex3d);
                        }
                        else
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_NotImplemented>();
                        }
                        compute.dispatch(uint3(PREVIEW_SIZE, 1));
                    });
            }

        // Pass-name labels in grid scroll (added last, render on top of images).
        for (auto& pl : m_pass_labels) m_grid_scroll->remove_child(pl.lbl);
        m_pass_labels.clear();

        for (auto& pass : m_passes)
        {
            float py = (pass.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f) + PAD + 2.0f;
            auto lbl = make_label(0.0f, PASS_H - 2.0f * PAD - 4.0f,
                                  to_str(pass.name), C_TEXT_BRIGHT, LABEL_FONT);
            lbl->pos = { 0.0f, py };
            m_grid_scroll->add_child(lbl);
            m_pass_labels.push_back({ lbl, pass.call_id, pass.queue });
        }

        m_ready = true;
        apply_zoom();
        apply_pan();
    }

    // -----------------------------------------------------------------------
    //  apply_pan — move scroll container offsets, no child iteration.
    // -----------------------------------------------------------------------
    void apply_pan()
    {
        m_left_scroll->contents->pos = { 0.0f, m_offset.y };
        m_grid_scroll->contents->pos = m_offset;
    }

    // -----------------------------------------------------------------------
    //  apply_zoom — update content sizes and X-positions of column children.
    // -----------------------------------------------------------------------
    void apply_zoom()
    {
        float cw        = col_w();
        float content_h = LANE_COUNT * LANE_H + 6.0f + (float)m_resources.size() * ROW_H;
        float content_w = LABEL_W + (m_max_call_id + 2) * cw;

        m_left_scroll->contents->size = { LABEL_W, content_h };
        m_grid_scroll->contents->size = { content_w, content_h };
        m_grid_canvas->size           = { content_w, content_h };

        float block_w = cw - 2.0f * PAD - 4.0f;
        float block_h = PASS_H - 2.0f * PAD - 4.0f;
        float cell_cw = cw - 2.0f * PAD;

        for (auto& pl : m_pass_labels)
        {
            float py = (pl.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f) + PAD + 2.0f;
            pl.lbl->pos  = { LABEL_W + pl.call_id * cw + PAD + 2.0f, py };
            pl.lbl->size = { block_w, block_h };
        }

        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float cy = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f;
            for (auto& cell : m_resources[ri].cells)
            {
                if (!cell.preview) continue;
                cell.preview->pos  = { LABEL_W + cell.call_id * cw + PAD, cy };
                cell.preview->size = { cell_cw, CELL_H };
            }
        }
    }

    // -----------------------------------------------------------------------
    //  Coordinate helpers — no m_offset; bx/by from grid_canvas::get_render_bounds()
    //  already include the scroll displacement.
    // -----------------------------------------------------------------------
    float col_w() const { return COL_W_BASE * m_zoom; }

    float dep_x(UINT call_id, float bx) const
    {
        return bx + LABEL_W + call_id * col_w();
    }

    float lane_y(HAL::CommandListType queue, float by) const
    {
        return (queue == HAL::CommandListType::COMPUTE) ? by + LANE_H : by;
    }

    float row_y(int ri, float by) const
    {
        return by + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
    }

    // -----------------------------------------------------------------------
    //  Draw helper
    // -----------------------------------------------------------------------
    inline void dr(Context& c, const float4& col,
                   float x, float y, float w, float h) const
    {
        if (w <= 0.0f || h <= 0.0f) return;
        c.renderer->draw_color(c, col, rect{ x, y, w, h });
    }

    // -----------------------------------------------------------------------
    //  draw_grid_content — called by grid_canvas::draw().
    //  canvas.get_render_bounds() reflects the scroll offset, so dep_x/lane_y/
    //  row_y return correct screen coordinates without manual m_offset.
    // -----------------------------------------------------------------------
    void draw_grid_content(Context& c, GUI::base& canvas)
    {
        const rect  b  = canvas.get_render_bounds();
        const float bx = b.x, by = b.y, bh = b.h;

        // Selected pass column highlight (drawn first, behind everything).
        if (m_sel_call_id != std::numeric_limits<UINT>::max())
            dr(c, C_SEL_PASS, dep_x(m_sel_call_id, bx), by, col_w(), bh);

        // vertical grid lines
        for (UINT cid = 0; cid <= m_max_call_id + 1; cid++)
            dr(c, C_GRID, dep_x(cid, bx), by, 1.0f, bh);

        // pass blocks
        for (auto& pass : m_passes)
        {
            float px = dep_x(pass.call_id, bx);
            float py = lane_y(pass.queue, by);
            const float4& pc = (pass.queue == HAL::CommandListType::DIRECT)
                                 ? C_PASS_DIRECT : C_PASS_COMPUTE;
            dr(c, pc, px + PAD, py + PAD, col_w() - 2.0f * PAD, PASS_H - 2.0f * PAD);

            if (pass.put_fence)
                dr(c, C_FENCE, px + col_w() - PAD - 1.5f, by, 3.0f, LANE_COUNT * LANE_H);
        }

        // cross-queue dependency connectors
        for (auto& pass : m_passes)
        {
            if (!pass.wait_pass) continue;
            float from_cx = dep_x(pass.wait_pass->call_id, bx) + col_w() * 0.5f;
            float to_cx   = dep_x(pass.call_id,            bx) + col_w() * 0.5f;
            float from_cy = lane_y(pass.wait_pass->get_type(), by) + LANE_H * 0.5f;
            float to_cy   = lane_y(pass.queue,                 by) + LANE_H * 0.5f;
            float min_x   = std::min(from_cx, to_cx);
            dr(c, C_FENCE_LINK, min_x,      from_cy - 1.0f, std::abs(to_cx - from_cx), 2.0f);
            float min_y = std::min(from_cy, to_cy);
            dr(c, C_FENCE_LINK, to_cx - 1.0f, min_y, 2.0f, std::abs(to_cy - from_cy));
        }

        // resource tracks
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            auto& tr = m_resources[ri];
            float ry = row_y(ri, by);
            float cy = ry + (ROW_H - CELL_H) * 0.5f;

            // lifetime spine
            {
                constexpr UINT NONE = std::numeric_limits<UINT>::max();
                UINT id_start   = NONE;
                UINT id_deleted = NONE;

                for (auto& cell : tr.cells)
                {
                    const bool del_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                    if (!del_only) {
                        if (id_start == NONE) id_start = cell.call_id;
                        if (cell.is_created && cell.call_id < id_start) id_start = cell.call_id;
                    }
                    if (cell.is_deleted) id_deleted = cell.call_id;
                }

                if (id_start != NONE)
                {
                    float lx  = dep_x(id_start, bx) + col_w() * 0.5f;
                    float rx  = lx;
                    if (id_deleted != NONE && id_deleted > id_start)
                        rx = dep_x(id_deleted, bx) + PAD + 2.5f;
                    float mid = cy + CELL_H * 0.5f;
                    if (rx > lx)
                        dr(c, C_LIFETIME, lx, mid - 1.0f, rx - lx, 2.0f);
                }
            }

            for (auto& cell : tr.cells)
            {
                float cx  = dep_x(cell.call_id, bx);
                float cw  = col_w() - 2.0f * PAD;
                const bool deletion_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                if (!deletion_only)
                    dr(c, cell.is_write ? C_WRITE : C_READ, cx + PAD, cy, cw, CELL_H);
                if (cell.has_barrier)
                    dr(c, C_BARRIER,  cx + PAD - 2.5f, cy - 2.0f, 3.0f, CELL_H + 4.0f);
                if (cell.is_created)
                    dr(c, C_CREATED,  cx + PAD, cy, 5.0f, CELL_H);
                if (cell.is_deleted)
                    dr(c, C_DELETED,  cx + PAD, cy, 5.0f, CELL_H);
            }
        }
    }

};

// ---------------------------------------------------------------------------
//  Colour table
// ---------------------------------------------------------------------------
const float4 FrameGraphTimelineCanvas::C_DIRECT_LANE  = { 0.13f, 0.13f, 0.22f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_COMPUTE_LANE = { 0.09f, 0.16f, 0.16f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_PASS_DIRECT  = { 0.28f, 0.50f, 0.82f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_PASS_COMPUTE = { 0.20f, 0.68f, 0.46f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_WRITE        = { 0.78f, 0.28f, 0.16f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_READ         = { 0.20f, 0.50f, 0.80f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_BARRIER      = { 0.95f, 0.82f, 0.08f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_CREATED      = { 0.18f, 0.84f, 0.18f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_DELETED      = { 1.00f, 1.00f, 1.00f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_FENCE        = { 0.95f, 0.72f, 0.06f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_FENCE_LINK   = { 0.95f, 0.72f, 0.06f, 0.6f };
const float4 FrameGraphTimelineCanvas::C_LABEL_BG     = { 0.07f, 0.07f, 0.09f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_ROW_EVEN     = { 0.12f, 0.12f, 0.14f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_ROW_ODD      = { 0.09f, 0.09f, 0.11f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_GRID         = { 0.24f, 0.24f, 0.30f, 0.6f };
const float4 FrameGraphTimelineCanvas::C_SEPARATOR    = { 0.32f, 0.32f, 0.38f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_LIFETIME     = { 0.55f, 0.55f, 0.60f, 0.6f };
const float4 FrameGraphTimelineCanvas::C_TEXT_BRIGHT  = { 0.90f, 0.90f, 0.92f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_TEXT_DIM     = { 0.55f, 0.55f, 0.58f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_SEL_PASS     = { 1.00f, 0.82f, 0.16f, 0.20f };
const float4 FrameGraphTimelineCanvas::C_SEL_RESOURCE = { 1.00f, 0.82f, 0.16f, 0.12f };

// ---------------------------------------------------------------------------
//  Factory
// ---------------------------------------------------------------------------
GUI::base::ptr FrameGraphDebug::create_timeline_layout(FrameGraph::Graph& graph)
{
    return std::make_shared<FrameGraphTimelineCanvas>(graph);
}
