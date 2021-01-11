#include "pch.h"
#include "Image.h"
#include "../Renderer/Renderer.h"

namespace GUI
{
    namespace Elements
    {
        void image::draw(DX12::context& c)
        {
            renderer->draw(c, texture, get_render_bounds());
        }
    }
}