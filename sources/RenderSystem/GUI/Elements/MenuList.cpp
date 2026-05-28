module GUI:MenuList;
import :Renderer;



void GUI::Elements::menu_list::draw(Context& c)
{
    c.renderer->draw_container(get_ptr(), c);
}

GUI::Elements::menu_list::menu_list(bool vertical)
{
    this->vertical = vertical;
    padding = { 1, 1, 1, 1 };
    docking = GUI::dock::NONE;
    if (!vertical)
    {
        docking = dock::TOP;
        size = { 25, 25 };
    }
    else
    {

       // max_size = { 0, 300 };
        width_size = size_type::MATCH_CHILDREN;
        height_size = size_type::MATCH_CHILDREN;
        contents->width_size = size_type::MATCH_CHILDREN;
        contents->height_size = size_type::MATCH_CHILDREN;

        filled->width_size = size_type::MATCH_CHILDREN;
        filled->height_size = size_type::MATCH_CHILDREN;
          filled->docking= dock::NONE;
        auto_size = true;
        filled->clamp_to_parent = ParentClamp::HEIGHT;

        filled->x_type = pos_x_type::LEFT;
        if (hor) base::remove_child(hor);
       // filled->docking = GUI::dock::TOP;
      //  contents->docking = GUI::dock::NONE;
    }
	
    clamp_to_parent = ParentClamp::ALL;
    // size = { padding->left + padding->right, padding->top + padding->bottom };
}

void GUI::Elements::menu_list::close_menus()
{
    if (vertical)self_close();
}

bool GUI::Elements::menu_list::need_open_on_hover()
{
    return true;
}

void GUI::Elements::menu_list::make_fixed_width()
{
    width_size = size_type::MATCH_CHILDREN;
    height_size = size_type::MATCH_CHILDREN;
    contents->width_size = size_type::MATCH_PARENT_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;

    filled->width_size = size_type::MATCH_PARENT_CHILDREN;
    filled->height_size = size_type::MATCH_CHILDREN;

 //   auto_size = true;
    filled->clamp_to_parent = ParentClamp::HEIGHT;
    filled->docking = dock::NONE;
    filled->x_type = pos_x_type::LEFT;
    if (hor) base::remove_child(hor);
}

bool GUI::Elements::menu_list::is_menu_component()
{
    return true;
}

bool GUI::Elements::menu_list::is_menu_open()
{
    for (auto c : elements)
    {
        if (c->is_menu_open())
            return true;
    }

    return false;
}

bool GUI::Elements::menu_list::is_self_open()
{
    return get_parent() != nullptr;
}

void GUI::Elements::menu_list::self_open(user_interface* ui)
{
    ui->add_child(get_ptr());
}

void GUI::Elements::menu_list::self_close()
{
    remove_from_parent();
}

void GUI::Elements::menu_list::remove_all()
{
    contents->remove_all();
    elements.clear();

    if (!vertical)
    {
        docking = dock::TOP;
        size = { 25, 25 };
    }

    else
        size = { padding->left + padding->right, padding->top + padding->bottom };
}

GUI::Elements::menu_list_element::menu_list_element(bool on_strip)
{
    this->on_strip = on_strip;
}

GUI::Elements::menu_strip::menu_strip() : menu_list(false)
{
    draw_icon = false;
}

bool GUI::Elements::menu_strip::need_open_on_hover()
{
    return is_menu_open();
}


namespace GUI
{

    void Elements::menu_list_element::draw(Context& c)
    {
            if (is_hovered())
				c.renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
			else
				c.renderer->draw_color(c, float4(5, 5, 5, 150) / 255.0f, get_render_bounds());

    }

    bool Elements::menu_list_element::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
    {
        base::on_mouse_action(action, button, pos);

        //		if (on_select)
        //		on_select(std::static_pointer_cast<menu_list_element>(get_ptr()));
        if (button == mouse_button::LEFT && action == mouse_action::DOWN)
        {
            if (menu)
            {
                if (menu->get_parent())
                    close_menu();
                else
                    open_menu();
            }

            else
            {

                    user_ui->close_menus();

                if (on_click)
                    on_click(get_ptr<menu_list_element>());
            }
        }

        return true;
    }

    std::shared_ptr<Elements::menu_list> Elements::menu_list_element::get_menu()
    {
        if (!menu)
            menu.reset(new Elements::menu_list());

        return menu;
    }

    void Elements::menu_list_element::close_menus()
    {
        base::close_menus();
        close_menu();
    }

    bool Elements::menu_list_element::is_menu_open()
    {
        if (!menu)
            return false;

        return menu->get_parent() != nullptr;
    }

    void Elements::menu_list_element::open_menu()
    {
        if (menu)
      
        {

            user_ui->add_child(menu);

            rect p = get_render_bounds();

            if (!on_strip)
                menu->pos = vec2(p.x + p.w, p.y);
            else
                menu->pos = vec2(p.x, p.y + p.h);


        }
    }

    void Elements::menu_list_element::close_menu()
    {
        if (menu)

        {
            menu->close_menus();

            menu->remove_from_parent();


        }

    }
	
    void Elements::menu_list_element::on_mouse_enter(vec2 pos)
    {
        if (on_hover)
            on_hover(get_ptr<menu_list_element>());
    }

}

