module GUI:ToggleSwitch;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        toggle_switch::toggle_switch()
            : checked([this](const bool& v){ on_toggle(v); })
        {
            clickable    = true;
            width_size   = size_type::FIXED;
            height_size  = size_type::FIXED;
            size         = {44, 24};
            thinkable    = true;
        }

        void toggle_switch::think(float dt)
        {
            float target = checked.get() ? 1.0f : 0.0f;
            float delta  = target - thumb_t;

            if (delta != 0.0f)
            {
                float step = anim_speed * dt;
                if (step >= std::abs(delta))
                    thumb_t = target;
                else
                    thumb_t += (delta > 0.0f ? step : -step);
            }
        }

        void toggle_switch::draw(Context& c)
        {
            if (!initialized)
            {
                thumb_t     = checked.get() ? 1.0f : 0.0f;
                initialized = true;
            }

            rect b = get_render_bounds();
            float s = c.scale;

            // track: option_normal stretched, tinted off→on color
            GUI::Texture track_tex = Skin::get().DefaultOptionBox.Normal;
            track_tex.mul_color = {
                color_off.x + (color_on.x - color_off.x) * thumb_t,
                color_off.y + (color_on.y - color_off.y) * thumb_t,
                color_off.z + (color_on.z - color_off.z) * thumb_t,
                1.0f
            };
            track_tex.padding = {9,9,9,9};
            c.renderer->draw(c, track_tex, b);

            // thumb: option_checked (round filled circle), sliding left↔right
            float pad      = 2.0f * s;
            float thumb_sz = b.size.y - pad * 2.0f;
            float travel   = b.size.x - thumb_sz - pad * 2.0f;

            rect thumb;
            thumb.pos.x  = b.pos.x + pad + thumb_t * travel;
            thumb.pos.y  = b.pos.y + pad;
            thumb.size.x = thumb_sz;
            thumb.size.y = thumb_sz;
            c.renderer->draw(c, Skin::get().DefaultOptionBox.Checked, thumb);
        }

        bool toggle_switch::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            if (button == mouse_button::LEFT && action == mouse_action::UP)
            {
                if (allow_uncheck || !checked.get())
                    checked = !checked.get();
            }
            return true;
        }
    }
}
