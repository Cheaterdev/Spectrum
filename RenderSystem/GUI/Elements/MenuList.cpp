
#include "pch.h"

void GUI::Elements::menu_list::draw(Render::context& c)
{
    renderer->draw_container(get_ptr(), c);
}
/*
GUI::Elements::menu_list_element::ptr GUI::Elements::menu_list::add_item(std::string elem)
{

    label::ptr e(new label());
    e->text = elem;
    e->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    e->docking = dock::FILL;
    menu_list_element::ptr l_e(new menu_list_element(!vertical));
    l_e->docking = vertical ? dock::TOP : dock::LEFT;
	l_e->text = elem;
    if (draw_icon)
    {
        l_e->padding = { 30, 3, 30, 3 };
        l_e->size = e->size.get() + vec2(60, 6);
    }
    else
    {
        l_e->padding = { 3, 3, 3, 3 };
        l_e->size = e->size.get() + vec2(6, 6);
    }

    l_e->add_child(e);
    l_e->on_hover = [this](menu_list_element::ptr elem)
    {
        if (!need_open_on_hover()) return;

        if (elem->is_menu_open()) return;

        base::close_menus();
        elem->open_menu();
        /* run_on_ui([this, elem]
         {
             close_menus();
           //
         });*
    };
    add_child(l_e);
    elements.push_back(l_e);

//    if (vertical)
    //    size = { std::max(size->x, padding->left + padding->right + l_e->size->x + l_e->margin->left + l_e->margin->right), size->y + l_e->size->y + l_e->margin->top + l_e->margin->bottom };

    l_e->selected = elements.size() == 1;

    return l_e;
}
*/
GUI::Elements::menu_list::menu_list(bool vertical)
{
    this->vertical = vertical;
    padding = { 1, 1, 1, 1 };

    if (!vertical)
    {
        docking = dock::TOP;
        size = {25, 25};
    }

    else
    {
        width_size = size_type::MATCH_CHILDREN;
        height_size = size_type::MATCH_CHILDREN;
    }

	clip_to_parent = true;
    // size = { padding->left + padding->right, padding->top + padding->bottom };
}

void GUI::Elements::menu_list::close_menus()
{
    if (vertical)self_close();
}


namespace GUI
{

    void Elements::menu_list_element::draw(Render::context& c)
    {
            if (is_hovered())
				renderer->draw_color(c, float4(53, 114, 202, 255) / 255, get_render_bounds());
			else
				renderer->draw_color(c, float4(5, 5, 5, 150) / 255, get_render_bounds());

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
                run_on_ui([this]()
                {
                    user_ui->close_menus();
                });

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
            run_on_ui([this]
        {

            user_ui->add_child(menu);

            rect p = get_render_bounds();

            if (!on_strip)
                menu->pos = vec2(p.x + p.w, p.y);
            else
                menu->pos = vec2(p.x, p.y + p.h);


        });
    }

    void Elements::menu_list_element::close_menu()
    {
        if (menu)
            run_on_ui([this]
        {
            menu->close_menus();

            menu->remove_from_parent();


        });
    }

    void Elements::menu_list_element::on_mouse_enter(vec2 pos)
    {
        if (on_hover)
            on_hover(get_ptr<menu_list_element>());
    }

}

