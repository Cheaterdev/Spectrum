export module GUI:Window;
import :Base;
import :Resizable;
import :Dragger;
import :Button;


export namespace GUI
{
    namespace Elements
    {
        class window : public resizable
        {


            protected:
                label::ptr label_text;
                dragger::ptr title_bar;


               
            public:
                bool remove_if_empty = false;
                base::ptr contents;
				button::ptr close_button;


                Skin::Window skin;

                using ptr = s_ptr<window>;
                using wptr = w_ptr<window>;

                window();
                virtual void set_title(std::string title)
                {
                    label_text->text = title;
                }
                virtual void draw(Graphics::context& c) override;

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                virtual void on_bounds_changed(const rect& r) override;

                virtual void add_child(base::ptr obj) override;

                virtual void remove_child(base::ptr obj) override;

                virtual void on_touch() override;



        };

    }
}