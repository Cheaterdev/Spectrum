module GUI:ListBox;
import :Renderer;

import <windows/windows.h>;


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
    if (elements.size() == 1) anchor = 0;
    return l_e;
}

GUI::Elements::list_box::list_box()
{
    contents->size = { 20, 00 };
    contents->width_size = size_type::MATCH_PARENT_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;
    padding = { 5, 5, 5, 5 };
}

     void GUI::Elements::list_box::on_select(list_element* elem, bool shift, bool ctrl)
     {
         const auto found = std::find_if(elements.begin(), elements.end(), [&](auto& e) { return e.get() == elem; });
         if (found == elements.end())
             return;
         const int index = int(found - elements.begin());

         if (!multi_select || (!shift && !ctrl))
         {
             for (auto& e : elements)
                 e->selected = e.get() == elem;
             anchor = index;
         }
         else if (shift && anchor >= 0 && anchor < (int)elements.size())
         {
             // Ctrl+Shift adds the range to what is selected; Shift alone replaces it.
             const int lo = std::min(anchor, index), hi = std::max(anchor, index);
             for (int i = 0; i < (int)elements.size(); i++)
                 if (i >= lo && i <= hi)  elements[i]->selected = true;
                 else if (!ctrl)          elements[i]->selected = false;
         }
         else
         {
             elem->selected = ctrl ? !elem->selected : true;
             anchor = index;
         }

         if (on_selection_changed)
             on_selection_changed(selected_indices());
     }

     std::vector<int> GUI::Elements::list_box::selected_indices() const
     {
         std::vector<int> result;
         for (int i = 0; i < (int)elements.size(); i++)
             if (elements[i]->selected)
                 result.push_back(i);
         return result;
     }
    


        void  GUI::Elements::list_box::clear_items()
        {
                for (auto& e : elements)
                  e->remove_from_parent();

                elements.clear();
                anchor = -1;
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
                // Mouse events carry no modifiers.
                owner->on_select(this, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
            
                                   if (on_select)
                    on_select(std::static_pointer_cast<list_element>(get_ptr()));
            }


        return true;
    }




            

}