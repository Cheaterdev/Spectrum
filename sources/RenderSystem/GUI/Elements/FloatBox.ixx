export module GUI:FloatBox;
import :Base;
import :EditText;

export namespace GUI
{
    namespace Elements
    {
        // Numeric field over edit_text. Typing is free-form (digits, sign,
        // point, exponent); the value commits on Enter or when focus leaves,
        // clamped to [min, max] and re-formatted -- an unparsable entry
        // reverts. Escape reverts too. Up/Down and the wheel (while focused,
        // so a panel scrolled past it still scrolls) step by `step`: Shift
        // x10, Ctrl x0.1.
        class float_box : public edit_text
        {
                float value = 0;

                // Parses the text; nullopt if it isn't a number.
                std::optional<float> parse();
                void commit();
                void step_by(float steps, key_mods mods);
                void apply(float v, bool notify);

            public:
                using ptr  = s_ptr<float_box>;
                using wptr = w_ptr<float_box>;

                float min  = -std::numeric_limits<float>::max();
                float max  = std::numeric_limits<float>::max();
                float step = 0.1f;
                int   precision = 3;   // decimals shown; trailing zeros are trimmed

                // Committed values only (not every keystroke). UI thread.
                Events::Event<float> on_value_change;

                float_box();

                float get_value() const { return value; }
                // Clamps and shows v; doesn't fire on_value_change.
                void  set_value(float v);

                virtual void on_key_action(key_action action, long key, key_mods mods) override;
                virtual bool on_wheel(mouse_wheel type, float value, vec2 pos) override;
                virtual void on_focus_changed(bool focused) override;
        };
    }
}
