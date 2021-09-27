#include "pch.h"
#include "Image.h"
#include "GUI/Renderer/Renderer.h"


namespace GUI
{
    namespace Elements
    {
        void image::draw(Render::context& c)
        {
            renderer->draw(c, texture, get_render_bounds());
        }

    }

}