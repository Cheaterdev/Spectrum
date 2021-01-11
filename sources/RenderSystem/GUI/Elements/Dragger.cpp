#include "pch.h"
#include "Dragger.h"

namespace GUI
{
    namespace Elements
    {
        bool dragger::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button, pos);
            auto real_target = target;

            if (!real_target)
                return false;

            if (button == mouse_button::LEFT)
            {
                dragging = action == mouse_action::DOWN;
                set_movable(dragging);

                if (dragging)
                {
                    start_pos = pos ;// real_target->to_local(pos);
                    orig_pos = target->pos;
                    orig_size = target->size;
                    orig_scale = target->result_scale;

                    if (on_start_drag)
                        on_start_drag();
                }
                else
                {
                    if (on_end_drag)
                        on_end_drag();
                }

                // oops?
                if (action == mouse_action::CANCEL)
                {
                    //  target->pos = start_pos;
                }

				return true;
            }
			return false;
        }

        bool dragger::on_mouse_move(vec2 pos)
        {
            base::on_mouse_move(pos);

            if (!dragging)
                return false;

            auto real_target = target;

            if (!real_target)
                return false;

            vec2 delta = (pos - start_pos) / real_target->result_scale;
            start_pos = pos;

            if (!allow_x) delta.x = 0;

            if (!allow_y) delta.y = 0;

            vec2 new_pos = real_target->pos.get() + delta;
            vec2 dt = new_pos - real_target->pos.get();
            real_target->pos = new_pos;

            if (on_move)
            {
                on_move(real_target->pos.get(), dt);
            }

            return true;
            // Translate to parent
            //    if (real_target->get_parent())
            //      p = real_target->get_parent()->to_local(p);
            //	run_on_ui([=, this]{
            /*       if (allow_x && allow_y)
                       real_target->pos = p;
                   else if (allow_x)
                       real_target->pos = {p.x, real_target->pos->y};
                   else
                       real_target->pos = {real_target->pos->x, p.y};
            		*/
            //	prev_pos = pos;
            //	if (on_move)
            //	{
            //		on_move(real_target->pos.get());
            //	}
            //	});
        }

        dragger::dragger()
        {
            target = this;
            clickable = true;
        }
    }
}
