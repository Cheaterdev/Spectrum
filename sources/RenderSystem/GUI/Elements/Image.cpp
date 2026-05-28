module GUI:Image;
import :Renderer;


namespace GUI
{
    namespace Elements
    {
        image::image()
        {
            clickable = false;
        }

        void image::draw(Context& c)
        {
            c.renderer->draw(c, texture, get_render_bounds());
        }

    }

}