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

        void progress_bar::draw(Context& c)
        {
            rect b = get_render_bounds();

            // background track
            c.renderer->draw_color(c, track_color, b);

            // filled portion — clamp value to [0,1]
            float raw = value;
            float v   = raw < 0.0f ? 0.0f : (raw > 1.0f ? 1.0f : raw);
            if (v > 0.0f)
            {
                rect fill = b;
                fill.size.x *= v;
                c.renderer->draw_color(c, fill_color, fill);
            }
        }
    }
}
