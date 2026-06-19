export module GUI:FloatSlider;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        class float_slider : public base
        {
            bool dragging = false;

            float normalized() const;           // (value - min) / (max - min), clamped 0..1
            void  set_from_pos(float screen_x); // convert screen x to value and assign

        public:
            using ptr  = s_ptr<float_slider>;
            using wptr = w_ptr<float_slider>;

            float min = 0.0f;
            float max = 1.0f;

            property<float> value;

            float4 left_color  = {0.15f, 0.15f, 0.15f, 1.0f};
            float4 right_color = {0.85f, 0.85f, 0.85f, 1.0f};

            Events::Event<float> on_change;

            float_slider();

            virtual void draw(Context& c) override;
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
            virtual bool on_mouse_move(vec2 pos) override;
        };
    }
}
