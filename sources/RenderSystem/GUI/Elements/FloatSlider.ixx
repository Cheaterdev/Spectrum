export module GUI:FloatSlider;
import :Base;
import :Label;

export namespace GUI
{
    namespace Elements
    {
        // Compact number text that never uses exponent notation: 0.25, 12.5, 1234.
        std::string format_number(double v);

        // 1536 -> "1.5 KB", 67108864 -> "64 MB" (binary units).
        std::string format_bytes(double bytes);

        class float_slider : public base
        {
            bool dragging = false;
            label::ptr value_label; // created by show_value(); text follows `value`
            std::function<std::string(float)> formatter; // empty: format_number

            void update_value_label(float v);

            float normalized() const;           // (value - min) / (max - min), clamped 0..1
            void  set_from_pos(float screen_x); // convert screen x to value and assign

        public:
            using ptr  = s_ptr<float_slider>;
            using wptr = w_ptr<float_slider>;

            float min = 0.0f;
            float max = 1.0f;

            bool logarithmic = false; // log mapping of value to position; requires min > 0

            property<float> value;

            float4 left_color  = {0.15f, 0.15f, 0.15f, 1.0f};
            float4 right_color = {0.85f, 0.85f, 0.85f, 1.0f};

            Events::Event<float> on_change;

            float_slider();

            // Draws the current value (3 significant digits) centered over the track.
            // On by default (the constructor calls it); idempotent.
            void show_value();

            // Replaces the default number text, e.g. format_bytes for a memory size.
            void set_formatter(std::function<std::string(float)> f);

            virtual void draw(Context& c) override;
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
            virtual bool on_mouse_move(vec2 pos) override;
        };
    }
}
