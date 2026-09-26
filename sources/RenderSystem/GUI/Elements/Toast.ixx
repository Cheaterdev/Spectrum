export module GUI:Toast;
import :Base;
import :Label;
import :ColoredRect;

export namespace GUI
{
    namespace Elements
    {
        enum class toast_kind : int
        {
            info,
            success,
            warning,
            error
        };

        // One notification: accent strip + message. Animated and removed by
        // toast_manager; a click dismisses it early.
        class toast_widget : public base
        {
                colored_rect::ptr background, strip;
                label::ptr        message;
                float4            accent;

            public:
                using ptr = s_ptr<toast_widget>;

                float remaining;
                float lifetime;

                toast_widget(const std::string& text, toast_kind kind, float seconds);

                // 0..1 opacity, from the fade-in/out.
                void set_opacity(float a);

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        };

        // Timed messages stacked in the bottom-right corner, newest at the
        // bottom, fading out after a few seconds. Add one instance to
        // user_interface (like tooltip_manager); show() works from any thread
        // and before the manager exists -- messages wait for it.
        class toast_manager : public base
        {
                base::ptr stack;
                std::vector<toast_widget::ptr> toasts;

                static constexpr size_t max_visible = 5;

            public:
                using ptr  = s_ptr<toast_manager>;
                using wptr = w_ptr<toast_manager>;

                static void show(std::string text, toast_kind kind = toast_kind::info, float seconds = 4.0f);

                toast_manager();

                virtual void think(float dt) override;

            protected:
                virtual void on_add(base* parent) override;
        };
    }
}
