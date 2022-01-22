#include "pch_render.h"
#include "Image.h"
#include "GUI/Renderer/Renderer.h"


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