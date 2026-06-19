export module GUI:ColorPicker;
import :Base;
import :ColoredRect;
import :Label;
import :FloatSlider;

export namespace GUI
{
    namespace Elements
    {
        class color_picker : public base
        {
            colored_rect::ptr  preview;
            float_slider::ptr  slider_r;
            float_slider::ptr  slider_g;
            float_slider::ptr  slider_b;
            float_slider::ptr  slider_a;
            label::ptr         hex_label;
            label::ptr         rgb_label;

            float4 current_color  = {0, 0, 0, 1};
            bool   setting_color  = false;

            void on_slider_changed();
            void update_slider_gradients();
            void update_display();

        public:
            using ptr  = s_ptr<color_picker>;
            using wptr = w_ptr<color_picker>;

            Events::Event<float4> on_change;

            color_picker();

            float4 get_color() const;
            void   set_color(float4 color);
        };
    }
}
