module FrameGraphDebug;

import windows;
import Core;
import HAL;
import GUI;
import FrameGraph;

using namespace GUI::Elements;

// ---------------------------------------------------------------------------
//  FrameGraphTimelineCanvas
//
//  A video-editor–style timeline for the FrameGraph debugger.
//
//  Layout (top → bottom):
//    ┌─ DIRECT queue swim lane  (pass blocks at their dependency_level column)
//    ├─ COMPUTE queue swim lane
//    ├──── separator ─────────────────────────────────────────────
//    ├─ Resource row 0   [READ][WRITE][READ] …
//    ├─ Resource row 1   …
//    └─ …
//
//  X-axis  = dependency_level * column_width
//  Left column (LABEL_W pixels wide) shows lane / resource names and is
//  always visible (drawn last so it overlaps the scrolled content).
//
//  Right-drag  → pan (X and Y)
//  Scroll      → horizontal zoom anchored to cursor
// ---------------------------------------------------------------------------

class FrameGraphTimelineCanvas : public GUI::base
{
    // -----------------------------------------------------------------------
    //  Layout constants
    // -----------------------------------------------------------------------
    static constexpr float LABEL_W      = 162.0f;
    static constexpr float LANE_H       = 40.0f;
    static constexpr float LANE_COUNT   = 2.0f;
    static constexpr float ROW_H        = 26.0f;
    static constexpr float COL_W_BASE   = 140.0f;
    static constexpr float PASS_H       = 32.0f;
    static constexpr float CELL_H       = 16.0f;
    static constexpr float PAD          = 3.0f;
    static constexpr float LABEL_FONT   = 10.0f;

    // -----------------------------------------------------------------------
    //  Data model (rebuilt on every on_compile)
    // -----------------------------------------------------------------------
    struct PassInfo
    {
        std::wstring          name;
        UINT                  call_id    = 0;
        HAL::CommandListType  queue      = HAL::CommandListType::DIRECT;
        bool                  put_fence  = false;
        FrameGraph::Pass*     wait_pass  = nullptr;
    };

    struct ResourceCell
    {
        UINT  call_id     = 0;
        bool  is_write    = false;
        bool  has_barrier = false;
        bool  is_created  = false;
        bool  is_deleted  = false;
    };

    struct ResourceTrack
    {
        std::string               name;
        std::vector<ResourceCell> cells;
    };

    FrameGraph::Graph&          graph;
    std::vector<PassInfo>       m_passes;
    std::vector<ResourceTrack>  m_resources;
    UINT                        m_max_call_id = 0;
    bool                        m_ready    = false;
    bool                        m_compiled = false;

    // -----------------------------------------------------------------------
    //  View state
    // -----------------------------------------------------------------------
    vec2  m_offset       = { 0.0f, 0.0f };
    float m_zoom         = 1.0f;

    bool  m_dragging     = false;
    vec2  m_drag_start;
    vec2  m_offset_at_drag;

    // -----------------------------------------------------------------------
    //  Label children  (positioned dynamically on pan/zoom)
    // -----------------------------------------------------------------------
    label::ptr              m_lbl_direct;
    label::ptr              m_lbl_compute;
    std::vector<label::ptr> m_resource_labels;

    struct PassLabel
    {
        label::ptr            lbl;
        UINT                  call_id;
        HAL::CommandListType  queue;
    };
    std::vector<PassLabel>  m_pass_labels;

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

    // -----------------------------------------------------------------------
    //  Helpers – create a label with common settings
    // -----------------------------------------------------------------------
    static label::ptr make_label(float w, float h, const std::string& text,
                                  float4 color, float font_size)
    {
        auto lbl          = std::make_shared<label>();
        lbl->text         = text;
        lbl->color        = color;
        lbl->font_size    = font_size;
        lbl->docking      = GUI::dock::NONE;
        lbl->width_size   = GUI::size_type::FIXED;
        lbl->height_size  = GUI::size_type::FIXED;
        lbl->size         = { w, h };
        lbl->clip_to_parent = GUI::ParentClip::ALL;
        return lbl;
    }

    // narrow-wstring → std::string  (ASCII / Latin-1 pass names are safe)
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
        clip_child  = true;

        // -- fixed queue-lane labels ------------------------------------------
        m_lbl_direct  = make_label(LABEL_W - 8.0f, LANE_H - 6.0f,
                                    "DIRECT",  C_TEXT_BRIGHT, LABEL_FONT);
        m_lbl_compute = make_label(LABEL_W - 8.0f, LANE_H - 6.0f,
                                    "COMPUTE", C_TEXT_BRIGHT, LABEL_FONT);
        add_child(m_lbl_direct);
        add_child(m_lbl_compute);

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

        m_passes.clear();
        m_resources.clear();
        m_max_call_id = 0;

        // passes
        for (auto* pass : g.builder.enabled_passes)
        {
            PassInfo info;
            info.name       = pass->name;
            info.call_id    = pass->call_id;
            info.queue      = pass->get_type();
            info.put_fence  = pass->put_fence;
            info.wait_pass  = pass->wait_pass;
            m_passes.push_back(info);
            if (pass->call_id > m_max_call_id)
                m_max_call_id = pass->call_id;
        }

        // resource tracks
        std::map<std::string, ResourceTrack> track_map;
        for (auto* pass : g.builder.enabled_passes)
        {
            // primary cells – resources actively used by this pass
            for (auto& [alloc, flags] : pass->used.resource_flags)
            {
                auto& tr  = track_map[alloc->name];
                tr.name   = alloc->name;

                ResourceCell cell;
                cell.call_id    = pass->call_id;
                cell.is_write   = check(flags & FrameGraph::WRITEABLE_FLAGS);
                cell.is_created = (pass->used.resource_creations.count(alloc) > 0);
                cell.is_deleted = (pass->used.resource_deletions_after.count(alloc) > 0);
                tr.cells.push_back(cell);
            }

            // deletion-only cells – resources freed after this pass but not
            // in resource_flags (the pass is merely the synchronization point)
            for (auto* alloc : pass->used.resource_deletions_before)
            {
                if (pass->used.resource_flags.count(alloc)) continue; // already handled above

                auto& tr  = track_map[alloc->name];
                tr.name   = alloc->name;

                ResourceCell cell;
                cell.call_id    = pass->call_id;
                cell.is_write   = false;
                cell.is_created = false;
                cell.is_deleted = true;
                tr.cells.push_back(cell);
            }
        }
        for (auto& [name, tr] : track_map)
        {
            // enabled_passes is already sorted by dependency_level,
            // so cells are naturally in order — no sort needed.
            for (size_t i = 1; i < tr.cells.size(); ++i)
                if (tr.cells[i].is_write != tr.cells[i - 1].is_write)
                    tr.cells[i].has_barrier = true;
            m_resources.push_back(std::move(tr));
        }

        // ---- rebuild resource-name labels -----------------------------------
        for (auto& lbl : m_resource_labels) remove_child(lbl);
        m_resource_labels.clear();

        for (auto& tr : m_resources)
        {
            auto lbl = make_label(LABEL_W - 8.0f, ROW_H - 4.0f,
                                   tr.name, C_TEXT_BRIGHT, LABEL_FONT);
            add_child(lbl);
            m_resource_labels.push_back(lbl);
        }

        // ---- rebuild pass-name labels inside swim-lane blocks ---------------
        for (auto& pl : m_pass_labels) remove_child(pl.lbl);
        m_pass_labels.clear();

        for (auto& pass : m_passes)
        {
            float block_w = col_w() - 2.0f * PAD - 4.0f;
            float block_h = PASS_H - 2.0f * PAD - 4.0f;
            auto lbl = make_label(block_w, block_h,
                                   to_str(pass.name), C_TEXT_BRIGHT, LABEL_FONT);
            add_child(lbl);
            m_pass_labels.push_back({ lbl, pass.call_id, pass.queue });
        }

        m_ready = true;
        update_label_positions();
    }

    // -----------------------------------------------------------------------
    //  Reposition all label children to match current offset / zoom
    //  Call after every pan/zoom change so the layout system re-places them.
    // -----------------------------------------------------------------------
    void update_label_positions()
    {
        // Queue lane labels (left column)
        if (m_lbl_direct)
            m_lbl_direct->pos = { 4.0f, m_offset.y + (LANE_H - 14.0f) * 0.5f };
        if (m_lbl_compute)
            m_lbl_compute->pos = { 4.0f, m_offset.y + LANE_H + (LANE_H - 14.0f) * 0.5f };

        // Resource name labels (left column)
        for (int ri = 0; ri < (int)m_resource_labels.size(); ri++)
        {
            float ry = m_offset.y + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
            m_resource_labels[ri]->pos = { 4.0f, ry + (ROW_H - 14.0f) * 0.5f };
        }

        // Pass-name labels (inside swim-lane blocks)
        float cw = col_w();
        for (auto& pl : m_pass_labels)
        {
            float px = LABEL_W + pl.call_id * cw + m_offset.x + PAD + 2.0f;
            float py = m_offset.y
                       + (pl.queue == HAL::CommandListType::COMPUTE ? LANE_H : 0.0f)
                       + PAD + 2.0f;
            float block_w = cw - 2.0f * PAD - 4.0f;
            pl.lbl->pos  = { px, py };
            pl.lbl->size = { block_w, PASS_H - 2.0f * PAD - 4.0f };
        }
    }

    // -----------------------------------------------------------------------
    //  Coordinate helpers  (return absolute screen coords)
    // -----------------------------------------------------------------------
    float col_w() const { return COL_W_BASE * m_zoom; }

    float dep_x(UINT call_id, float canvas_x) const
    {
        return canvas_x + LABEL_W + call_id * col_w() + m_offset.x;
    }

    float lane_y(HAL::CommandListType queue, float canvas_y) const
    {
        float y = canvas_y + m_offset.y;
        if (queue == HAL::CommandListType::COMPUTE) y += LANE_H;
        return y;
    }

    float row_y(int ri, float canvas_y) const
    {
        return canvas_y + m_offset.y + LANE_COUNT * LANE_H + 6.0f + ri * ROW_H;
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

public:
    // -----------------------------------------------------------------------
    //  draw()
    // -----------------------------------------------------------------------
    virtual void draw(Context& c) override
    {
        if (!m_ready) return;

        const rect b   = get_render_bounds();
        const float bx = b.x, by = b.y, bw = b.w, bh = b.h;

        // queue lane backgrounds (full width)
        dr(c, C_DIRECT_LANE,  bx, by + m_offset.y,          bw, LANE_H);
        dr(c, C_COMPUTE_LANE, bx, by + m_offset.y + LANE_H, bw, LANE_H);

        // separator line
        dr(c, C_SEPARATOR, bx, by + m_offset.y + LANE_COUNT * LANE_H, bw, 2.0f);

        // resource row alternating backgrounds
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float ry = row_y(ri, by);
            if (ry + ROW_H < by || ry > by + bh) continue;
            dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN,
               bx + LABEL_W, ry, bw - LABEL_W, ROW_H);
        }

        // vertical grid lines
        for (UINT cid = 0; cid <= m_max_call_id + 1; cid++)
        {
            float gx = dep_x(cid, bx);
            if (gx < bx + LABEL_W || gx > bx + bw) continue;
            dr(c, C_GRID, gx, by, 1.0f, bh);
        }

        // pass blocks
        for (auto& pass : m_passes)
        {
            float px = dep_x(pass.call_id, bx);
            float py = lane_y(pass.queue, by);
            float pw = col_w() - 2.0f * PAD;
            const float4& pc = (pass.queue == HAL::CommandListType::DIRECT)
                                 ? C_PASS_DIRECT : C_PASS_COMPUTE;
            dr(c, pc, px + PAD, py + PAD, pw, PASS_H - 2.0f * PAD);

            if (pass.put_fence)
            {
                float fx = px + col_w() - PAD - 1.5f;
                dr(c, C_FENCE, fx, by + m_offset.y, 3.0f, LANE_COUNT * LANE_H);
            }
        }

        // wait_pass dependency connectors
        for (auto& pass : m_passes)
        {
            if (!pass.wait_pass) continue;
            float from_cx = dep_x(pass.wait_pass->call_id, bx) + col_w() * 0.5f;
            float to_cx   = dep_x(pass.call_id,            bx) + col_w() * 0.5f;
            float from_cy = lane_y(pass.wait_pass->get_type(), by) + LANE_H * 0.5f;
            float to_cy   = lane_y(pass.queue,                 by) + LANE_H * 0.5f;
            float min_x = std::min(from_cx, to_cx);
            dr(c, C_FENCE_LINK, min_x, from_cy - 1.0f, std::abs(to_cx - from_cx), 2.0f);
            float min_y = std::min(from_cy, to_cy);
            dr(c, C_FENCE_LINK, to_cx - 1.0f, min_y, 2.0f, std::abs(to_cy - from_cy));
        }

        // resource cells
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            auto& tr = m_resources[ri];
            float ry = row_y(ri, by);
            float cy = ry + (ROW_H - CELL_H) * 0.5f;
            if (ry + ROW_H < by || ry > by + bh) continue;

            // lifetime spine: from first usage to the deletion marker
            {
                constexpr UINT NONE = std::numeric_limits<UINT>::max();
                UINT id_start   = NONE;   // first real usage (non-deletion-only)
                UINT id_deleted = NONE;   // call_id of the cell carrying is_deleted

                for (auto& cell : tr.cells)
                {
                    const bool del_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                    if (!del_only)
                    {
                        if (id_start == NONE) id_start = cell.call_id;
                        // creation can pull start further left
                        if (cell.is_created && cell.call_id < id_start)
                            id_start = cell.call_id;
                    }
                    if (cell.is_deleted)
                        id_deleted = cell.call_id;
                }

                if (id_start != NONE)
                {
                    float lx  = dep_x(id_start, bx) + col_w() * 0.5f;
                    float rx;
                    if (id_deleted != NONE && id_deleted > id_start)
                        // end at the centre of the 5px deletion marker
                        rx = dep_x(id_deleted, bx) + PAD + 2.5f;
                    else if (id_deleted == NONE)
                        rx = lx; // no deletion known — nothing to draw
                    else
                        rx = lx; // deleted at or before start — nothing to draw

                    float mid = cy + CELL_H * 0.5f;
                    if (rx > lx)
                        dr(c, C_LIFETIME, lx, mid - 1.0f, rx - lx, 2.0f);
                }
            }

            for (auto& cell : tr.cells)
            {
                float cx = dep_x(cell.call_id, bx);
                float cw = col_w() - 2.0f * PAD;

                // deletion-only cells have no read/write usage in this pass —
                // draw only the marker, not a full cell background
                const bool deletion_only = cell.is_deleted && !cell.is_write && !cell.is_created;
                if (!deletion_only)
                    dr(c, cell.is_write ? C_WRITE : C_READ, cx + PAD, cy, cw, CELL_H);

                if (cell.has_barrier)
                    dr(c, C_BARRIER, cx + PAD - 2.5f, cy - 2.0f, 3.0f, CELL_H + 4.0f);

                // creation: green left accent
                if (cell.is_created)
                    dr(c, C_CREATED, cx + PAD, cy, 5.0f, CELL_H);

                // deletion: white left accent — resource is freed before this
                // pass begins, so the marker sits at the column's leading edge
                if (cell.is_deleted)
                    dr(c, C_DELETED, cx + PAD, cy, 5.0f, CELL_H);
            }
        }

        // ---- left label-column overlay (always on top) ----------------------
        dr(c, C_LABEL_BG, bx, by + m_offset.y,           LABEL_W, LANE_H);
        dr(c, C_LABEL_BG, bx, by + m_offset.y + LANE_H,  LABEL_W, LANE_H);
        for (int ri = 0; ri < (int)m_resources.size(); ri++)
        {
            float ry = row_y(ri, by);
            if (ry + ROW_H < by || ry > by + bh) continue;
            dr(c, (ri & 1) ? C_ROW_ODD : C_ROW_EVEN, bx, ry, LABEL_W, ROW_H);
        }
        dr(c, C_SEPARATOR, bx + LABEL_W - 1.0f, by, 1.0f, bh);

        base::draw(c);  // render label children on top
    }

    // -----------------------------------------------------------------------
    //  Input
    // -----------------------------------------------------------------------
    virtual bool on_mouse_action(mouse_action action, mouse_button button,
                                  vec2 pos) override
    {
        if (button == mouse_button::RIGHT)
        {
            pressed = (action == mouse_action::DOWN);
            if (pressed)
            {
                m_drag_start     = pos;
                m_offset_at_drag = m_offset;
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
            m_offset = m_offset_at_drag + (pos - m_drag_start);
            update_label_positions();
        }
        return m_dragging;
    }

    virtual bool on_wheel(mouse_wheel type, float value, vec2 wheel_pos) override
    {
        float prev_zoom = m_zoom;
        m_zoom *= 1.0f + value / 10.0f;
        m_zoom  = Math::clamp(m_zoom, 0.05f, 20.0f);

        float cx    = get_render_bounds().x;
        float pivot = wheel_pos.x - cx - LABEL_W - m_offset.x;
        m_offset.x  = wheel_pos.x - cx - LABEL_W - pivot * (m_zoom / prev_zoom);

        update_label_positions();
        return true;
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

// ---------------------------------------------------------------------------
//  Factory
// ---------------------------------------------------------------------------
GUI::base::ptr FrameGraphDebug::create_timeline_layout(FrameGraph::Graph& graph)
{
    return std::make_shared<FrameGraphTimelineCanvas>(graph);
}
