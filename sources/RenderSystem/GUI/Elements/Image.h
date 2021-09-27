#pragma once
#include "GUI/GUI.h"

namespace GUI
{
    namespace Elements
    {
        class image : public base
        {
            public:
                using ptr = s_ptr<image>;
                using wptr = w_ptr<image>;

                Texture texture;
                virtual void draw(Render::context& c);
                image()
                {
                    clickable = false;
                }
        };



    }
}