module GUI:Separator;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        separator::separator(orientation o)
        {
            orient  = o;
            clickable = false;

            if (o == orientation::horizontal)
            {
                docking     = dock::TOP;
                height_size = size_type::FIXED;
                size        = {0, thickness};
            }
            else
            {
                docking    = dock::LEFT;
                width_size = size_type::FIXED;
                size       = {thickness, 0};
            }
        }

        void separator::draw(Context& c)
        {
            rect b = get_render_bounds();

            if (orient == orientation::horizontal)
                b.size.y = thickness * c.scale;
            else
                b.size.x = thickness * c.scale;

            c.renderer->draw_color(c, color, b);
        }
    }
}
