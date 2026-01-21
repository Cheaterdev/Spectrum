module GUI:ListBox;
import :Renderer;


   GUI::Elements::list_element::list_element(list_box* owner):owner(owner)
   {
   
   }


void GUI::Elements::list_box::draw(Context& c)
{
    c.renderer->draw_area(get_ptr(), c);
}

GUI::Elements::list_element::ptr GUI::Elements::list_box::add_item(std::string elem)
{
    label::ptr e(new label());
    e->text = elem;
    e->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    e->docking = dock::FILL;
    //e->color = rgba8(10, 10, 10, 255);
    list_element::ptr l_e(new list_element(this));
    l_e->docking = dock::TOP;
    l_e->padding = { 1, 1, 2, 1 };
    l_e->size = e->size.get() + vec2(4, 4);
    l_e->add_child(e);
   
    add_child(l_e);
    elements.push_back(l_e);
//	contents->size = { 80, contents->size->y + l_e->size->y + l_e->margin->top + l_e->margin->bottom };
    l_e->selected = elements.size() == 1;
    return l_e;
}

GUI::Elements::list_box::list_box()
{
    contents->size = { 20, 00 };
    contents->width_size = size_type::MATCH_PARENT_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;
    padding = { 5, 5, 5, 5 };
}

     void GUI::Elements::list_box::on_select(list_element* elem)
     {

         // todo:optimize this sht
             for (auto& e : elements)
            e->selected = e.get() == elem;
     
     }
    


        void  GUI::Elements::list_box::clear_items()
        {
                for (auto& e : elements)
                  e->remove_from_parent();

                elements.clear();
        }
namespace GUI
{


    void Elements::list_element::draw(Context& c)
    {
           if (selected)
             c.renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
    }

    bool Elements::list_element::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
    {
        base::on_mouse_action(action, button, pos);

        if (button == mouse_button::LEFT)
            if (action == mouse_action::UP)
            {
                owner->on_select(this);
            
                                   if (on_select)
                    on_select(std::static_pointer_cast<list_element>(get_ptr()));
            }


        return true;
    }




            

}