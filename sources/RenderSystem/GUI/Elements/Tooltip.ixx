export module GUI:Tooltip;
import :Base;
import :Label;
import :ColoredRect;

export namespace GUI
{
    namespace Elements
    {
        // Floating tooltip overlay — managed by tooltip_manager, not used directly
        class tooltip_widget : public base
        {
            colored_rect::ptr background;
            label::ptr        text_label;
        public:
            using ptr  = s_ptr<tooltip_widget>;
            using wptr = w_ptr<tooltip_widget>;

            tooltip_widget();
            void show(const std::string& text, vec2 screen_pos);
            void hide();
        };

        // Add one instance as a child of user_interface to enable tooltips on all widgets
        class tooltip_manager : public base
        {
            tooltip_widget::ptr overlay;
            base*  last_hovered  = nullptr;
            float  timer         = 0.0f;
            bool   shown         = false;

            static constexpr float DELAY = 0.6f;

        public:
            using ptr  = s_ptr<tooltip_manager>;
            using wptr = w_ptr<tooltip_manager>;

            tooltip_manager();

            virtual void think(float dt) override;

        protected:
            virtual void on_add(base* parent) override;
        };
    }
}
