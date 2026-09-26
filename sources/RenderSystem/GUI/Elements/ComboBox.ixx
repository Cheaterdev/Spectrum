export module GUI:ComboBox;





import :MenuList;
import :Button;
import :EditText;


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

                // Searchable: a filter field first in the dropdown.
                edit_text::ptr                  search;
                std::vector<combo_element::ptr> items;
                void apply_filter(const std::string& text);
                void select_first_match();
            public:
                using ptr = s_ptr<combo_box>;
                using wptr = w_ptr<combo_box>;

                combo_box();
                Skin::ComboBox skin;

                void remove_items();
                bool is_menu_open();

                // A search field at the top of the dropdown that filters the
                // items as you type (case-insensitive substring); Enter picks
                // the first match, Escape closes. Call before add_item: the
                // field has to be the dropdown's first entry.
                void set_searchable(bool value);
                bool is_searchable() const { return search != nullptr; }
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual void close_menus() override;


              //  std::function<void(combo_box*)> on_open;

                combo_element::ptr add_item(std::string str);

                virtual void draw(Context& c) override;

        };


    }
}
