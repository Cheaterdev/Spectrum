export module GUI:TabControl;
import :Base;
import :MenuList;
import :Button;


export namespace GUI
{
    namespace Elements
    {
        class tab_control;
        class tab_button : public button
        {
                button::ptr close_button;


            public:
                using ptr = s_ptr<tab_button>;
                using wptr = w_ptr<tab_button>;


                base::ptr page;
                w_ptr<tab_control> owner;
                virtual void draw(Context& c) override;

                bool is_current();
                tab_button();

                virtual bool need_drag_drop() override;

                virtual void generate_container(base::ptr obj) override;

            protected:

                virtual void on_drag_start() override;

                virtual void on_drag_end() override;

        };

        class tab_strip: public base
        {
                my_unique_vector<tab_button::ptr> buttons;
                tab_button::ptr current;
                menu_list::ptr menu;
                button::ptr all;
                base::ptr strip;
                tab_control* owner;
                void recalculate_tabs();
            public:
                tab_button::ptr get_first_button();

                friend class tab_control;
                using ptr = s_ptr<tab_strip>;
                using wptr = w_ptr<tab_strip>;
                Skin::TabStrip skin;
				tab_strip();
                virtual void add_child(base::ptr obj) override;
                virtual void remove_child(base::ptr obj) override;

                virtual void draw(Context& c) override;
				virtual void draw_after(Context&) override;

                virtual void on_bounds_changed(const rect& r) override;
                virtual void close_menus() override;

                virtual bool can_accept(drag_n_drop_package::ptr) override;

                virtual bool on_drop(drag_n_drop_package::ptr, vec2) override;


        };
        class tab_control: public base
        {
                tab_strip::ptr strip;

                base::ptr cur_page;
                base::ptr contents;
            public:
                using ptr = s_ptr<tab_control>;
                using wptr = w_ptr<tab_control>;
                tab_control();
                bool remove_if_empty = false;
                void show_page(tab_button::ptr tab_but);
                tab_button::ptr add_page(std::string str, base::ptr obj = nullptr);
                tab_button::ptr add_button(tab_button::ptr);

				void remove_button(tab_button::ptr b);

                virtual void draw(Context&) override;

        };




    }
}