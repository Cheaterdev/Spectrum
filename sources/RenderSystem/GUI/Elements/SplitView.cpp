module GUI:SplitView;
import :Renderer;
import :Dragger;

namespace GUI
{
    namespace Elements
    {
        // A dragger handle (look, cursor, mouse capture) whose position comes
        // from the ratio: docked, so the pos the dragger moves is ignored, and
        // the ratio is taken from the absolute mouse position -- summing deltas
        // would drift from the mouse once the ratio clamps at min_panel.
        class split_view::divider : public dragger
        {
                split_view* owner;

            public:
                explicit divider(split_view* owner) : owner(owner)
                {
                    target     = this;
                    allow_x    = owner->horizontal;
                    allow_y    = !owner->horizontal;
                }

                bool on_mouse_move(vec2 pos) override
                {
                    dragger::on_mouse_move(pos);
                    if (is_dragging())
                        owner->set_ratio_from(pos);
                    return true;
                }
        };

        split_view::split_view(bool horizontal) : horizontal(horizontal)
        {
            first_panel  = std::make_shared<base>();
            second_panel = std::make_shared<base>();
            bar          = std::make_shared<divider>(this);

            const dock edge = horizontal ? dock::LEFT : dock::TOP;
            first_panel->docking  = edge;
            bar->docking          = edge;
            second_panel->docking = dock::FILL;

            if (horizontal)
            {
                first_panel->width_size = size_type::FIXED;
                bar->width_size         = size_type::FIXED;
            }
            else
            {
                first_panel->height_size = size_type::FIXED;
                bar->height_size         = size_type::FIXED;
            }

            add_child(first_panel);
            add_child(bar);
            add_child(second_panel);
        }

        void split_view::set_ratio(float r)
        {
            ratio = std::clamp(r, 0.0f, 1.0f);
            set_update_layout();
        }

        void split_view::set_ratio_from(vec2 pos)
        {
            const rect  b     = get_render_bounds();
            const float start = horizontal ? b.x : b.y;
            const float span  = horizontal ? b.w : b.h;
            if (span <= 0) return;

            set_ratio((( horizontal ? pos.x : pos.y) - start) / span);
            if (on_ratio_change)
                on_ratio_change(ratio);
        }

        // The first panel's size follows the ratio of the space left after the
        // divider, set here -- before the docked children are placed -- so a
        // resize keeps the proportions instead of a fixed first panel.
        void split_view::update_childs_layout(sizer& r, float scale)
        {
            const float s     = result_scale * this->scale;
            const float space = (horizontal ? r.right - r.left : r.bottom - r.top) / (s > 0 ? s : 1.0f);
            const float room  = std::max(0.0f, space - divider_width);

            float first = room * ratio;
            if (room >= 2 * min_panel)
                first = std::clamp(first, min_panel, room - min_panel);

            if (horizontal)
            {
                first_panel->size = { first, first_panel->size->y };
                bar->size         = { divider_width, bar->size->y };
            }
            else
            {
                first_panel->size = { first_panel->size->x, first };
                bar->size         = { bar->size->x, divider_width };
            }

            base::update_childs_layout(r, scale);
        }
    }
}
