module GUI:StatGraph;
import :Renderer;

#include <cstdio>

namespace GUI
{
    namespace Elements
    {
        stat_graph::stat_graph()
        {
            docking     = dock::TOP;
            height_size = size_type::FIXED;
            width_size  = size_type::MATCH_PARENT;
            size        = {0, 60};

            header_label.reset(new label());
            header_label->docking     = dock::NONE;
            header_label->x_type      = pos_x_type::LEFT;
            header_label->y_type      = pos_y_type::TOP;
            header_label->pos         = {4, 2};
            header_label->width_size  = size_type::MATCH_PARENT;
            header_label->height_size = size_type::FIXED;
            header_label->size        = {0, 14};
            header_label->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            header_label->color       = {0.9f, 0.9f, 0.9f, 0.85f};
            add_child(header_label);
        }

        // --- ring-buffer helpers ---

        float stat_graph::get_sample(size_t i) const
        {
            // oldest sample is at index: (head + capacity - count) % capacity
            return buffer[(head + capacity - count + i) % capacity];
        }

        // --- public API ---

        void stat_graph::push(float v)
        {
            if (buffer.size() != capacity)
            {
                buffer.assign(capacity, 0.0f);
                head  = 0;
                count = 0;
            }
            buffer[head] = v;
            head = (head + 1) % capacity;
            if (count < capacity) ++count;
            update_label();
        }

        float stat_graph::current_value() const
        {
            if (count == 0) return 0.0f;
            return buffer[(head + capacity - 1) % capacity];
        }

        float stat_graph::average_value() const
        {
            if (count == 0) return 0.0f;
            float sum = 0.0f;
            for (size_t i = 0; i < count; ++i)
                sum += get_sample(i);
            return sum / (float)count;
        }

        float stat_graph::min_stored() const
        {
            if (count == 0) return 0.0f;
            float m = get_sample(0);
            for (size_t i = 1; i < count; ++i)
                m = std::min(m, get_sample(i));
            return m;
        }

        float stat_graph::max_stored() const
        {
            if (count == 0) return 0.0f;
            float m = get_sample(0);
            for (size_t i = 1; i < count; ++i)
                m = std::max(m, get_sample(i));
            return m;
        }

        void stat_graph::update_label()
        {
            header_label->visible = show_stats;
            if (!show_stats) return;

            char buf[128];
            float cur = current_value();
            float avg = average_value();

            const char* sep   = title.empty() ? "" : ": ";
            const char* u     = unit.empty()  ? ""  : unit.c_str();
            const char* usp   = unit.empty()  ? ""  : " ";

            snprintf(buf, sizeof(buf), "%s%s%.1f%s%s  avg %.1f%s%s",
                title.c_str(), sep,
                cur, usp, u,
                avg, usp, u);

            header_label->text = buf;
        }

        // --- draw ---

        void stat_graph::draw(Context& c)
        {
            rect b = get_render_bounds();

            // background
            c.renderer->draw_color(c, background_color, b);

            if (count < 2) return;

            // compute visible range
            float vmin, vmax;
            if (auto_scale)
            {
                vmin = vmax = get_sample(0);
                for (size_t i = 1; i < count; ++i)
                {
                    float v = get_sample(i);
                    if (v < vmin) vmin = v;
                    if (v > vmax) vmax = v;
                }
                // small padding so lines don't clip at edges
                float pad = (vmax - vmin) * 0.08f;
                if (pad < 0.01f) pad = vmax * 0.05f + 1.0f;
                vmin -= pad;
                vmax += pad;
            }
            else
            {
                vmin = fixed_min;
                vmax = fixed_max;
            }

            float range = vmax - vmin;
            if (range <= 0.0f) range = 1.0f;

            float bottom = b.pos.y + b.size.y;
            float col_w  = b.size.x / (float)count;
            float line_h = 2.0f * c.scale;

            for (size_t i = 0; i < count; ++i)
            {
                float v = get_sample(i);
                float t = (v - vmin) / range;
                t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

                float x = b.pos.x + (float)i * col_w;
                float y = bottom - t * b.size.y;
                float w = col_w + 1.0f; // slight overlap to avoid 1-pixel gaps

                // filled area from sample y to bottom
                rect fill;
                fill.pos.x  = x;
                fill.pos.y  = y;
                fill.size.x = w;
                fill.size.y = bottom - y;
                c.renderer->draw_color(c, fill_color, fill);

                // bright line at the top of the column
                rect line_r;
                line_r.pos.x  = x;
                line_r.pos.y  = y;
                line_r.size.x = w;
                line_r.size.y = line_h;
                c.renderer->draw_color(c, line_color, line_r);
            }
        }
    }
}
