#include "pch.h"
#include "ColoredRect.h"
#include "../Renderer/Renderer.h"

namespace GUI
{
    namespace Elements
    {

        void colored_rect::draw(DX12::context& c)
        {
            renderer->draw_color(c, color, get_render_bounds());
        }



    }

}