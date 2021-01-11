#pragma once
#include "Dragger.h"
namespace GUI
{
    namespace Elements
    {
        enum class direction : int
        {
            LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8,

            LEFT_TOP = LEFT | TOP, RIGHT_TOP = RIGHT | TOP,
            LEFT_BOTTOM = LEFT | BOTTOM, RIGHT_BOTTOM = RIGHT | BOTTOM
        };

        static direction to_direction(dock d)
        {
            if (d == dock::LEFT)
                return direction::LEFT;

            if (d == dock::RIGHT)
                return direction::RIGHT;

            if (d == dock::TOP)
                return direction::TOP;

            if (d == dock::BOTTOM)
                return direction::BOTTOM;

            return direction::LEFT;
        }

        static dock invert(dock d)
        {
            if (d == dock::LEFT)
                return dock::RIGHT;

            if (d == dock::RIGHT)
                return dock::LEFT;

            if (d == dock::TOP)
                return dock::BOTTOM;

            if (d == dock::BOTTOM)
                return dock::TOP;

            return dock::LEFT;
        }


        class resizer : public dragger
        {

        public:
            direction dir = direction::BOTTOM;
            using ptr = s_ptr<resizer>;
            using wptr = w_ptr<resizer>;
            resizer();
            virtual void on_mouse_enter(vec2) override;

            virtual void on_mouse_leave(vec2) override;

            virtual bool on_mouse_move(vec2 pos) override;



        };
    }
}