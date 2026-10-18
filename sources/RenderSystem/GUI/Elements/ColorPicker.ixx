export module GUI:ColorPicker;
import :Base;
import :ColoredRect;
import :Label;
import :FloatSlider;

export namespace GUI
{
    namespace Elements
    {
        // Which components the picker edits. A disabled channel keeps whatever
        // value set_color() gave it and just isn't shown or editable.
        enum class color_channels : uint
        {
            None = 0,
            R    = 1 << 0,
            G    = 1 << 1,
            B    = 1 << 2,
            A    = 1 << 3,
            RGB  = 7,
            RGBA = 15
        };

        // Which read-outs are shown besides the sliders.
        enum class color_display : uint
        {
            None    = 0,
            Preview = 1 << 0,   // swatch
            Hex     = 1 << 1,   // #RRGGBB (#RRGGBBAA with alpha)
            Rgb     = 1 << 2,   // rgb(r g b) of the enabled channels, 0-255
            All     = 7
        };

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

            color_channels channels = color_channels::RGBA;
            color_display  display  = color_display::All;

            void on_slider_changed();
            void update_slider_gradients();
            void update_display();
            void relayout();

        public:
            using ptr  = s_ptr<color_picker>;
            using wptr = w_ptr<color_picker>;

            Events::Event<float4> on_change;

            color_picker();

            float4 get_color() const;
            void   set_color(float4 color);

            void set_channels(color_channels value);
            void set_display(color_display value);
        };
    }
}
