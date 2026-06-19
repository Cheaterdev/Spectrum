export module GUI:ToggleSwitch;
import :Base;
import :Skin;

export namespace GUI
{
    namespace Elements
    {
        class toggle_switch : public base
        {
            float thumb_t    = 0.0f;   // 0 = off, 1 = on, animated
            bool  initialized = false;

        public:
            using ptr  = s_ptr<toggle_switch>;
            using wptr = w_ptr<toggle_switch>;

            property<bool> checked;

            float4 color_off   = {0.35f, 0.35f, 0.35f, 1.0f};
            float4 color_on    = {0.25f, 0.65f, 0.25f, 1.0f};
            float  anim_speed  = 10.0f;
            bool   allow_uncheck = true;

            Events::Event<bool> on_toggle;

            toggle_switch();

            void set_checked(bool v) { checked = v; }
            bool is_checked() const  { return checked.get(); }

            virtual void draw(Context& c) override;
            virtual void think(float dt) override;
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        };
    }
}
