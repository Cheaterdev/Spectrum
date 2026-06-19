module GUI:FloatSlider;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        float_slider::float_slider()
            : value([this](const float& v){ on_change(v); })
        {
            clickable    = true;
            height_size  = size_type::FIXED;
            size         = {100, 14};
        }

        float float_slider::normalized() const
        {
            float range = max - min;
            if (range == 0.0f) return 0.0f;
            float t = ((float)value - min) / range;
            return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        }

        void float_slider::set_from_pos(float screen_x)
        {
            rect b = get_render_bounds();
            float t = (screen_x - b.pos.x) / b.size.x;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            value = min + t * (max - min);
        }

        void float_slider::draw(Context& c)
        {
            rect  b = get_render_bounds();
            float s = c.scale;

            c.renderer->draw_color(c, left_color, right_color, b);

            float thumb_cx = b.pos.x + normalized() * b.size.x;

            rect outer;
            outer.pos.x  = thumb_cx - 2.0f * s;
            outer.pos.y  = b.pos.y;
            outer.size.x = 4.0f * s;
            outer.size.y = b.size.y;
            c.renderer->draw_color(c, float4(1, 1, 1, 0.9f), outer);

            rect inner;
            inner.pos.x  = thumb_cx - s;
            inner.pos.y  = b.pos.y + s;
            inner.size.x = 2.0f * s;
            inner.size.y = b.size.y - 2.0f * s;
            c.renderer->draw_color(c, float4(0, 0, 0, 0.7f), inner);
        }

        bool float_slider::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            if (button != mouse_button::LEFT)
                return false;

            if (action == mouse_action::DOWN)
            {
                dragging = true;
                set_movable(true);
                set_from_pos(pos.x);
            }
            else
            {
                dragging = false;
                set_movable(false);
            }

            return true;
        }

        bool float_slider::on_mouse_move(vec2 pos)
        {
            if (dragging)
                set_from_pos(pos.x);
            return true;
        }
    }
}
