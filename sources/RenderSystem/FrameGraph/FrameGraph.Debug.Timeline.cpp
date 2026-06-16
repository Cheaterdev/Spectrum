module FrameGraphDebug;
import RenderSystem;

import windows;
import Core;
import HAL;
import GUI;
import FrameGraph;
import Graphics;


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
        bool                 put_fence  = false;
        bool                 disabled   = false;
        bool                 runs_alone = false;
        // Cross-queue deps resolved at rebuild time — no raw pointers.
        std::vector<std::pair<HAL::CommandListType, UINT>> cross_queue_deps;
        // Copied from Pass::debug_commands during rebuild(); barrier_point == nullptr.
        std::vector<HAL::CommandRecord> debug_commands;
    };

    struct ResourceCell
    {
        UINT  call_id     = 0;
        bool  is_write    = false;
        bool  has_barrier = false;
        bool  is_created  = false;
        bool  is_deleted  = false;
        bool  disabled    = false;
        image::ptr                     preview;
        label::ptr                     state_lbl;
        std::shared_ptr<Texture>       thumb_tex;
        FrameGraph::ResourceAllocInfo* alloc         = nullptr;
        Events::prop_helper*           debug_handler = nullptr;
        std::string                    state_str;
    };

    struct ResourceTrack
    {
        std::string               name;
        bool                      is_static      = false;
        bool                      is_passed      = false;
        bool                      is_non_deleted = false;
        bool                      has_mismatch   = false; // barrier before/after continuity error
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
    float m_zoom   = 1.0f;

    // Selection state — updated on thumbnail/pass click.
    UINT m_sel_call_id      = std::numeric_limits<UINT>::max();
    std::vector<UINT> m_sel_wait_call_ids;
    int  m_sel_ri           = -1;

    // -----------------------------------------------------------------------
    //  scroll_container with no scrollbars and no built-in drag.
    // -----------------------------------------------------------------------
    //  Main scroll area — draws lane/row backgrounds, owns pan/zoom input,
    //  and is itself the scroll container that holds all content children.
    // -----------------------------------------------------------------------
    struct content_scroll : scroll_container
    {
        using ptr = std::shared_ptr<content_scroll>;
        FrameGraphTimelineCanvas& owner;

        bool m_dragging      = false;
        vec2 m_drag_start;
        vec2 m_offset_at_drag;

        explicit content_scroll(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking               = GUI::dock::FILL;
            width_size            = GUI::size_type::NONE;
            height_size           = GUI::size_type::NONE;
            clickable             = true;
            vert->visible         = false;
            hor->visible          = false;
            over_filled->visible  = false;
            contents->width_size  = GUI::size_type::FIXED;
            contents->height_size = GUI::size_type::FIXED;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) { scroll_container::draw(c); return; }

            const rect  b  = get_render_bounds();
            const float bx = b.x, by = b.y, bw = b.w, bh = b.h;
            const float oy = by + contents->pos->y;

            // Row backgrounds — content area only (overlay panels cover frozen strips)
            for (int ri = 0; ri < (int)owner.m_resources.size(); ri++)
            {
                float ry = oy + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
                if (ry + ROW_H < by || ry > by + bh) continue;
                owner.dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN, bx + LABEL_W, ry, bw - LABEL_W, ROW_H);
            }

            if (owner.m_sel_ri >= 0 && owner.m_sel_ri < (int)owner.m_resources.size())
            {
                float ry = oy + LANE_COUNT * LANE_H + 6.0f + owner.m_sel_ri * ROW_H;
                owner.dr(c, C_SEL_RESOURCE, bx + LABEL_W, ry, bw - LABEL_W, ROW_H);
            }

            scroll_container::draw(c);
        }

        virtual void resized() override
        {
            scroll_container::resized();
            owner.apply_pan();
        }

        // Scissor rects need correct render_bounds, which base::update_layout sets
        // AFTER calling resized(). Override update_layout so we set scissors once
        // the full layout pass has completed and render_bounds is valid.
        virtual ::sizer update_layout(::sizer r, float scale) override
        {
            ::sizer result = scroll_container::update_layout(r, scale);
            if (!owner.m_top_panel || !owner.m_left_panel) return result;

            const rect fb = filled->get_render_bounds();
            filled->child_scissor = rect{
                fb.x + LABEL_W,
                fb.y + LANE_COUNT * LANE_H,
                fb.w - LABEL_W,
                fb.h - LANE_COUNT * LANE_H
            };
            const rect sb     = get_render_bounds();
            const float vert_w = (vert && vert->visible) ? vert->size->x : 0.0f;
            const float hor_h  = (hor  && hor->visible)  ? hor->size->y  : 0.0f;
            rect top_clip { sb.x + LABEL_W, sb.y,                       sb.w - LABEL_W - vert_w, LANE_COUNT * LANE_H };
            rect left_clip{ sb.x,           sb.y + LANE_COUNT * LANE_H, LABEL_W,                 sb.h - LANE_COUNT * LANE_H - hor_h };
            owner.m_top_panel->self_scissor  = top_clip;
            owner.m_top_panel->child_scissor = top_clip;
            owner.m_left_panel->self_scissor  = left_clip;
            owner.m_left_panel->child_scissor = left_clip;
           /* if (owner.m_minimap)
            {
                owner.m_minimap->pos  = { 0.0f, sb.h - minimap_overlay::H };
                owner.m_minimap->size = { sb.w,  minimap_overlay::H };
            }      */
            return result;
        }

        virtual void moving(vec2 pos) override
        {
            scroll_container::moving(pos);
            owner.apply_pan();
        }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::RIGHT)
            {
                pressed = (action == mouse_action::DOWN);
                if (pressed)
                {
                    m_drag_start     = pos;
                    m_offset_at_drag = contents->pos.get();
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
                vec2 p = m_offset_at_drag + (pos - m_drag_start);
                contents->pos = vec2::min(vec2(0, 0),
                    vec2::max(p, -contents->scaled_size.get() + vec2(filled->get_render_bounds().size)));
                owner.apply_pan();
            }
            return m_dragging;
        }

        virtual bool on_wheel(mouse_wheel type, float value, vec2 wheel_pos) override
        {
            moving({ 0.0f, value * 60.0f });
            return true;
        }

        void add_overlay(GUI::base::ptr obj)
        {
            GUI::base::add_child(obj);
        }
    };

    // -----------------------------------------------------------------------
    //  Inner canvas drawn inside m_scroll.
    //  Added as the FIRST child so that labels and preview images render on top.
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
            clamp_to_parent = GUI::ParentClamp::ALL;
            clickable      = false;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) return;
            owner.draw_grid_content(c, *this);
        }
    };

    // -----------------------------------------------------------------------
    //  Corner overlay — top-left intersection, never scrolls.
    //  Draws lane-colour backgrounds for the label-column area and holds
    //  the DIRECT / COMPUTE queue name labels.
    // -----------------------------------------------------------------------
    struct corner_overlay : GUI::base
    {
        using ptr = std::shared_ptr<corner_overlay>;
        FrameGraphTimelineCanvas& owner;

        explicit corner_overlay(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking        = GUI::dock::NONE;
            width_size     = GUI::size_type::FIXED;
            height_size    = GUI::size_type::FIXED;
            size           = { LABEL_W, LANE_COUNT * LANE_H };
            pos            = { 0.0f, 0.0f };
            clickable      = false;
            clamp_to_parent = GUI::ParentClamp::ALL;
        }

        virtual void draw(Context& c) override
        {
            const rect b = get_render_bounds();
            owner.dr(c, C_DIRECT_LANE,  b.x, b.y,          LABEL_W, LANE_H);
            owner.dr(c, C_COMPUTE_LANE, b.x, b.y + LANE_H, LABEL_W, LANE_H);
            base::draw(c);
        }
    };

    // -----------------------------------------------------------------------
    //  Left-panel overlay — follows Y scroll, pinned at X = 0.
    //  Draws resource-row label-column backgrounds and holds the name labels.
    // -----------------------------------------------------------------------
    struct left_panel_overlay : GUI::base
    {
        using ptr = std::shared_ptr<left_panel_overlay>;
        FrameGraphTimelineCanvas& owner;

        explicit left_panel_overlay(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking        = GUI::dock::NONE;
            width_size     = GUI::size_type::FIXED;
            height_size    = GUI::size_type::FIXED;
            size           = { LABEL_W, 0.0f };
            clickable      = false;
            clamp_to_parent = GUI::ParentClamp::NONE;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) { base::draw(c); return; }
            const rect b  = get_render_bounds();
            const rect sb = owner.m_scroll->get_render_bounds();
            for (int ri = 0; ri < (int)owner.m_resources.size(); ri++)
            {
                float ry = b.y + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
                if (ry + ROW_H < sb.y || ry > sb.y + sb.h) continue;
                owner.dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN, sb.x, ry, LABEL_W, ROW_H);
            }
            owner.dr(c, C_SEPARATOR, sb.x + LABEL_W - 1.0f, sb.y, 1.0f, sb.h);
            base::draw(c);
        }
    };

    // -----------------------------------------------------------------------
    //  Top-panel overlay — follows X scroll, pinned at Y = 0.
    //  Draws lane-colour backgrounds for the content area and holds pass labels.
    // -----------------------------------------------------------------------
    struct top_panel_overlay : GUI::base
    {
        using ptr = std::shared_ptr<top_panel_overlay>;
        FrameGraphTimelineCanvas& owner;

        explicit top_panel_overlay(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking        = GUI::dock::NONE;
            width_size     = GUI::size_type::FIXED;
            height_size    = GUI::size_type::FIXED;
            size           = { 0.0f, LANE_COUNT * LANE_H };
            clickable      = false;
            clamp_to_parent = GUI::ParentClamp::NONE;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) { base::draw(c); return; }
            const rect  sb = owner.m_scroll->get_render_bounds();
            const float cx = sb.x + LABEL_W;
            const float cw = sb.w - LABEL_W;

            owner.dr(c, C_DIRECT_LANE,  cx, sb.y,                              cw, LANE_H);
            owner.dr(c, C_COMPUTE_LANE, cx, sb.y + LANE_H,                     cw, LANE_H);
            owner.dr(c, C_SEPARATOR,    cx, sb.y + LANE_COUNT * LANE_H - 2.0f, cw, 2.0f);

            // Selection highlights — same columns as in the grid, clipped to lane height.
            const float bx = sb.x + owner.m_scroll->contents->pos->x;
            const float lane_h = LANE_COUNT * LANE_H;
            for (UINT wid : owner.m_sel_wait_call_ids)
                owner.dr(c, C_SEL_WAIT_PASS, owner.dep_x(wid, bx), sb.y, owner.col_w(), lane_h);
            if (owner.m_sel_call_id != std::numeric_limits<UINT>::max())
                owner.dr(c, C_SEL_PASS, owner.dep_x(owner.m_sel_call_id, bx), sb.y, owner.col_w(), lane_h);

            // Pass blocks — X follows content scroll, Y is fixed to viewport top.
            for (auto& pass : owner.m_passes)
            {
                float px = owner.dep_x(pass.call_id, bx);
                float py = (pass.queue == HAL::CommandListType::COMPUTE)
                           ? sb.y + LANE_H : sb.y;
                const float4& pc = pass.disabled ? C_PASS_DISABLED
                                   : (pass.queue == HAL::CommandListType::DIRECT)
                                   ? C_PASS_DIRECT : C_PASS_COMPUTE;
                owner.dr(c, pc, px + PAD, py + PAD, owner.col_w() - 2.0f * PAD, PASS_H - 2.0f * PAD);
                if (!pass.disabled)
                {
                    if (pass.runs_alone)
                        owner.dr(c, C_RUNS_ALONE, px + PAD, py + PASS_H - PAD - 3.0f,
                                 owner.col_w() - 2.0f * PAD, 3.0f);
                    if (pass.put_fence)
                        owner.dr(c, C_FENCE, px + owner.col_w() - PAD - 1.5f, py, 3.0f, LANE_H);
                    if (!pass.cross_queue_deps.empty())
                        owner.dr(c, C_FENCE, px + PAD, py, 3.0f, LANE_H);
                }
            }

            // Cross-queue bezier connectors — all passes.
            {
                std::vector<std::pair<float2, float2>> curves;
                for (auto& pass : owner.m_passes)
                {
                    if (pass.disabled || pass.cross_queue_deps.empty()) continue;
                    float to_cx = owner.dep_x(pass.call_id, bx) + PAD;
                    float to_cy = ((pass.queue == HAL::CommandListType::COMPUTE)
                                   ? sb.y + LANE_H : sb.y) + LANE_H * 0.5f;
                    for (auto& [type, dep_id] : pass.cross_queue_deps)
                    {
                        float from_cx = owner.dep_x(dep_id, bx) + owner.col_w() - PAD - 1.5f;
                        float from_cy = ((type == HAL::CommandListType::COMPUTE)
                                         ? sb.y + LANE_H : sb.y) + LANE_H * 0.5f;
                        curves.push_back({ { from_cx, from_cy }, { to_cx, to_cy } });
                    }
                }
                owner.draw_splines(c, curves, C_FENCE_LINK);
            }

            base::draw(c);
        }
    };

    // -----------------------------------------------------------------------
    //  Minimap — scaled-down overview of the full timeline for navigation.
    // -----------------------------------------------------------------------
    struct minimap_overlay : GUI::base
    {
        using ptr = std::shared_ptr<minimap_overlay>;
        FrameGraphTimelineCanvas& owner;

        static constexpr float H = 72.0f;
        bool m_dragging = false;

        explicit minimap_overlay(FrameGraphTimelineCanvas& o) : owner(o)
        {
            docking         = GUI::dock::FILL;
            width_size      = GUI::size_type::FIXED;
            height_size     = GUI::size_type::FIXED;
			x_type = GUI::pos_x_type::RIGHT;
			y_type = GUI::pos_y_type::BOTTOM;
			margin = { 16,16,16,16 };
			size = { 256,256 };
			clickable = true;
			clamp_to_parent = GUI::ParentClamp::NONE;
        }

        void get_scale(const rect& b, float& sx, float& sy) const
        {
            float cw = LABEL_W + (owner.m_max_call_id + 2) * owner.col_w();
            float ch = LANE_COUNT * LANE_H + 6.0f + (float)owner.m_resources.size() * ROW_H;
            sx = (cw > 0.0f) ? b.w / cw : 1.0f;
            sy = (ch > 0.0f) ? b.h / ch : 1.0f;
        }

        virtual void draw(Context& c) override
        {
            if (!owner.m_ready) return;
            const rect b = get_render_bounds();
            float sx, sy;
            get_scale(b, sx, sy);

            owner.dr(c, { 0.04f, 0.04f, 0.06f, 0.90f }, b.x, b.y, b.w, b.h);

            for (auto& pass : owner.m_passes)
            {
                float lane_off = (pass.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f);
                float px = b.x + (LABEL_W + pass.call_id * owner.col_w() + PAD) * sx;
                float pw = std::max(1.0f, (owner.col_w() - 2.0f * PAD) * sx);
                float py = b.y + (lane_off + PAD) * sy;
                float ph = std::max(1.0f, (PASS_H - 2.0f * PAD) * sy);
                const float4& pc = pass.disabled ? C_PASS_DISABLED
                    : (pass.queue == HAL::CommandListType::DIRECT) ? C_PASS_DIRECT : C_PASS_COMPUTE;
                owner.dr(c, pc, px, py, pw, ph);
            }

            for (int ri = 0; ri < (int)owner.m_resources.size(); ri++)
            {
                float cy = b.y + (LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f) * sy;
                float ch = std::max(1.5f, CELL_H * sy);
                for (auto& cell : owner.m_resources[ri].cells)
                {
                    float cx = b.x + (LABEL_W + cell.call_id * owner.col_w() + PAD) * sx;
                    float cw = std::max(1.5f, (owner.col_w() - 2.0f * PAD) * sx);
                    const float4& cc = cell.disabled
                        ? (cell.is_write ? C_WRITE_DISABLED : C_READ_DISABLED)
                        : (cell.is_write ? C_WRITE : C_READ);
                    owner.dr(c, cc, cx, cy, cw, ch);
                }
            }

            // Viewport indicator
            const vec2 cp = owner.m_scroll->contents->pos.get();
            const rect vp = owner.m_scroll->filled->get_render_bounds();
            float vx = b.x + (-cp.x) * sx;
            float vy = b.y + (-cp.y) * sy;
            float vw = std::max(2.0f, vp.w * sx);
            float vh = std::max(2.0f, vp.h * sy);
            owner.dr(c, { 1.0f, 1.0f, 1.0f, 0.10f }, vx,      vy,      vw, vh);
            owner.dr(c, { 1.0f, 1.0f, 1.0f, 0.65f }, vx,      vy,      vw, 1.0f);
            owner.dr(c, { 1.0f, 1.0f, 1.0f, 0.65f }, vx,      vy+vh-1, vw, 1.0f);
            owner.dr(c, { 1.0f, 1.0f, 1.0f, 0.65f }, vx,      vy,      1.0f, vh);
            owner.dr(c, { 1.0f, 1.0f, 1.0f, 0.65f }, vx+vw-1, vy,      1.0f, vh);
        }

        void navigate(vec2 mouse_pos)
        {
            const rect b = get_render_bounds();
            float sx, sy;
            get_scale(b, sx, sy);
            if (sx <= 0.0f || sy <= 0.0f) return;

            const rect vp = owner.m_scroll->filled->get_render_bounds();
            float cx = (mouse_pos.x - b.x) / sx - vp.w * 0.5f;
            float cy = (mouse_pos.y - b.y) / sy - vp.h * 0.5f;

            float cw = LABEL_W + (owner.m_max_call_id + 2) * owner.col_w();
            float ch = LANE_COUNT * LANE_H + 6.0f + (float)owner.m_resources.size() * ROW_H;
            owner.m_scroll->contents->pos = vec2(
                Math::clamp(-cx, std::min(0.0f, -(cw - vp.w)), 0.0f),
                Math::clamp(-cy, std::min(0.0f, -(ch - vp.h)), 0.0f)
            );
            owner.apply_pan();
        }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::LEFT)
            {
                m_dragging = (action == mouse_action::DOWN);
                if (m_dragging) navigate(pos);
                else m_dragging = false;
            }
            return true;
        }

        virtual bool on_mouse_move(vec2 pos) override
        {
            if (m_dragging) navigate(pos);
            return m_dragging;
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
    //  Label widget with an on_click callback.
    // -----------------------------------------------------------------------
    struct clickable_label : label
    {
        using ptr = std::shared_ptr<clickable_label>;
        std::function<void()> on_click;

        clickable_label() { clickable = true; }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::LEFT && action == mouse_action::DOWN && on_click)
            {
                on_click();
                return true;
            }
            return label::on_mouse_action(action, button, pos);
        }
    };

    // -----------------------------------------------------------------------
    //  Transparent clickable overlay for a pass block.
    // -----------------------------------------------------------------------
    struct pass_button : GUI::base
    {
        using ptr = std::shared_ptr<pass_button>;
        std::function<void()> on_click;

        pass_button()
        {
            clickable   = true;
            docking     = GUI::dock::NONE;
            width_size  = GUI::size_type::FIXED;
            height_size = GUI::size_type::FIXED;
        }

        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::LEFT && action == mouse_action::DOWN && on_click)
            {
                on_click();
                return true;
            }
            return base::on_mouse_action(action, button, pos);
        }
    };

    // -----------------------------------------------------------------------
    //  Info panel shown in the preview dock when a pass is clicked.
    // -----------------------------------------------------------------------
    struct PassInfoContent : GUI::base
    {
        using ptr = std::shared_ptr<PassInfoContent>;

        PassInfoContent(const PassInfo& info, const std::vector<PassInfo>& all_passes)
        {
            docking     = GUI::dock::FILL;
            width_size  = GUI::size_type::MATCH_PARENT;
            height_size = GUI::size_type::MATCH_PARENT;

            auto scr          = std::make_shared<scroll_container>();
            scr->docking      = GUI::dock::FILL;
            scr->width_size   = GUI::size_type::MATCH_PARENT;
            scr->height_size  = GUI::size_type::MATCH_PARENT;
            scr->hor->visible = false;
            scr->contents->width_size  = GUI::size_type::MATCH_PARENT;
            scr->contents->height_size = GUI::size_type::FIXED;

            float y = 8.0f;
            const float4 col_bright = { 0.90f, 0.90f, 0.92f, 1.0f };
            const float4 col_dim    = { 0.55f, 0.55f, 0.58f, 1.0f };

            auto add_row = [&](const std::string& text,
                               float4 color = { 0.90f, 0.90f, 0.92f, 1.0f })
            {
                auto lbl         = std::make_shared<label>();
                lbl->text        = text;
                lbl->color       = color;
                lbl->font_size   = 11.0f;
                lbl->docking     = GUI::dock::NONE;
                lbl->width_size  = GUI::size_type::MATCH_PARENT;
                lbl->height_size = GUI::size_type::FIXED;
                lbl->size        = { 0.0f, 20.0f };
                lbl->pos         = { 8.0f, y };
                scr->contents->add_child(lbl);
                y += 20.0f;
            };

            add_row("Pass:   " + to_str(info.name));
            if (info.disabled)
            {
                add_row("Status: DISABLED", col_dim);
                scr->contents->size = { 0.0f, y + 8.0f };
                add_child(scr);
                return;
            }

            add_row("Queue:  " + std::string(
                info.queue == HAL::CommandListType::DIRECT ? "Direct" : "Compute"));
            add_row("ID:     #" + std::to_string(info.call_id));
            add_row(std::string("Fence:  ") + (info.put_fence ? "yes" : "no"));

            auto queue_name = [](HAL::CommandListType t) -> const char*
            {
                switch (t) {
                    case HAL::CommandListType::DIRECT:  return "Direct";
                    case HAL::CommandListType::COMPUTE: return "Compute";
                    case HAL::CommandListType::COPY:    return "Copy";
                    default:                            return "?";
                }
            };
            for (auto& [type, dep_id] : info.cross_queue_deps)
            {
                std::string dep_str = "#" + std::to_string(dep_id);
                for (auto& p : all_passes)
                    if (p.call_id == dep_id)
                        { dep_str = to_str(p.name) + "  (#" + std::to_string(dep_id) + ")"; break; }
                add_row(std::string("Sync ") + queue_name(type) + ":  " + dep_str);
            }
            if (info.cross_queue_deps.empty())
                add_row("Sync:   none");

            if constexpr (!BuildOptions::Dev)
            {
                y += 6.0f;
                add_row("-- Commands (Dev=false, not recorded)", col_dim);
            }
            else if (!info.debug_commands.empty())
            {
                y += 6.0f;
                add_row("-- Commands (" +
                        std::to_string(info.debug_commands.size()) + ")", col_dim);
                y += 2.0f;

                auto cmd_color = [](HAL::CommandType t) -> float4
                {
                    using CT = HAL::CommandType;
                    switch (t)
                    {
                        case CT::Transition:
                            return { 0.90f, 0.65f, 0.20f, 1.0f };
                        case CT::Draw: case CT::DrawIndexed:
                        case CT::DispatchMesh: case CT::ExecuteIndirect:
                            return { 0.40f, 0.82f, 0.40f, 1.0f };
                        case CT::Dispatch: case CT::DispatchGraph: case CT::DispatchRays:
                            return { 0.30f, 0.72f, 0.92f, 1.0f };
                        case CT::CopyResource: case CT::CopyBuffer: case CT::CopyTexture:
                        case CT::UpdateTexture: case CT::ReadTexture: case CT::BuildRAS:
                            return { 0.30f, 0.78f, 0.68f, 1.0f };
                        case CT::ClearRTV: case CT::ClearUAV:
                        case CT::ClearDepth: case CT::ClearStencil: case CT::ClearDepthStencil:
                            return { 0.75f, 0.42f, 0.85f, 1.0f };
                        case CT::StartEvent: case CT::EndEvent:
                            return { 0.90f, 0.90f, 0.92f, 1.0f };
                        default:
                            return { 0.55f, 0.55f, 0.58f, 1.0f };
                    }
                };

                for (auto& cmd : info.debug_commands)
                {
                    std::string text = cmd.description.empty() ? "(unknown)" : cmd.description;
                    if (cmd.type == HAL::CommandType::Transition)
                        text = "Barrier  " + text;
                    add_row(text, cmd_color(cmd.type));
                }
            }

            scr->contents->size = { 0.0f, y + 8.0f };
            add_child(scr);
        }
    };

    // -----------------------------------------------------------------------
    //  Barrier-occurrence list shown when a resource cell is clicked.
    // -----------------------------------------------------------------------
    struct ResourceBarrierContent : GUI::base
    {
        using ptr = std::shared_ptr<ResourceBarrierContent>;

        ResourceBarrierContent(const ResourceTrack& track,
                                const std::vector<PassInfo>& passes,
                                UINT clicked_call_id)
        {
            docking     = GUI::dock::FILL;
            width_size  = GUI::size_type::MATCH_PARENT;
            height_size = GUI::size_type::MATCH_PARENT;

            auto scr          = std::make_shared<scroll_container>();
            scr->docking      = GUI::dock::FILL;
            scr->width_size   = GUI::size_type::MATCH_PARENT;
            scr->height_size  = GUI::size_type::MATCH_PARENT;
            scr->hor->visible = false;
            scr->contents->width_size  = GUI::size_type::MATCH_PARENT;
            scr->contents->height_size = GUI::size_type::FIXED;

            float y = 8.0f;
            const float4 col_bright  = { 0.90f, 0.90f, 0.92f, 1.0f };
            const float4 col_dim     = { 0.55f, 0.55f, 0.58f, 1.0f };
            const float4 col_barrier = { 0.90f, 0.65f, 0.20f, 1.0f };
            const float4 col_sel     = { 1.00f, 1.00f, 0.50f, 1.0f };
            const float4 col_break   = { 1.00f, 0.40f, 0.10f, 1.0f }; // active breakpoint

            // Returns a clickable_label so callers can wire up on_click handlers.
            auto add_row = [&](const std::string& text, float indent,
                               float4 color = { 0.90f, 0.90f, 0.92f, 1.0f }) -> clickable_label::ptr
            {
                auto lbl         = std::make_shared<clickable_label>();
                lbl->text        = text;
                lbl->color       = color;
                lbl->font_size   = 11.0f;
                lbl->docking     = GUI::dock::NONE;
                lbl->width_size  = GUI::size_type::MATCH_PARENT;
                lbl->height_size = GUI::size_type::FIXED;
                lbl->size        = { 0.0f, 18.0f };
                lbl->pos         = { 8.0f + indent, y };
                scr->contents->add_child(lbl);
                y += 18.0f;
                return lbl;
            };

            add_row("Resource: " + track.name, 0.0f);
            y += 4.0f;
            add_row("-- Transitions --", 0.0f, col_dim);
            y += 2.0f;

            // Collect all barriers for this resource in pass order, then validate.
            struct BarrierEntry
            {
                const PassInfo*                              pass   = nullptr;
                const HAL::CommandRecord::BarrierDetail*     detail = nullptr;
                bool                                         mismatch = false;
            };
            std::vector<BarrierEntry> entries;

            // Passes are already in call_id order (enabled first, disabled appended later).
            for (auto& pass : passes)
            {
                if (pass.disabled) continue;
                for (auto& cmd : pass.debug_commands)
                {
                    if (cmd.type != HAL::CommandType::Transition) continue;
                    for (auto& bd : cmd.barrier_details)
                        if (bd.resource_name == track.name)
                            entries.push_back({ &pass, &bd });
                }
            }

            // Validate per-subresource continuity and cross-frame state.
            // No ALL_SUBRESOURCES slot — the engine always emits individual barriers.
            {
                const bool is_persistent = track.is_static || track.is_passed;

                std::unordered_map<uint, HAL::ResourceState> sub_state;
                std::unordered_map<uint, size_t>             first_idx; // subres -> first entry index

                for (size_t i = 0; i < entries.size(); ++i)
                {
                    auto&       e           = entries[i];
                    const auto& bd          = *e.detail;
                    const uint  sr          = bd.subres;
                    const bool  has_discard = (static_cast<uint>(bd.flags) &
                                               static_cast<uint>(HAL::BarrierFlags::DISCARD)) != 0;

                    const bool is_first = (first_idx.find(sr) == first_idx.end());
                    if (is_first)
                        first_idx[sr] = i;

                    if (is_first)
                    {
                        if (is_persistent)
                        {
                            // Persistent: must DISCARD or declare a real before-layout.
                            // Only layout is validated — sync/access are not tracked state.
                            if (!has_discard &&
                                bd.before.layout == HAL::TextureLayout::UNDEFINED)
                                e.mismatch = true;
                        }
                        else
                        {
                            // Non-persistent: first barrier must always begin with
                            // UNDEFINED layout and carry the DISCARD flag.
                            if (!has_discard ||
                                bd.before.layout != HAL::TextureLayout::UNDEFINED)
                                e.mismatch = true;
                        }
                    }
                    else
                    {
                        // Non-first occurrence: only layout must carry over — sync/access
                        // in "before" describe synchronization scope, not previous state.
                        auto it = sub_state.find(sr);
                        if (it != sub_state.end())
                        {
                            if (bd.before.layout != it->second.layout)
                                e.mismatch = true;
                        }
                    }

                    sub_state[sr] = bd.after;
                }

            }

            const float4 col_err     = C_MISMATCH;
            const float4 col_err_dim = { 0.80f, 0.40f, 0.35f, 1.0f };

            if (entries.empty())
            {
                if constexpr (!BuildOptions::Dev)
                    add_row("(Dev=false, transitions not recorded)", 0.0f, col_dim);
                else
                    add_row("(no transitions recorded)", 0.0f, col_dim);
            }
            else
            {
                const PassInfo* last_pass = nullptr;
                for (auto& e : entries)
                {
                    // Print pass header whenever the pass changes.
                    if (e.pass != last_pass)
                    {
                        last_pass = e.pass;
                        y += 4.0f;
                        float4 hcol = e.mismatch                           ? col_err
                                    : (e.pass->call_id == clicked_call_id) ? col_sel
                                                                            : col_bright;
                        add_row(to_str(e.pass->name), 0.0f, hcol);
                    }

                    const auto& bd = *e.detail;

                    // Subresource / split-barrier annotations on the Sync line.
                    std::string sub_str;
                    if (bd.subres != HAL::ALL_SUBRESOURCES)
                        sub_str = "  sub=" + std::to_string(bd.subres);

                    std::string flag_str;
                    using BF = HAL::BarrierFlags;
                    auto bf = static_cast<uint>(bd.flags);
                    if      ((bf & static_cast<uint>(BF::BEGIN)) && !(bf & static_cast<uint>(BF::END)))
                        flag_str = "  [begin]";
                    else if ((bf & static_cast<uint>(BF::END))   && !(bf & static_cast<uint>(BF::BEGIN)))
                        flag_str = "  [end]";
                    else if (bf & static_cast<uint>(BF::DISCARD))
                        flag_str = "  [discard]";

                    const HAL::Debug::BreakKey bp_key{ bd.resource_name, bd.subres, bd.before, bd.after };
                    const bool bp_on  = HAL::Debug::BarrierBreakpoints::has(bp_key);
                    float4 sc = e.mismatch ? col_err : (bp_on ? col_break : col_barrier);
                    float4 dc = e.mismatch ? col_err_dim : col_dim;

                    // The Sync line is clickable — toggles a GPU-side __debugbreak() that
                    // fires only when this exact before->after transition is executed.
                    const std::string sync_base =
                        "Sync:   " + barrier_sync_str(bd.before.operation) +
                        "  ->  "   + barrier_sync_str(bd.after.operation) +
                        sub_str + flag_str;
                    const std::string bp_prefix = bp_on ? "[*] " : "    ";

                    auto sync_lbl        = add_row(bp_prefix + sync_base, 12.0f, sc);
                    sync_lbl->clickable  = true;

                    const bool is_err = e.mismatch;
                    sync_lbl->on_click = [lbl      = sync_lbl,
                                          bp_key, sync_base, is_err,
                                          c_ok = col_barrier, c_bp = col_break]
                                         () mutable
                    {
                        bool now_active = HAL::Debug::BarrierBreakpoints::toggle(bp_key);
                        lbl->text  = (now_active ? "[*] " : "    ") + sync_base;
                        if (!is_err)
                            lbl->color = now_active ? c_bp : c_ok;
                    };

                    add_row("Access: " + barrier_access_str(bd.before.access) +
                            "  ->  "   + barrier_access_str(bd.after.access),
                            12.0f, dc);
                    add_row("Layout: " + barrier_layout_str(bd.before.layout) +
                            "  ->  "   + barrier_layout_str(bd.after.layout),
                            12.0f, dc);
                }
            }

            scr->contents->size = { 0.0f, y + 8.0f };
            add_child(scr);
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

        // Selected mip level: written from UI thread (combobox), read from render thread.
        volatile int m_sel_mip   = 0;
        // Selected array index: written from UI thread (combobox), read from render thread.
        volatile int m_sel_array = 0;

        combo_box::ptr m_mip_combo;
        combo_box::ptr m_array_combo;

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

            // Status bar — mip selector
            {
                auto bar              = std::make_shared<GUI::base>();
                bar->docking          = GUI::dock::TOP;
                bar->height_size      = GUI::size_type::FIXED;
                bar->size             = { 0.0f, 26.0f };

                auto mip_lbl          = std::make_shared<label>();
                mip_lbl->text         = "Mip:";
                mip_lbl->font_size    = 10.0f;
                mip_lbl->docking      = GUI::dock::NONE;
                mip_lbl->width_size   = GUI::size_type::FIXED;
                mip_lbl->height_size  = GUI::size_type::FIXED;
                mip_lbl->size         = { 28.0f, 16.0f };
                mip_lbl->pos          = { 4.0f, 5.0f };
                bar->add_child(mip_lbl);

                m_mip_combo           = std::make_shared<combo_box>();
                m_mip_combo->docking  = GUI::dock::NONE;
                m_mip_combo->width_size  = GUI::size_type::FIXED;
                m_mip_combo->height_size = GUI::size_type::FIXED;
                m_mip_combo->size     = { 84.0f, 20.0f };
                m_mip_combo->pos      = { 34.0f, 3.0f };
                m_mip_combo->visible  = false;
                bar->add_child(m_mip_combo);

                auto arr_lbl          = std::make_shared<label>();
                arr_lbl->text         = "Array:";
                arr_lbl->font_size    = 10.0f;
                arr_lbl->docking      = GUI::dock::NONE;
                arr_lbl->width_size   = GUI::size_type::FIXED;
                arr_lbl->height_size  = GUI::size_type::FIXED;
                arr_lbl->size         = { 36.0f, 16.0f };
                arr_lbl->pos          = { 124.0f, 5.0f };
                bar->add_child(arr_lbl);

                m_array_combo           = std::make_shared<combo_box>();
                m_array_combo->docking  = GUI::dock::NONE;
                m_array_combo->width_size  = GUI::size_type::FIXED;
                m_array_combo->height_size = GUI::size_type::FIXED;
                m_array_combo->size     = { 84.0f, 20.0f };
                m_array_combo->pos      = { 162.0f, 3.0f };
                m_array_combo->visible  = false;
                bar->add_child(m_array_combo);

                docker->add_child(bar);
            }

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
                            m_current_tex = std::make_shared<Texture>(RenderSystem::get().device(), desc);

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

                                const bool is_cube = !!dynamic_cast<HAL::CubeView*>(m_alloc->view.get());
                                const bool is_3d   = !is_cube && (d.z > 1);
                                std::string t = is_cube ? "TextureCube"
                                              : is_3d   ? "Texture3D" : "Texture2D";
                                std::string s = std::to_string(d.x) + "x" + std::to_string(d.y)
                                              + (is_cube ? "x6"
                                              : is_3d   ? "x" + std::to_string(d.z) : "");
                                run_on_ui([this, t, s]()
                                {
                                    m_type_lbl->text = "Type:     " + t;
                                    m_dim_lbl->text  = "Size:     " + s;
                                });

                                UINT mip_count   = m_alloc->resource->get_desc().as_texture().MipLevels;
                                UINT array_count = m_alloc->resource->get_desc().as_texture().ArraySize;
                                run_on_ui([this, mip_count]()
                                {
                                    m_mip_combo->remove_items();
                                    for (UINT i = 0; i < mip_count; ++i)
                                    {
                                        auto item = m_mip_combo->add_item("Mip " + std::to_string(i));
                                        item->on_select = [this, i]() { m_sel_mip = (int)i; };
                                    }
                                    if (mip_count > 0)
                                        m_mip_combo->get_label()->text = "Mip 0";
                                    m_mip_combo->visible = (mip_count > 1);
                                });
                                run_on_ui([this, array_count]()
                                {
                                    m_array_combo->remove_items();
                                    for (UINT i = 0; i < array_count; ++i)
                                    {
                                        auto item = m_array_combo->add_item("Array " + std::to_string(i));
                                        item->on_select = [this, i]() { m_sel_array = (int)i; };
                                    }
                                    if (array_count > 0)
                                        m_array_combo->get_label()->text = "Array 0";
                                    m_array_combo->visible = (array_count > 1);
                                });
                            }

                            // Hand the new texture to the image widget on the UI thread.
                            auto tex = m_current_tex;
                            run_on_ui([this, tex]()
                            {
                                m_img->texture.texture = tex->texture_2d();
                                m_img->visible         = true;
                                m_buffer_tree->visible = false;
                            });
                        }

                        if (!m_current_tex) return;

                        auto& compute = context->get_list()->get_compute();
                        {
                            Slots::FrameGraph_Debug_Common common;
                            common.GetTarget()             = m_current_tex->texture_2d().rwTexture2D;
                            common.GetTargetSize()         = SZ;
                            common.GetSelectedMip()        = (UINT)m_sel_mip;
                            common.GetSelectedArrayIndex() = (UINT)m_sel_array;
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
                            UINT array_size = m_alloc->resource->get_desc().as_texture().ArraySize;
                            if (array_size > 1)
                            {
                                compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2DArray>();
                                Slots::FrameGraph_Debug_Texture2DArray tex2darr;
                                tex2darr.GetSource()     = *src;
                                tex2darr.GetSourceSize() = m_alloc->resource->get_desc().as_texture().Dimensions.xy;
                                tex2darr.GetOffset()     = snap_pan;
                                tex2darr.GetScale()      = snap_scale;
                                compute.set(tex2darr);
                            }
                            else
                            {
                                compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
                                Slots::FrameGraph_Debug_Texture2D tex2d;
                                tex2d.GetSource()     = *src;
                                tex2d.GetSourceSize() = m_alloc->resource->get_desc().as_texture().Dimensions.xy;
                                tex2d.GetOffset()     = snap_pan;
                                tex2d.GetScale()      = snap_scale;
                                compute.set(tex2d);
                            }
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
                        else if (auto* src = dynamic_cast<HAL::CubeView*>(m_alloc->view.get()))
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_TextureCube>();
                            Slots::FrameGraph_Debug_TextureCube cube;
                            cube.GetSource()     = src->textureCube;
                            cube.GetSourceSize() = m_alloc->resource->get_desc().as_texture().Dimensions.xy;
                            compute.set(cube);
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
                        else if (auto* src = dynamic_cast<HAL::ByteBufferView*>(m_alloc->view.get()))
                        {
                            if (!m_buffer_inited)
                            {
                                m_buffer_inited  = true;
                                uint64 sz = src->get_desc().as_buffer().SizeInBytes;
                                run_on_ui([this, sz]()
                                {
                                    m_type_lbl->text       = "Type:     ByteBuffer";
                                    m_dim_lbl->text        = "Size:     " + std::to_string(sz) + "B";
                                    m_img->visible         = false;
                                    m_buffer_tree->visible = false;
                                });
                            }
                        }
                        else if (auto* src = dynamic_cast<HAL::CounterView*>(m_alloc->view.get()))
                        {
                            if (!m_buffer_inited)
                            {
                                m_buffer_inited  = true;
                                run_on_ui([this]()
                                {
                                    m_type_lbl->text       = "Type:     Counter";
                                    m_dim_lbl->text        = "Size:     1 x 4B";
                                    m_img->visible         = false;
                                    m_buffer_tree->visible = false;
                                });
                            }
                        }
                        else if (auto* src = dynamic_cast<HAL::RTXSceneView*>(m_alloc->view.get()))
                        {
                            if (!m_buffer_inited)
                            {
                                m_buffer_inited  = true;
                                run_on_ui([this]()
                                {
                                    m_type_lbl->text       = "Type:     RTX Scene";
                                    m_dim_lbl->text        = "Size:     (BVH)";
                                    m_img->visible         = false;
                                    m_buffer_tree->visible = false;
                                });
                            }
                        }
                        else if (auto* src = dynamic_cast<HAL::BufferView*>(m_alloc->view.get()))
                        {
                            // FormattedBufferView<T,F> — template, no common non-template base
                            if (!m_buffer_inited)
                            {
                                m_buffer_inited  = true;
                                uint64 sz = src->get_desc().as_buffer().SizeInBytes;
                                run_on_ui([this, sz]()
                                {
                                    m_type_lbl->text       = "Type:     FormattedBuffer";
                                    m_dim_lbl->text        = "Size:     " + std::to_string(sz) + "B";
                                    m_img->visible         = false;
                                    m_buffer_tree->visible = false;
                                });
                            }
                        }
                        else
                        {
                            run_on_ui([this]()
                            {
                                m_type_lbl->text = "Type:     (unhandled view type)";
                            });
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
    content_scroll::ptr      m_scroll;
    left_panel_overlay::ptr  m_left_panel;
    top_panel_overlay::ptr   m_top_panel;
    corner_overlay::ptr      m_corner;
    grid_canvas::ptr         m_grid_canvas;
    minimap_overlay::ptr     m_minimap;
    GUI::base::ptr           m_preview_panel;
    GUI::base::ptr           m_barriers_panel;

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
        pass_button::ptr     btn;
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
    static const float4 C_PASS_DISABLED;
    static const float4 C_RUNS_ALONE;
    static const float4 C_WRITE;
    static const float4 C_READ;
    static const float4 C_WRITE_DISABLED;
    static const float4 C_READ_DISABLED;
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
    static const float4 C_SEL_WAIT_PASS;
    static const float4 C_MISMATCH;

    // -----------------------------------------------------------------------
    //  Helpers
    // -----------------------------------------------------------------------
    static std::string resource_flags_to_str(FrameGraph::ResourceFlags flags)
    {
        using RF = FrameGraph::ResourceFlags;
        std::string s;
        auto add = [&](RF f, const char* name){ if (check(flags & f)){ if (!s.empty()) s += '|'; s += name; } };
        add(RF::RenderTarget,    "RT");
        add(RF::DepthStencil,    "DS");
        add(RF::UnorderedAccess, "UAV");
        add(RF::CopyDest,        "DST");
        add(RF::GenCPU,          "CPU");
        add(RF::PixelRead,       "PSR");
        add(RF::ComputeRead,     "CSR");
        add(RF::DSRead,          "DS_R");
        add(RF::CopySource,      "SRC");
        add(RF::ReadCPU,         "CPU");
        return s;
    }

    static std::string layout_to_str(HAL::TextureLayout layout)
    {
        using TL = HAL::TextureLayout;
        auto has = [&](TL f){ return (static_cast<uint32_t>(layout) & static_cast<uint32_t>(f)) != 0; };
        if (!static_cast<uint32_t>(layout)) return "";
        std::string s;
        auto add = [&](TL f, const char* name){ if (has(f)){ if (!s.empty()) s += '|'; s += name; } };
        add(TL::SHADER_RESOURCE,    "SRV");
        add(TL::COPY_SOURCE,        "SRC");
        add(TL::DEPTH_STENCIL_READ, "DS_R");
        add(TL::COPY_QUEUE,         "CQ");
        add(TL::PRESENT,            "PRESENT");
        return s;
    }

    static std::string barrier_sync_str(HAL::BarrierSync s)
    {
        using BS = HAL::BarrierSync;
        if (s == BS::NONE) return "NONE";
        auto u   = static_cast<uint>(s);
        auto has = [&](BS f) { return (u & static_cast<uint>(f)) != 0; };
        // Check composite aliases first.
        if (u == static_cast<uint>(BS::ALL_DIRECT))  return "ALL_DIRECT";
        if (u == static_cast<uint>(BS::ALL_COMPUTE)) return "ALL_COMPUTE";
        if (u == static_cast<uint>(BS::ALL_SHADING)) return "ALL_SHADING";
        if (u == static_cast<uint>(BS::DRAW))        return "DRAW";
        std::string r;
        auto add = [&](BS f, const char* name){ if (has(f)){ if (!r.empty()) r += '|'; r += name; } };
        add(BS::INDEX_INPUT,      "IDX");
        add(BS::VERTEX_SHADING,   "VS");
        add(BS::PIXEL_SHADING,    "PS");
        add(BS::DEPTH_STENCIL,    "DS");
        add(BS::RENDER_TARGET,    "RT");
        add(BS::COMPUTE_SHADING,  "CS");
        add(BS::RAYTRACING,       "RAY");
        add(BS::COPY,             "COPY");
        add(BS::EXECUTE_INDIRECT, "IND");
        add(BS::CLEAR_UNORDERED_ACCESS_VIEW, "CLEAR_UAV");
        add(BS::BUILD_RAYTRACING_ACCELERATION_STRUCTURE, "BVH");
        add(BS::COPY_RAYTRACING_ACCELERATION_STRUCTURE,  "BVH_COPY");
        add(BS::SPLIT,            "SPLIT");
        return r.empty() ? "?" : r;
    }

    static std::string barrier_access_str(HAL::BarrierAccess a)
    {
        using BA = HAL::BarrierAccess;
        if (static_cast<uint>(a) == 0)                               return "COMMON";
        if (static_cast<uint>(a) == static_cast<uint>(BA::NO_ACCESS)) return "NONE";
        auto has = [&](BA f) { return (static_cast<uint>(a) & static_cast<uint>(f)) != 0; };
        std::string r;
        auto add = [&](BA f, const char* name){ if (has(f)){ if (!r.empty()) r += '|'; r += name; } };
        add(BA::VERTEX_BUFFER,    "VBV");
        add(BA::CONSTANT_BUFFER,  "CBV");
        add(BA::INDEX_BUFFER,     "IBV");
        add(BA::RENDER_TARGET,    "RTV");
        add(BA::UNORDERED_ACCESS, "UAV");
        add(BA::DEPTH_STENCIL_WRITE, "DSV-W");
        add(BA::DEPTH_STENCIL_READ,  "DSV-R");
        add(BA::SHADER_RESOURCE,  "SRV");
        add(BA::INDIRECT_ARGUMENT,"IND");
        add(BA::COPY_DEST,        "CPY-D");
        add(BA::COPY_SOURCE,      "CPY-S");
        add(BA::RAYTRACING_ACCELERATION_STRUCTURE_READ,  "BVH-R");
        add(BA::RAYTRACING_ACCELERATION_STRUCTURE_WRITE, "BVH-W");
        return r.empty() ? "?" : r;
    }

    static std::string barrier_layout_str(HAL::TextureLayout l)
    {
        using TL = HAL::TextureLayout;
        if (l == TL::UNDEFINED) return "UNDEF";
        if (l == TL::NONE)      return "NONE";
        auto has = [&](TL f) { return (static_cast<uint32_t>(l) & static_cast<uint32_t>(f)) != 0; };
        std::string r;
        auto add = [&](TL f, const char* name){ if (has(f)){ if (!r.empty()) r += '|'; r += name; } };
        add(TL::PRESENT,             "PRESENT");
        add(TL::RENDER_TARGET,       "RTV");
        add(TL::UNORDERED_ACCESS,    "UAV");
        add(TL::DEPTH_STENCIL_WRITE, "DSV-W");
        add(TL::DEPTH_STENCIL_READ,  "DSV-R");
        add(TL::SHADER_RESOURCE,     "SRV");
        add(TL::COPY_SOURCE,         "CPY-S");
        add(TL::COPY_DEST,           "CPY-D");
        add(TL::COPY_QUEUE,          "CQ");
        return r.empty() ? "?" : r;
    }

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
        lbl->clamp_to_parent = GUI::ParentClamp::ALL;
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

        m_scroll = std::make_shared<content_scroll>(*this);

        // Grid canvas — first child inside contents (labels/images render on top).
        m_grid_canvas = std::make_shared<grid_canvas>(*this);
        m_scroll->add_child(m_grid_canvas);

        // Overlay panels — drawn on top of contents via base::add_child.
        // Corner: top-left intersection, never scrolls.
        m_corner = std::make_shared<corner_overlay>(*this);
        m_lbl_direct  = make_label(LABEL_W - 8.0f, LANE_H - 6.0f, "DIRECT",  C_TEXT_BRIGHT, LABEL_FONT);
        m_lbl_compute = make_label(LABEL_W - 8.0f, LANE_H - 6.0f, "COMPUTE", C_TEXT_BRIGHT, LABEL_FONT);
        m_lbl_direct->pos  = { 4.0f, (LANE_H - 14.0f) * 0.5f };
        m_lbl_compute->pos = { 4.0f, LANE_H + (LANE_H - 14.0f) * 0.5f };
        m_corner->add_child(m_lbl_direct);
        m_corner->add_child(m_lbl_compute);
        m_scroll->add_overlay(m_corner);

        // Left panel: pinned at X=0, follows Y scroll.
        m_left_panel = std::make_shared<left_panel_overlay>(*this);
        m_scroll->add_overlay(m_left_panel);

        // Top panel: follows X scroll, pinned at Y=0.
        m_top_panel = std::make_shared<top_panel_overlay>(*this);
        m_scroll->add_overlay(m_top_panel);

        // Minimap — fixed at the bottom of the scroll viewport.
        m_minimap = std::make_shared<minimap_overlay>(*this);
        m_scroll->add_overlay(m_minimap);

        // Right dock on this (dock_base) — permanent preview panel.
        auto right_dock  = get_dock(GUI::dock::RIGHT);
        right_dock->size = { 400.0f, 0.0f };

        m_preview_panel              = std::make_shared<GUI::base>();
        m_preview_panel->docking     = GUI::dock::FILL;
        m_preview_panel->width_size  = GUI::size_type::MATCH_PARENT;
        m_preview_panel->height_size = GUI::size_type::MATCH_PARENT;
        right_dock->get_tabs()->add_page("Preview", m_preview_panel);

        m_barriers_panel              = std::make_shared<GUI::base>();
        m_barriers_panel->docking     = GUI::dock::FILL;
        m_barriers_panel->width_size  = GUI::size_type::MATCH_PARENT;
        m_barriers_panel->height_size = GUI::size_type::MATCH_PARENT;
        right_dock->get_tabs()->add_page("Transitions", m_barriers_panel);

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
                if (cell.preview)       m_scroll->remove_child(cell.preview);
                if (cell.state_lbl)     m_scroll->remove_child(cell.state_lbl);
                if (cell.debug_handler) cell.debug_handler->unregister();
            }

        m_passes.clear();
        m_resources.clear();
        m_max_call_id = 0;

        for (auto* pass : g.builder.enabled_passes)
        {
            PassInfo info;
            info.name           = pass->name;
            info.call_id        = pass->call_id;
            info.queue          = pass->get_type();
            info.put_fence      = pass->put_fence;
            info.debug_commands = pass->debug_commands;
            static const HAL::CommandListType all_types[] = {
                HAL::CommandListType::DIRECT,
                HAL::CommandListType::COMPUTE,
                HAL::CommandListType::COPY,
            };
            for (auto type : all_types)
            {
                if (type == pass->get_type()) continue;
                const auto* dep = pass->sync_state.values[type];
                if (!dep || dep->call_id == pass->call_id) continue;
                info.cross_queue_deps.push_back({ type, dep->call_id });
            }
            m_passes.push_back(info);
            if (pass->call_id > m_max_call_id)
                m_max_call_id = pass->call_id;
        }

        // Disabled passes — synthetic call_ids placed after the enabled section.
        std::unordered_map<const FrameGraph::Pass*, UINT> disabled_col;
        {
            UINT next_col = m_max_call_id + 2;
            for (auto& sp : g.builder.passes)
            {
                if (sp->enabled) continue;
                PassInfo info;
                info.name     = sp->name;
                info.call_id  = next_col;
                info.queue    = sp->get_type();
                info.disabled = true;
                disabled_col[sp.get()] = next_col;
                m_passes.push_back(info);
                if (next_col > m_max_call_id) m_max_call_id = next_col;
                ++next_col;
            }
        }

        // Compute runs_alone: enabled passes with no concurrent pass on any other queue.
        for (auto& pass : m_passes)
        {
            if (pass.disabled) continue;
            // lo = the last other-queue call_id that any same-queue pass at <= this call_id
            // waited on. We accumulate across all predecessors so that passes with no deps
            // of their own still inherit the fence established by an earlier pass.
            UINT lo = 0;
            for (auto& other : m_passes)
            {
                if (other.disabled || other.queue != pass.queue || other.call_id > pass.call_id) continue;
                for (auto& [dep_queue, dep_id] : other.cross_queue_deps)
                    lo = std::max(lo, dep_id);
            }
            // hi = call_id of the first other-queue pass that waits on this queue at >= pass.call_id.
            UINT hi = m_max_call_id + 1;
            for (auto& other : m_passes)
            {
                if (other.disabled || other.queue == pass.queue) continue;
                for (auto& [dep_queue, dep_id] : other.cross_queue_deps)
                    if (dep_queue == pass.queue && dep_id >= pass.call_id)
                        hi = std::min(hi, other.call_id);
            }
            // If no other-queue pass has call_id strictly inside (lo, hi), this pass runs alone.
            bool has_concurrent = false;
            for (auto& other : m_passes)
            {
                if (other.disabled || other.queue == pass.queue) continue;
                if (other.call_id > lo && other.call_id < hi)
                    { has_concurrent = true; break; }
            }
            pass.runs_alone = !has_concurrent;
        }

        std::map<std::string, ResourceTrack> track_map;
        for (auto* pass : g.builder.enabled_passes)
        {
            for (auto& [alloc, flags] : pass->used.resource_flags)
            {
                auto& tr = track_map[alloc->name];
                tr.name      = alloc->name;
                tr.is_static = alloc->is_static();
                tr.is_passed = alloc->passed;

                ResourceCell cell;
                cell.call_id    = pass->call_id;
                cell.is_write   = check(flags & FrameGraph::WRITEABLE_FLAGS);
                cell.is_created = (pass->used.resource_creations.count(alloc) > 0);
                cell.is_deleted = (pass->used.resource_deletions_after.count(alloc) > 0);
                cell.alloc      = alloc;
                cell.state_str = resource_flags_to_str(flags);
              
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

        // Disabled pass resource cells — muted, no thumbnails.
        for (auto& sp : g.builder.passes)
        {
            if (sp->enabled) continue;
            auto it = disabled_col.find(sp.get());
            if (it == disabled_col.end()) continue;
            UINT col = it->second;
            for (auto& [alloc, flags] : sp->used.resource_flags)
            {
                auto& tr    = track_map[alloc->name];
                tr.name     = alloc->name;
                tr.is_static = alloc->is_static();
                tr.is_passed = alloc->passed;
                ResourceCell cell;
                cell.call_id  = col;
                cell.is_write = check(flags & FrameGraph::WRITEABLE_FLAGS);
                cell.alloc    = alloc;
                cell.disabled = true;
                tr.cells.push_back(cell);
            }
        }

        for (auto& [name, tr] : track_map)
        {
            for (size_t i = 1; i < tr.cells.size(); ++i)
                if (!tr.cells[i].disabled && !tr.cells[i - 1].disabled &&
                    tr.cells[i].is_write != tr.cells[i - 1].is_write)
                    tr.cells[i].has_barrier = true;
            for (auto& cell : tr.cells)
                if (cell.alloc && cell.alloc->non_deleted)
                    { tr.is_non_deleted = true; break; }
            m_resources.push_back(std::move(tr));
        }

        // Validate barrier before/after continuity for every resource track.
        {
            std::vector<const PassInfo*> sorted;
            sorted.reserve(m_passes.size());
            for (auto& p : m_passes)
                if (!p.disabled) sorted.push_back(&p);
            std::sort(sorted.begin(), sorted.end(),
                      [](const PassInfo* a, const PassInfo* b){ return a->call_id < b->call_id; });

            for (auto& tr : m_resources)
            {
                // Per-subresource last-known after-state. No ALL_SUBRESOURCES key —
                // their system always emits individual per-subresource barriers.
                std::unordered_map<uint, HAL::ResourceState> sub_state;

                const bool is_persistent = tr.is_static || tr.is_passed;

                // First-occurrence info per subresource, for the cross-frame check.
                struct FirstOcc { HAL::BarrierAccess access; HAL::TextureLayout layout; bool has_discard; };
                std::unordered_map<uint, FirstOcc> first_occ;

                for (auto* pass : sorted)
                {
                    if (tr.has_mismatch) break;
                    for (auto& cmd : pass->debug_commands)
                    {
                        if (tr.has_mismatch) break;
                        if (cmd.type != HAL::CommandType::Transition) continue;
                        for (auto& bd : cmd.barrier_details)
                        {
                            if (bd.resource_name != tr.name) continue;

                            auto bf = static_cast<uint>(bd.flags);
                            bool has_discard = (bf & static_cast<uint>(HAL::BarrierFlags::DISCARD)) != 0;

                            bool is_first = !first_occ.count(bd.subres);
                            if (is_first)
                            {
                                first_occ[bd.subres] = { bd.before.access, bd.before.layout, has_discard };

                                if (is_persistent)
                                {
                                    // Persistent: must DISCARD or declare a real before-layout.
                                    // Only layout is validated — sync/access are not tracked state.
                                    // UNDEFINED without DISCARD means D3D12 will validate against
                                    // its tracked layout from the previous frame and error.
                                    if (!has_discard &&
                                        bd.before.layout == HAL::TextureLayout::UNDEFINED)
                                    {
                                        tr.has_mismatch = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    // Non-persistent: first barrier must always begin with
                                    // UNDEFINED layout and carry the DISCARD flag.
                                    if (!has_discard ||
                                        bd.before.layout != HAL::TextureLayout::UNDEFINED)
                                    {
                                        tr.has_mismatch = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                // Continuity: only layout must carry over between passes.
                                // Sync/access in "before" describe synchronization scope,
                                // not the resource's previous tracked state.
                                auto it = sub_state.find(bd.subres);
                                if (it != sub_state.end() &&
                                    bd.before.layout != it->second.layout)
                                {
                                    tr.has_mismatch = true;
                                    break;
                                }
                            }
                            sub_state[bd.subres] = bd.after;
                        }
                    }
                }

            }
        }

        // Resource-name labels + type icons in left column.
        for (auto& lbl  : m_resource_labels) m_left_panel->remove_child(lbl);
        for (auto& icon : m_resource_icons)  m_left_panel->remove_child(icon);
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
            icon->clamp_to_parent = GUI::ParentClamp::ALL;
            icon->clickable      = false;
			icon->texture.texture = Skin::get().DefaultEditBox.Normal.texture;

            // TODO: set icon->texture.texture to an appropriate loaded texture, e.g.:
            //   Texture2D  -> load_icon("path/to/texture2d.png")
            //   Texture3D  -> load_icon("path/to/texture3d.png")
            //   Buffer     -> load_icon("path/to/buffer.png")
            // Icons are assigned later via run_on_ui once the resource type is known.
            m_left_panel->add_child(icon);
            m_resource_icons.push_back(icon);

            // Resource name label — indented past the icon.
            float ly  = row_top + (ROW_H - 14.0f) * 0.5f;
            float lx  = 4.0f + ICON_W + 4.0f;
            auto lbl  = make_label(LABEL_W - lx - 4.0f, ROW_H - 4.0f,
                                   m_resources[ri].name,
                                   m_resources[ri].has_mismatch ? C_MISMATCH : C_TEXT_BRIGHT,
                                   LABEL_FONT);
            lbl->pos  = { lx, ly };
            m_left_panel->add_child(lbl);
            m_resource_labels.push_back(lbl);

            // Resource type label — bottom-right corner of the left-panel row.
            std::string type_str;
            for (auto& cell : m_resources[ri].cells)
            {
                if (!cell.alloc || !cell.alloc->resource) continue;
                auto& desc = cell.alloc->resource->get_desc();
                if (desc.is_texture())
                {
                    if (cell.alloc->view && dynamic_cast<HAL::CubeView*>(cell.alloc->view.get()))
                        type_str = "Cube";
                    else
                        type_str = (desc.as_texture().Dimensions.z > 1) ? "3D" : "2D";
                }
                else if (cell.alloc->view)
                {
                    if      (dynamic_cast<HAL::StructuredBufferViewBase*>(cell.alloc->view.get())) type_str = "Struct";
                    else if (dynamic_cast<HAL::ByteBufferView*>(cell.alloc->view.get()))           type_str = "Byte";
                    else if (dynamic_cast<HAL::CounterView*>(cell.alloc->view.get()))              type_str = "Counter";
                    else if (dynamic_cast<HAL::RTXSceneView*>(cell.alloc->view.get()))             type_str = "BVH";
                    else                                                                            type_str = "Buffer";
                }
                break;
            }
            if (!type_str.empty())
            {
                auto tlbl            = make_label(LABEL_W - 4.0f, 14.0f, type_str, C_TEXT_DIM, 9.0f);
                tlbl->pos            = { 2.0f, row_top + ROW_H - 15.0f };
                tlbl->magnet_text    = FW1_RIGHT | FW1_VCENTER | FW1_NOWORDWRAP;
                m_left_panel->add_child(tlbl);
                m_resource_labels.push_back(tlbl);
            }
        }

        // Preview images for all cells — read cells reuse the last write's alloc+call_id.
        auto* passes_ptr = &m_passes;
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float cy = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f;

            FrameGraph::ResourceAllocInfo* last_write_alloc      = nullptr;
            UINT                           last_write_call_id    = 0;
            std::shared_ptr<Texture>       last_write_thumb_tex;

            for (auto& cell : m_resources[ri].cells)
            {
                if (cell.disabled) continue;

                if (cell.is_write)
                {
                    last_write_alloc   = cell.alloc;
                    last_write_call_id = cell.call_id;

                    if (!cell.is_deleted && cell.alloc->resource && cell.alloc->resource->get_desc().is_texture())
                    {
                        const bool is_cube = cell.alloc->view &&
                            !!dynamic_cast<HAL::CubeView*>(cell.alloc->view.get());
                        // 3x2 face grid → 42x42 per face; 2D uses 128x64.
                        const uint2 thumb_dim = is_cube ? uint2{ 126, 84 } : uint2{ 128, 64 };
                        HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(
                            HAL::Format::R8G8B8A8_UNORM, { thumb_dim }, 1, 1,
                            HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
                        last_write_thumb_tex = std::make_shared<Texture>(RenderSystem::get().device(), desc);
                        cell.thumb_tex       = last_write_thumb_tex;
                    }
                    else
                    {
                        last_write_thumb_tex = nullptr;
                    }
                }

                // Skip read cells that have no prior write in this track.
                if (!cell.is_write && !last_write_alloc) continue;
                // Resource is being freed at this pass — data is garbage, skip preview.
                if (cell.is_deleted) continue;

                auto img         = std::make_shared<preview_image>();
                img->docking     = GUI::dock::NONE;
                img->width_size  = GUI::size_type::FIXED;
                img->height_size = GUI::size_type::FIXED;
                img->size        = { 0.0f, CELL_H };
                img->pos         = { 0.0f, cy };

                if (last_write_thumb_tex)
                {
                    img->texture.texture = last_write_thumb_tex->texture_2d();
                    if (!cell.is_write)
                        img->texture.mul_color = { 1.0f, 1.0f, 1.0f, 0.7f };
                }

                // Read cells open the preview from the last write pass.
                FrameGraph::ResourceAllocInfo* src_alloc   = cell.is_write ? cell.alloc      : last_write_alloc;
                UINT                           src_call_id = cell.is_write ? cell.call_id    : last_write_call_id;

                img->on_click = [this,
                                  ri         = ri,
                                  cid        = cell.call_id,
                                  src_cid    = src_call_id,
                                  alloc      = src_alloc,
                                  passes_ptr]()
                {
                    m_sel_call_id = cid;
                    m_sel_ri      = ri;

                    std::string pass_name_str;
                    for (auto& p : *passes_ptr)
                        if (p.call_id == src_cid) { pass_name_str = to_str(p.name); break; }

                    m_preview_panel->remove_all();
                    m_preview_panel->add_child(
                        std::make_shared<ResourcePreviewContent>(alloc, src_cid, pass_name_str));

                    m_barriers_panel->remove_all();
                    m_barriers_panel->add_child(
                        std::make_shared<ResourceBarrierContent>(m_resources[ri], m_passes, cid));
                };

                m_scroll->add_child(img);
                cell.preview = img;
            }
        }

        // Read-state labels.
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float cy = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f;
            for (auto& cell : m_resources[ri].cells)
            {
                if (cell.state_str.empty()) continue;
                auto lbl = make_label(0.0f, CELL_H, cell.state_str, C_TEXT_BRIGHT, LABEL_FONT);
                lbl->magnet_text = FW1_CENTER | FW1_BOTTOM | FW1_NOWORDWRAP;
                lbl->pos = { 0.0f, cy };
                m_scroll->add_child(lbl);
                cell.state_lbl = lbl;
            }
        }

        // process_debug_resource handlers — write cells only.
        // Read cells already received thumb_tex at construction time.
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            auto& tr       = m_resources[ri];
            auto  icon_img = m_resource_icons[ri];

            for (auto& cell : tr.cells)
            {
                if (!cell.preview || !cell.alloc || !cell.is_write || !cell.thumb_tex) continue;

                UINT  capture_call_id = cell.call_id;
                auto* info            = cell.alloc;
                auto  thumb_tex       = cell.thumb_tex;

                auto cam = std::make_shared<third_person_camera>();
                cell.debug_handler = info->process_debug_resource.register_handler(this,
                    [this, capture_call_id, info, cam, thumb_tex, icon_img,
                     icon_done = false]
                    (FrameGraph::Pass* pass, FrameGraph::FrameContext* context) mutable
                    {
                        if (pass->call_id != capture_call_id) return;

                        if (!icon_done)
                        {
                            icon_done         = true;
                            const bool is_tex = info->resource->get_desc().is_texture();
                            const bool is_3d  = is_tex &&
                                info->resource->get_desc().as_texture().Dimensions.z > 1;
                            run_on_ui([icon_img, is_tex, is_3d]()
                            {
                                // TODO: assign icon_img->texture.texture to the appropriate icon.
                                (void)icon_img; (void)is_tex; (void)is_3d;
                            });
                        }

                        uint2 preview_size = uint2(thumb_tex->get_desc().as_texture().Dimensions.xy);

                        auto& compute = context->get_list()->get_compute();
                        {
                            Slots::FrameGraph_Debug_Common common;
                            common.GetTarget()      = thumb_tex->texture_2d().rwTexture2D;
                            common.GetTargetSize()  = preview_size;
                            common.GetSelectedMip() = 0;
                            compute.set(common);
                        }

                        if (auto* source = dynamic_cast<HAL::Texture2DView*>(info->view.get()))
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
                            Slots::FrameGraph_Debug_Texture2D tex2d;
                            uint2  src_dim   = info->resource->get_desc().as_texture().Dimensions.xy;
                            float2 src_size  = float2(src_dim);
                            float2 dst_size  = float2(preview_size);
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
                                float(preview_size.x) / float(preview_size.y), 1.0f, 1500.0f);
                            cam->frame_move(0.1f);
                            cam->update();
                            Slots::FrameGraph_Debug_Texture3D tex3d;
                            tex3d.GetSource()     = *source;
                            tex3d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions;
                            tex3d.GetCamera()     = cam->camera_cb.current;
                            compute.set(tex3d);
                        }
                        else if (auto* source = dynamic_cast<HAL::CubeView*>(info->view.get()))
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_TextureCube>();
                            Slots::FrameGraph_Debug_TextureCube cube;
                            cube.GetSource()     = source->textureCube;
                            cube.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions.xy;
                            compute.set(cube);
                        }
                        else
                        {
                            compute.set_pipeline<PSOS::FrameGraph_Debug_NotImplemented>();
                        }
                        compute.dispatch(uint3(preview_size, 1));
                    });
            }
        }

        // Pass-name labels in top-panel overlay (render on top of lane backgrounds).
        for (auto& pl : m_pass_labels)
        {
            m_top_panel->remove_child(pl.lbl);
            m_top_panel->remove_child(pl.btn);
        }
        m_pass_labels.clear();

        for (auto& pass : m_passes)
        {
            float py = (pass.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f) + PAD + 2.0f;
            auto lbl = make_label(0.0f, PASS_H - 2.0f * PAD - 4.0f,
                                  to_str(pass.name), pass.disabled ? C_TEXT_DIM : C_TEXT_BRIGHT, LABEL_FONT);
            lbl->pos = { 0.0f, py };
            m_top_panel->add_child(lbl);

            auto btn = std::make_shared<pass_button>();
            btn->pos = { 0.0f, (pass.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f) + PAD };
            m_top_panel->add_child(btn);

            PassInfo captured = pass;
            btn->on_click = [this, captured]()
            {
                m_sel_call_id = captured.call_id;
                m_sel_ri      = -1;
                m_sel_wait_call_ids.clear();
                for (auto& [type, dep_id] : captured.cross_queue_deps)
                    m_sel_wait_call_ids.push_back(dep_id);

                m_preview_panel->remove_all();
                m_preview_panel->add_child(
                    std::make_shared<PassInfoContent>(captured, m_passes));
            };

            m_pass_labels.push_back({ lbl, btn, pass.call_id, pass.queue });
        }

        m_ready = true;
        apply_zoom();

        run_on_ui([this]()
        {
            add_child(m_scroll);
            apply_pan();
        });
    }

    // -----------------------------------------------------------------------
    //  apply_pan — move scroll container offsets, no child iteration.
    // -----------------------------------------------------------------------
    void apply_pan()
    {
        const vec2 cp     = m_scroll->contents->pos.get();
        // Panels are direct children of content_scroll (not contents), so their pos
        // is relative to the scroll container viewport — no counter-offset needed.
        m_left_panel->pos = { 0.0f, cp.y  };   // pin X=0, follow Y
        m_top_panel->pos  = { cp.x, 0.0f  };   // follow X, pin Y=0
        m_corner->pos     = { 0.0f, 0.0f  };   // always top-left
    }

    // -----------------------------------------------------------------------
    //  apply_zoom — update content sizes and X-positions of column children.
    // -----------------------------------------------------------------------
    void apply_zoom()
    {
        float cw        = col_w();
        float content_h = LANE_COUNT * LANE_H + 6.0f + (float)m_resources.size() * ROW_H;
        float content_w = LABEL_W + (m_max_call_id + 2) * cw;

        m_left_panel->size       = { LABEL_W, content_h };
        m_top_panel->size        = { content_w, LANE_COUNT * LANE_H };
        m_scroll->contents->size = { content_w, content_h };
        m_grid_canvas->size      = { content_w, content_h };

        float block_w = cw - 2.0f * PAD - 4.0f;
        float block_h = PASS_H - 2.0f * PAD - 4.0f;
        float cell_cw = cw - 2.0f * PAD;

        for (auto& pl : m_pass_labels)
        {
            float lane_off = (pl.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f);
            float col_x    = LABEL_W + pl.call_id * cw;
            pl.lbl->pos  = { col_x + PAD + 2.0f, lane_off + PAD + 2.0f };
            pl.lbl->size = { block_w, block_h };
            pl.btn->pos  = { col_x + PAD, lane_off + PAD };
            pl.btn->size = { cw - 2.0f * PAD, PASS_H - 2.0f * PAD };
        }

        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float cy = LANE_COUNT * LANE_H + 6.0f + ri * ROW_H + (ROW_H - CELL_H) * 0.5f;
            for (auto& cell : m_resources[ri].cells)
            {
                float cx = LABEL_W + cell.call_id * cw + PAD;
                if (cell.preview)
                {
                    cell.preview->pos  = { cx, cy };
                    cell.preview->size = { cell_cw, CELL_H };
                }
                if (cell.state_lbl)
                {
                    cell.state_lbl->pos  = { cx, cy };
                    cell.state_lbl->size = { cell_cw, CELL_H };
                }
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
    //  Bezier spline batch — flushes pending rects and draws bezier curves
    //  using the CanvasLines PSO.  Must be called AFTER all dr() calls.
    // -----------------------------------------------------------------------
    void draw_splines(Context& c,
                      const std::vector<std::pair<float2, float2>>& curves,
                      const float4& color)
    {
        if (curves.empty()) return;

        vec2 screen_sz = user_ui->size.get();

        c.renderer->flush(c);

        Slots::FlowGraph graph_data;
        graph_data.GetSize()        = vec4(float2(get_render_bounds().w, get_render_bounds().h), screen_sz);
        graph_data.GetOffset_size() = { 0.0f, 0.0f, 1.0f, 0.0f };
        graph_data.GetInv_pixel()   = vec2(1.0f, 1.0f) / screen_sz;
        c.command_list->get_graphics().set(graph_data);

        std::vector<::Table::VSLine> verts;
        verts.reserve(curves.size() * 4);

        for (auto& [from, to] : curves)
        {
            float2 p1 = from / screen_sz;
            float2 p4 = to   / screen_sz;
            float  mx = (p1.x + p4.x) * 0.5f;
            float2 p2 = { p1.x+50.0f/ screen_sz.x, p1.y };
            float2 p3 = {  p4.x-50.0f/ screen_sz.x, p4.y };

            verts.push_back({ p1, color });
            verts.push_back({ p2, color });
            verts.push_back({ p3, color });
            verts.push_back({ p4, color });
        }

        c.command_list->get_graphics().set_pipeline<PSOS::CanvasLines>();
        c.command_list->get_graphics().set_topology(
            HAL::PrimitiveTopologyType::PATCH, HAL::PrimitiveTopologyFeed::LIST, false, 4);

        auto data = c.command_list->place_data(sizeof(::Table::VSLine) * verts.size(),
                                               sizeof(::Table::VSLine));
        c.command_list->write<::Table::VSLine>(data, verts);

        auto view = data.resource->create_view<HAL::StructuredBufferView<::Table::VSLine>>(
            *c.command_list,
            StructuredBufferViewDesc{ (UINT)data.resource_offset, (UINT)data.size,
                                      counterType::NONE });

        Slots::LineRender linedata;
        linedata.GetVb() = view;
        c.command_list->get_graphics().set(linedata);

        c.command_list->get_graphics().draw((int)verts.size(), 0);
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

        // Selected and waited-for pass column highlights (drawn first, behind everything).
        for (UINT wid : m_sel_wait_call_ids)
            dr(c, C_SEL_WAIT_PASS, dep_x(wid, bx), by, col_w(), bh);
        if (m_sel_call_id != std::numeric_limits<UINT>::max())
            dr(c, C_SEL_PASS, dep_x(m_sel_call_id, bx), by, col_w(), bh);

        // vertical grid lines
        for (UINT cid = 0; cid <= m_max_call_id + 1; cid++)
            dr(c, C_GRID, dep_x(cid, bx), by, 1.0f, bh);

        // resource tracks
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            auto& tr = m_resources[ri];
            float ry = row_y(ri, by);
            float cy = ry + (ROW_H - CELL_H) * 0.5f;

            // Mismatch indicator: red stripe at the top of this resource row.
            if (tr.has_mismatch)
                dr(c, C_MISMATCH, bx, ry, b.w, 2.0f);

            // lifetime spine
            {
                constexpr UINT NONE = std::numeric_limits<UINT>::max();
                UINT id_start   = NONE;
                UINT id_last    = NONE;
                UINT id_deleted = NONE;

                for (auto& cell : tr.cells)
                {
                    if (cell.disabled) continue;
                    const bool del_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                    if (!del_only) {
                        if (id_start == NONE) id_start = cell.call_id;
                        if (cell.is_created && cell.call_id < id_start) id_start = cell.call_id;
                        id_last = cell.call_id;
                    }
                    if (cell.is_deleted) id_deleted = cell.call_id;
                }

                if (id_start != NONE)
                {
                    float lx, rx;
                    if (tr.is_static || tr.is_passed)
                    {
                        lx = dep_x(0, bx);
                        rx = dep_x(m_max_call_id, bx) + col_w();
                    }
                    else
                    {
                        lx = dep_x(id_start,  bx) + col_w() * 0.5f;
                        rx = dep_x(id_last,   bx) + col_w() * 0.5f;
                        if (id_deleted != NONE && id_deleted > id_start)
                            rx = std::max(rx, dep_x(id_deleted, bx) + PAD + 2.5f);
                        if (tr.is_non_deleted)
                            rx = dep_x(m_max_call_id, bx) + col_w();
                    }
                    float mid = cy + CELL_H * 0.5f;
                    if (rx > lx)
                        dr(c, C_LIFETIME, lx, mid - 1.0f, rx - lx, 2.0f);
                    if (tr.is_non_deleted)
                        dr(c, C_DELETED, rx - 5.0f, cy, 5.0f, CELL_H);
                }
            }

            for (auto& cell : tr.cells)
            {
                float cx  = dep_x(cell.call_id, bx);
                float cw  = col_w() - 2.0f * PAD;
                const bool deletion_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                if (!deletion_only)
                {
                    const float4& cc = cell.disabled
                        ? (cell.is_write ? C_WRITE_DISABLED : C_READ_DISABLED)
                        : (cell.is_write ? C_WRITE : C_READ);
                    dr(c, cc, cx + PAD, cy, cw, CELL_H);
                }
                if (!cell.disabled)
                {
                    if (cell.has_barrier)
                        dr(c, C_BARRIER, cx + PAD - 2.5f, cy - 2.0f, 3.0f, CELL_H + 4.0f);
                    if (cell.is_created && cell.call_id != 0)
                        dr(c, C_CREATED, cx + PAD, cy, 5.0f, CELL_H);
                    if (cell.is_deleted)
                        dr(c, C_DELETED, cx + PAD, cy, 5.0f, CELL_H);
                }
            }
        }

    }

};

// ---------------------------------------------------------------------------
//  Colour table
// ---------------------------------------------------------------------------
const float4 FrameGraphTimelineCanvas::C_DIRECT_LANE  = { 0.13f, 0.13f, 0.22f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_COMPUTE_LANE = { 0.09f, 0.16f, 0.16f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_PASS_DIRECT    = { 0.28f, 0.50f, 0.82f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_PASS_COMPUTE   = { 0.20f, 0.68f, 0.46f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_PASS_DISABLED  = { 0.26f, 0.26f, 0.30f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_RUNS_ALONE     = { 1.00f, 0.40f, 0.08f, 0.95f };
const float4 FrameGraphTimelineCanvas::C_WRITE          = { 0.78f, 0.28f, 0.16f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_READ           = { 0.20f, 0.50f, 0.80f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_WRITE_DISABLED = { 0.38f, 0.18f, 0.12f, 0.7f };
const float4 FrameGraphTimelineCanvas::C_READ_DISABLED  = { 0.12f, 0.26f, 0.40f, 0.7f };
const float4 FrameGraphTimelineCanvas::C_BARRIER      = { 0.95f, 0.82f, 0.08f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_CREATED      = { 0.18f, 0.84f, 0.18f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_DELETED      = { 1.00f, 1.00f, 1.00f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_FENCE        = { 0.95f, 0.72f, 0.06f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_FENCE_LINK   = { 0.95f, 0.72f, 0.06f, 0.9f };
const float4 FrameGraphTimelineCanvas::C_LABEL_BG     = { 0.07f, 0.07f, 0.09f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_ROW_EVEN     = { 0.12f, 0.12f, 0.14f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_ROW_ODD      = { 0.09f, 0.09f, 0.11f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_GRID         = { 0.24f, 0.24f, 0.30f, 0.6f };
const float4 FrameGraphTimelineCanvas::C_SEPARATOR    = { 0.32f, 0.32f, 0.38f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_LIFETIME     = { 0.55f, 0.55f, 0.60f, 0.6f };
const float4 FrameGraphTimelineCanvas::C_TEXT_BRIGHT  = { 0.90f, 0.90f, 0.92f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_TEXT_DIM     = { 0.55f, 0.55f, 0.58f, 1.0f };
const float4 FrameGraphTimelineCanvas::C_SEL_PASS      = { 1.00f, 0.82f, 0.16f, 0.20f };
const float4 FrameGraphTimelineCanvas::C_SEL_RESOURCE  = { 1.00f, 0.82f, 0.16f, 0.12f };
const float4 FrameGraphTimelineCanvas::C_SEL_WAIT_PASS = { 0.40f, 0.72f, 1.00f, 0.15f };
const float4 FrameGraphTimelineCanvas::C_MISMATCH      = { 0.95f, 0.20f, 0.15f, 1.0f };

// ---------------------------------------------------------------------------
//  Factory
// ---------------------------------------------------------------------------
GUI::base::ptr FrameGraphDebug::create_timeline_layout(FrameGraph::Graph& graph)
{
    return std::make_shared<FrameGraphTimelineCanvas>(graph);
}
