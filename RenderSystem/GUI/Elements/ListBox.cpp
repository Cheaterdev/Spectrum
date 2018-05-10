#include "pch.h"

void GUI::Elements::list_box::draw(Render::context& c)
{
    renderer->draw_area(get_ptr(), c);
}

GUI::Elements::label::ptr GUI::Elements::list_box::add_item(std::string elem)
{
    label::ptr e(new label());
    e->text = elem;
    e->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    e->docking = dock::FILL;
    e->color = rgba8(10, 10, 10, 255);
    list_element::ptr l_e(new list_element());
    l_e->docking = dock::TOP;
    l_e->padding = { 1, 1, 2, 1 };
    l_e->size = e->size.get() + vec2(4, 4);
    l_e->add_child(e);
    l_e->on_select = [this](list_element::ptr elem)
    {
        for (auto e : elements)
            e->selected = e == elem;
    };
    add_child(l_e);
    elements.push_back(l_e);
//	contents->size = { 80, contents->size->y + l_e->size->y + l_e->margin->top + l_e->margin->bottom };
    l_e->selected = elements.size() == 1;
    return e;
}

GUI::Elements::list_box::list_box()
{
    contents->size = { 20, 00 };
    contents->width_size = size_type::MATCH_PARENT_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;
    padding = { 5, 5, 5, 5 };
}


namespace GUI
{


    void Elements::list_element::draw(Render::context& c)
    {
        //   if (selected)
        //      renderer->draw(this, c);
    }

    bool Elements::list_element::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
    {
        base::on_mouse_action(action, button, pos);

        if (button == mouse_button::LEFT)
            if (action == mouse_action::UP)
                if (on_select)
                    on_select(std::static_pointer_cast<list_element>(get_ptr()));

        return true;
    }

}