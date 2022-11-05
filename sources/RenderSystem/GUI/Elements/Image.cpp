module GUI:Image;
import :Renderer;


namespace GUI
{
    namespace Elements
    {
        void image::draw(Graphics::context& c)
        {
            renderer->draw(c, texture, get_render_bounds());
        }

    }

}