module GUI:FloatBox;

import <windows/windows.h>;

namespace
{
    std::string format_float(float v, int precision)
    {
        std::string s = std::format("{:.{}f}", v, std::max(0, precision));
        if (s.find('.') != std::string::npos)
        {
            while (s.back() == '0') s.pop_back();
            if (s.back() == '.') s.pop_back();
        }
        return s == "-0" ? "0" : s;
    }
}

namespace GUI
{
    namespace Elements
    {
        float_box::float_box()
        {
            filter = [](char32_t ch)
                {
                    return (ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '+' || ch == 'e' || ch == 'E';
                };
            height_size = size_type::FIXED;
            size = { 70, single_line_height() };
            set_text(format_float(value, precision));
        }

        std::optional<float> float_box::parse()
        {
            const std::string text = get_text();
            const char* begin = text.data();
            const char* end   = text.data() + text.size();
            if (begin != end && *begin == '+') ++begin;   // from_chars rejects a leading '+'

            float v = 0;
            const auto [ptr, ec] = std::from_chars(begin, end, v);
            if (ec != std::errc() || ptr != end || !std::isfinite(v))
                return std::nullopt;
            return v;
        }

        void float_box::apply(float v, bool notify)
        {
            v = std::clamp(v, min, max);
            const bool changed = v != value;
            value = v;
            set_text(format_float(v, precision));
            if (notify && changed)
                on_value_change(v);
        }

        void float_box::set_value(float v)
        {
            apply(v, false);
        }

        void float_box::commit()
        {
            apply(parse().value_or(value), true);
        }

        void float_box::step_by(float steps, key_mods mods)
        {
            float s = step;
            if (mods.shift) s *= 10;
            if (mods.ctrl)  s *= 0.1f;
            apply(parse().value_or(value) + steps * s, true);
        }

        void float_box::on_key_action(key_action action, long key, key_mods mods)
        {
            if (action == key_action::DOWN)
            {
                switch (key)
                {
                case VK_RETURN: commit(); return;
                case VK_ESCAPE: apply(value, false); return;
                case VK_UP:     step_by(1, mods); return;
                case VK_DOWN:   step_by(-1, mods); return;
                }
            }
            edit_text::on_key_action(action, key, mods);
        }

        bool float_box::on_wheel(mouse_wheel type, float notches, vec2 pos)
        {
            if (type != mouse_wheel::VERTICAL || !is_focused())
                return false;

            // Wheel events carry no modifiers.
            key_mods mods;
            mods.shift = GetKeyState(VK_SHIFT) < 0;
            mods.ctrl  = GetKeyState(VK_CONTROL) < 0;
            step_by(notches > 0 ? 1.0f : -1.0f, mods);
            return true;
        }

        void float_box::on_focus_changed(bool focused)
        {
            if (!focused)
                commit();
        }
    }
}
