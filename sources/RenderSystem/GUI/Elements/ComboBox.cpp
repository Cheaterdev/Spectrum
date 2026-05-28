module GUI:ComboBox;
import :MenuList;
import :Renderer;


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
}

bool GUI::Elements::combo_box::is_menu_open()
{
    return menu->get_parent() != nullptr;
}

GUI::Elements::combo_element::ptr GUI::Elements::combo_box::add_item(std::string str)
{
    combo_element::ptr elem = menu->add_item<combo_element>(str);
    elem->on_click = [this](menu_list_element::ptr elem) {get_label()->text = elem->text; static_cast<combo_element*>(elem.get())->on_select(); };
    return elem;
}