module GUI:ProgressBar;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        progress_bar::progress_bar()
            : value([this](const float& v){ on_change(v); })
        {
            clickable   = false;
            docking     = dock::TOP;
            height_size = size_type::FIXED;
            size        = {0, 8};
        }

        void progress_bar::on_pre_render(Context& c)
        {
            base::on_pre_render(c);

            const float raw    = value;
            const float target = std::clamp(raw, 0.0f, 1.0f);

            if (!animated || first)
            {
                shown = target;
                first = false;
            }
            else
            {
                // Exponential ease: about 90% of a jump in 0.15 s, whatever the frame time.
                shown += (target - shown) * (1.0f - std::exp(-c.delta_time * 15.0f));
                if (std::abs(target - shown) < 0.0005f)
                    shown = target;
            }

            sweep = std::fmod(sweep + c.delta_time / 1.6f, 1.0f);
        }

        void progress_bar::draw(Context& c)
        {
            rect b = get_render_bounds();

            // background track
            c.renderer->draw_color(c, track_color, b);

            // filled portion, eased value in [0,1]
            const float v = animated ? shown : std::clamp(float(value), 0.0f, 1.0f);
            if (v <= 0.0f)
                return;

            rect fill = b;
            fill.size.x *= v;
            c.renderer->draw_color(c, fill_color, fill);

            if (!animated)
                return;

            // Highlight band sweeping left to right over the fill, a quarter of its
            // width (at least 24 px), faded at both ends by stacking slices.
            // Thin slices (about 2 px) so the falloff reads as a gradient, not steps.
            const float band = std::max(24.0f, fill.size.x * 0.25f);
            const float x0   = fill.pos.x - band + (fill.size.x + band) * sweep;
            const int slices = std::clamp(int(band / 2.0f), 12, 48);
            for (int i = 0; i < slices; i++)
            {
                const float t     = (i + 0.5f) / slices;                  // 0..1 across the band
                const float s     = std::sin(t * 3.14159265f);
                const float alpha = 0.28f * s * s;                        // soft at both ends
                float sx = x0 + band * i / slices;
                float ex = x0 + band * (i + 1) / slices;
                sx = std::max(sx, fill.pos.x);
                ex = std::min(ex, fill.pos.x + fill.size.x);
                if (ex <= sx)
                    continue;

                rect s = fill;
                s.pos.x  = sx;
                s.size.x = ex - sx;
                c.renderer->draw_color(c, float4(1, 1, 1, alpha), s);
            }

            // Leading-edge glow: a short falloff just outside the fill, in its colour.
            if (v < 0.999f)
            {
                const float edge   = fill.pos.x + fill.size.x;
                const float length = std::min(b.size.y * 2.0f, b.pos.x + b.size.x - edge);
                const int   steps  = std::clamp(int(length / 1.5f), 1, 24);
                for (int i = 0; i < steps && length > 0.0f; i++)
                {
                    const float t     = (i + 0.5f) / steps;
                    const float alpha = 0.35f * (1.0f - t) * (1.0f - t);
                    rect g = b;
                    g.pos.x  = edge + length * i / steps;
                    g.size.x = length / steps;
                    c.renderer->draw_color(c, float4(fill_color.x, fill_color.y, fill_color.z, alpha), g);
                }
            }
        }
    }
}
