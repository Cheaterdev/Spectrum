export module GUI:ComboBox;





import :MenuList;
import :Button;


export namespace GUI
{
    namespace Elements
    {
        class combo_element : public menu_list_element
        {
            public:
                using ptr = s_ptr<combo_element>;
                std::function<void()> on_select;
                combo_element(bool on_strip);

                void select();
        };
        class combo_box : public button
        {
            protected:
                menu_list::ptr menu;
            public:
                using ptr = s_ptr<combo_box>;
                using wptr = w_ptr<combo_box>;

                combo_box();
                Skin::ComboBox skin;

                void remove_items();
                bool is_menu_open();
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual void close_menus() override;


              //  std::function<void(combo_box*)> on_open;

                combo_element::ptr add_item(std::string str);

                virtual void draw(Context& c) override;

        };


    }
}
