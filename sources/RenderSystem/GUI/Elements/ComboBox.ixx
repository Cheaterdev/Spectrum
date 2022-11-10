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
                combo_element(bool on_strip) : menu_list_element(on_strip)
                {
                }

                void select()
                {
                    if (on_click) on_click(get_ptr<menu_list_element>());
                }
        };
        class combo_box : public button
        {
            protected:
                menu_list::ptr menu;
            public:
                using ptr = s_ptr<combo_box>;
                using wptr = w_ptr<combo_box>;

                using ptr = s_ptr<combo_box>;
                using wptr = w_ptr<combo_box>;

                combo_box();
                Skin::ComboBox skin;

                void remove_items()
                {
                    menu->remove_all();
                }
                bool is_menu_open()
                {
                    return menu->get_parent() != nullptr;
                }
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual void close_menus() override;


              //  std::function<void(combo_box*)> on_open;

                combo_element::ptr add_item(std::string str)
                {
                    combo_element::ptr elem = menu->add_item<combo_element>(str);
                    elem->on_click = [this](menu_list_element::ptr elem) {get_label()->text = elem->text; static_cast<combo_element*>(elem.get())->on_select(); };
                    return elem;
                }

                virtual void draw(Context& c) override;

        };


    }
}
