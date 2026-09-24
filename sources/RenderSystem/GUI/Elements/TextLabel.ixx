export module GUI:TextLabel;
import :Base;
import TextEngine;

export namespace GUI
{
    namespace Elements
    {
        // Single-run text drawn through Text::Engine (Skribidi): UTF-8, bidi,
        // font fallback and color emoji. Sizes itself to its text.
        class text_label : public base
        {
            Text::Layout layout;

            void update_size();

        public:
            using ptr = s_ptr<text_label>;
            using wptr = w_ptr<text_label>;

            property<std::string> text;
            property<float>       font_size;
            float4                color = float4(1, 1, 1, 1);

            text_label();

            virtual void on_pre_render(Context& c) override;
            virtual void pre_draw(HAL::CommandList::ptr list) override;
            virtual void draw(Context& c) override;
        };
    }
}
