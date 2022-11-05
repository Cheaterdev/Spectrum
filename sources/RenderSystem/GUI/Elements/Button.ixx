export module GUI:Button;
import :Image;
import :Skin;
import :Label;

export namespace GUI
{
    namespace Elements
    {

        class button : public base
        {

                label::ptr label_text;
                image::ptr im;

            public:
                enum class view_style
                {
                    DEFAULT, NO_BACKGROUND, FLAT
                };

                enum class click_styles
                {
                    DEFAULT, IMMEDIATELY
                };

                using ptr = s_ptr<button>;
                using wptr = w_ptr<button>;
               Events::Event<ptr> on_click;
                view_style background_style = view_style::DEFAULT;
                click_styles click_style = click_styles::DEFAULT;
                Skin::Button skin;



                button();

                virtual void draw(Graphics::context& c) override;

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                virtual void on_bounds_changed(const rect& r) override;

                label::ptr get_label()
                {
                    return label_text;
                }

                image::ptr get_image()
                {
                    return im;
                }

        };
    }

}