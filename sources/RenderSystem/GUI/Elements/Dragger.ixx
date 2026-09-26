export module GUI:Dragger;

import :Base;

export namespace GUI
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

            // Draw as a visible handle: idle_color, active_color while hovered
            // or dragged, and (plain draggers) a move cursor matching allow_x/y.
            bool draw_color = true;
            float4 idle_color   = float4(0, 0, 0, 0.12f);
            float4 active_color = float4(0.25f, 0.5f, 1.0f, 0.6f);
            dragger();
            bool is_dragging() const { return dragging; }
            using ptr = s_ptr<dragger>;
            using wptr = w_ptr<dragger>;
            std::function<void(vec2, vec2)> on_move;
            std::function<void()> on_start_drag;
            std::function<void()> on_end_drag;

            virtual bool on_mouse_move(vec2 pos) override;
            virtual void on_mouse_enter(vec2 pos) override;
            virtual void on_mouse_leave(vec2 pos) override;

            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
            
            virtual void draw(Context& c)	 override;

        };
    }
}