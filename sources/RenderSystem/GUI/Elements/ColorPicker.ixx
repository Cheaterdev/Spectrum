export module GUI:ColorPicker;
import :Base;
import :ColoredRect;
import :Label;

export namespace GUI
{
    namespace Elements
    {
        class color_slider : public base
        {
            bool dragging = false;

        public:
            using ptr = s_ptr<color_slider>;
            using wptr = w_ptr<color_slider>;

            float4 left_color  = {0, 0, 0, 1};
            float4 right_color = {1, 0, 0, 1};
            float  value       = 0.0f; // 0..1

            std::function<void(float)> on_change;

            color_slider();

            void set_value(float v, bool fire_callback = true);

            virtual void draw(Context& c) override;
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
            virtual bool on_mouse_move(vec2 pos) override;
        };

        class color_picker : public base
        {
            colored_rect::ptr preview;
            color_slider::ptr slider_r;
            color_slider::ptr slider_g;
            color_slider::ptr slider_b;
            color_slider::ptr slider_a;
            label::ptr        hex_label;
            label::ptr        rgb_label;

            float4 current_color = {0, 0, 0, 1};

            void on_slider_changed();
            void update_slider_gradients();
            void update_display();

        public:
            using ptr = s_ptr<color_picker>;
            using wptr = w_ptr<color_picker>;

            Events::Event<float4> on_change;

            color_picker();

            float4 get_color() const;
            void   set_color(float4 color);
        };
    }
}
