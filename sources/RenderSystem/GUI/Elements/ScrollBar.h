#pragma once
#include "Dragger.h"
#include "GUI/Skin.h"

import Constants;
namespace GUI
{
    namespace Elements
    {

        class scroll_bar : public base
        {
                dragger::ptr drag;

            public:

                using ptr = s_ptr<scroll_bar>;
                using wptr = w_ptr<scroll_bar>;
                enum class scroll_type : int
                {
                    HORIZONTAL,
                    VERTICAL
                } type;
                std::function<void(float)> on_move;

                scroll_bar(scroll_type type);
                Skin::Button drag_skin;
                virtual void draw(Render::context& c) override;
                void set_sizes(float filled_size, float container_size, float container_pos)
                {
                    float f = container_size;
                    float percent_size = std::min(1.0f, filled_size / f);
                    float _offset = -container_pos / (f /*- filled_size*/);

                    if (f < Math::eps10)
                    {
                        percent_size = 1;
                        _offset = 0;
                    }

                    if (type == scroll_type::HORIZONTAL)
                    {
                        drag->size = {render_bounds->w * percent_size, drag->size->y};
                        drag->pos = { render_bounds->w * _offset, drag->pos->y };
                    }

                    else
                    {
                        drag->size = { drag->size->x, render_bounds->h  * percent_size };
                        drag->pos = { drag->pos->x, render_bounds->h  * _offset };
                    }

                    visible = percent_size < 1;
                }
        };

    }
}