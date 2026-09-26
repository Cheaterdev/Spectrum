module GUI:ComboBox;
import :MenuList;
import :Renderer;


import <windows/windows.h>;

namespace
{
    // The dropdown's filter field. Counts as part of the menu, or the press
    // that focuses it would close the dropdown (user_interface closes menus
    // on any press outside a menu component).
    class combo_search : public GUI::Elements::edit_text
    {
    public:
        std::function<void()> on_enter;
        std::function<void()> on_escape;

        bool is_menu_component() override { return true; }

        void on_key_action(key_action action, long key, key_mods mods) override
        {
            if (action == key_action::DOWN && key == VK_RETURN && on_enter)  { on_enter();  return; }
            if (action == key_action::DOWN && key == VK_ESCAPE && on_escape) { on_escape(); return; }
            edit_text::on_key_action(action, key, mods);
        }
    };

    bool contains_ignore_case(std::string_view haystack, std::string_view needle)
    {
        auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c; };
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
                           [&](char a, char b) { return lower(a) == lower(b); }) != haystack.end();
    }
}

void GUI::Elements::combo_box::set_searchable(bool value)
{
    if (value == is_searchable())
        return;

    if (!value)
    {
        search->remove_from_parent();
        search = nullptr;
        apply_filter({});
        return;
    }

    auto field = std::make_shared<combo_search>();
    field->placeholder = "Search...";
    field->docking     = dock::TOP;
    field->width_size  = size_type::MATCH_PARENT_CHILDREN;
    field->height_size = size_type::FIXED;
    field->size        = { 0, field->single_line_height() };
    field->on_enter    = [this]() { select_first_match(); };
    field->on_escape   = [this]() { close_menus(); };
    // Fired from the tree walk; visibility changes relayout, so they go to
    // the UI thread.
    field->on_change = [this](const std::string& text)
        {
            run_on_ui([this, text]() { apply_filter(text); });
        };

    search = field;
    menu->contents->add_child(search);
}

void GUI::Elements::combo_box::apply_filter(const std::string& text)
{
    for (auto& item : items)
        item->visible = text.empty() || contains_ignore_case(item->text, text);
}

void GUI::Elements::combo_box::select_first_match()
{
    for (auto& item : items)
        if (item->visible.get())
        {
            item->select();
            close_menus();
            return;
        }
}

bool GUI::Elements::combo_box::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    button::on_mouse_action(action, button, pos);

    if (action == mouse_action::UP)
    { 
            menu->pos = vec2(render_bounds->pos) + vec2(0, render_bounds->h);
            menu->size = { render_bounds->size.x, menu->size->y };
            menu->make_fixed_width();
            menu->docking = dock::NONE;
            menu->width_size = size_type::FIXED;

            user_ui->add_child(menu);

            if (search)
            {
                search->set_text("");
                apply_filter({});
                search->focus();
            }
    }

    return true;
}
void GUI::Elements::combo_box::draw(Context& c)
{
    if (is_pressed())
        c.renderer->draw(c, skin.Pressed, get_render_bounds());
    else if (is_hovered())
        c.renderer->draw(c, skin.Hover, get_render_bounds());
    else
        c.renderer->draw(c, skin.Normal, get_render_bounds());
}


void GUI::Elements::combo_box::close_menus()
{
    // Before the menu leaves the tree: a detached field can't unfocus, and
    // would keep taking keys while hidden.
    if (search)
        search->unfocus();

    base::close_menus();

    menu->remove_from_parent();
}

GUI::Elements::combo_box::combo_box()
{
    size = { 25, 25 };
    height_size = size_type::FIXED;
    menu.reset(new menu_list());
    menu->draw_icon = false;
    padding = { 5, 2, 30, 2  };
    skin = Skin::get().DefaultComboBox;

  //  menu->clamp_to_parent = true;
    /*add_item("item 1");
    add_item("item 2");
    add_item("item 3");*/
}

GUI::Elements::combo_element::combo_element(bool on_strip) : menu_list_element(on_strip)
{
}

void GUI::Elements::combo_element::select()
{
    if (on_click) on_click(get_ptr<menu_list_element>());
}

void GUI::Elements::combo_box::remove_items()
{
    menu->remove_all();
    items.clear();

    // remove_all took the search field too; it has to stay first.
    if (search)
        menu->contents->add_child(search);
}

bool GUI::Elements::combo_box::is_menu_open()
{
    return menu->get_parent() != nullptr;
}

GUI::Elements::combo_element::ptr GUI::Elements::combo_box::add_item(std::string str)
{
    combo_element::ptr elem = menu->add_item<combo_element>(str);
    elem->on_click = [this](menu_list_element::ptr elem)
        {
            if (search) search->unfocus();
            get_label()->text = elem->text;
            static_cast<combo_element*>(elem.get())->on_select();
        };
    items.push_back(elem);
    return elem;
}