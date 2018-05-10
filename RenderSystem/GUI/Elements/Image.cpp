#include "pch.h"



namespace GUI
{
    namespace Elements
    {




        void image::draw(Render::context& c)
        {
          //  if (!texture.texture || texture.texture == Render::Texture::null)
         //       return;

            // t.texture = texture;
            renderer->draw(c, texture, get_render_bounds());
            //     renderer->draw(this, c);
        }

    }

}