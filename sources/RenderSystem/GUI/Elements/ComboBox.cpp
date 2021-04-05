#include "pch.h"

bool GUI::Elements::combo_box::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    button::on_mouse_action(action, button, pos);

    if (action == mouse_action::UP)
    { 
            menu->pos = vec2(render_bounds->pos) + vec2(0, render_bounds->h);
            menu->width_size = size_type::NONE;
            menu->size = { render_bounds->size.x, menu->size->y };
            user_ui->add_child(menu);
            menu->width_size = size_type::FIXED;
    }

    return true;
}
void GUI::Elements::combo_box::draw(Render::context& c)
{
    if (is_pressed())
        renderer->draw(c, skin.Pressed, get_render_bounds());
    else if (is_hovered())
        renderer->draw(c, skin.Hover, get_render_bounds());
    else
        renderer->draw(c, skin.Normal, get_render_bounds());
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
    /*add_item("item 1");
    add_item("item 2");
    add_item("item 3");*/
}