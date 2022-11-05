#pragma once
import GUI;

namespace GUI
{
    namespace Elements
    {
        class dragger : public base
        {
        protected:
            bool dragging = false;
            vec2 orig_pos;
            vec2 start_pos;
            vec2 orig_size;
            float orig_scale;
        public:
            base* target;
            bool allow_x = true;
            bool allow_y = true;
            dragger();
            using ptr = s_ptr<dragger>;
            using wptr = w_ptr<dragger>;
            std::function<void(vec2, vec2)> on_move;
            std::function<void()> on_start_drag;
            std::function<void()> on_end_drag;

            virtual bool on_mouse_move(vec2 pos) override;

            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

        };
    }
}