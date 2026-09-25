export module GUI:VectorIcon;
import :Base;
import TextEngine;

export namespace GUI
{
    namespace Elements
    {
        // One of Text::Engine's built-in vector icons (chevron_right,
        // chevron_down, close, error, warning), fitted into this element's
        // bounds and tinted with color. Rasterized at the exact on-screen size,
        // so it stays sharp at any UI scale. An empty name draws nothing.
        class vector_icon : public base
        {
            Text::Layout layout;

            // Set from the UI thread (e.g. a toggle click), read by the tree walk.
            std::mutex  m;
            std::string icon;

        public:
            using ptr = s_ptr<vector_icon>;
            using wptr = w_ptr<vector_icon>;

            float4 color = float4(1, 1, 1, 1);

            vector_icon(std::string icon = {});

            void set_icon(std::string name);

            virtual void on_pre_render(Context& c) override;
            virtual void pre_draw(HAL::CommandList::ptr list) override;
            virtual void draw(Context& c) override;
        };
    }
}
