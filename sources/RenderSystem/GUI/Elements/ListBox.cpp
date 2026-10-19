module GUI:ListBox;
import :Renderer;

import <windows/windows.h>;


   GUI::Elements::list_element::list_element(list_box* owner):owner(owner)
   {
   
   }


void GUI::Elements::list_box::on_pre_render(Context& c)
{
    scroll_container::on_pre_render(c);

    list_element* selected = nullptr;
    if (!multi_select)
        for (auto& e : elements)
            if (e->selected)
            {
                selected = e.get();
                break;
            }

    float y = 0.0f, h = 0.0f;
    if (selected)
    {
        y = selected->get_render_bounds().y - contents->get_render_bounds().y;
        h = selected->get_render_bounds().h;
    }

    const bool animate = user_ui && user_ui->window_animations;
    bar.update(selected != nullptr, y, h, c.delta_time, !animate);

    // The row under the cursor: the hit test finds a row's label, so look up the parents.
    list_element* hovered = nullptr;
    if (user_ui)
        if (auto hit = user_ui->get_hovered().lock())
            for (base* p = hit.get(); p && !hovered; p = p->get_raw_parent())
                for (auto& e : elements)
                    if (e.get() == p)
                    {
                        hovered = e.get();
                        break;
                    }

    float hover_y = 0.0f, hover_h = 0.0f;
    if (hovered)
    {
        hover_y = hovered->get_render_bounds().y - contents->get_render_bounds().y;
        hover_h = hovered->get_render_bounds().h;
    }
    hover_bar.update(hovered != nullptr, hover_y, hover_h, c.delta_time, !animate);
}

void GUI::Elements::list_box::draw(Context& c)
{
    c.renderer->draw_area(get_ptr(), c);

    if (elements.empty())
        return;

    // Over the background, under the rows (their text), inside the box.
    const rect box = get_render_bounds();
    const rect row = elements.front()->get_render_bounds();
    const float content_y = contents->get_render_bounds().y;

    auto draw_bar = [&](const SlidingHighlight& b, float alpha_scale)
    {
        if (b.alpha <= 0.0f)
            return;

        const float top    = std::max(content_y + b.y, box.y + padding->top * result_scale);
        const float bottom = std::min(content_y + b.y + b.h, box.y + box.h - padding->bottom * result_scale);
        if (bottom <= top)
            return;

        float4 color = float4(53, 114, 202, 255) / 255.0f;
        color.w *= b.alpha * alpha_scale;
        c.renderer->draw_color(c, color, rect(row.x, top, row.w, bottom - top));
    };

    draw_bar(hover_bar, 0.22f);   // under the selection, which stays the strong one
    draw_bar(bar, 1.0f);
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


    void Elements::list_element::on_pre_render(Context& c)
    {
        base::on_pre_render(c);

        const float target = selected ? 1.0f : 0.0f;
        const bool animate = user_ui && user_ui->window_animations;

        if (!animate)
            selection_t = target;
        else
            selection_t = selection_t < target ? std::min(selection_t + c.delta_time / 0.12f, target)
                                               : std::max(selection_t - c.delta_time / 0.12f, target);
    }

    void Elements::list_element::draw(Context& c)
    {
        // Single selection: the list's sliding bar draws it.
        if (selection_t > 0.0f && owner && owner->multi_select)
        {
            float4 color = float4(53, 114, 202, 255) / 255.0f;
            color.w *= selection_t;
            c.renderer->draw_color(c, color, get_render_bounds());
        }
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