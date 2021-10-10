#include "pch_render.h"
#include "ValueBox.h"

namespace GUI
{
    namespace Elements
    {

        value_box::value_box()
        {
            clickable = true;
            thinkable = true;
            minus_butt = create_less_button();
            plus_butt = create_more_button();
            plus_butt->click_style = button::click_styles::IMMEDIATELY;
            minus_butt->click_style = button::click_styles::IMMEDIATELY;
            //minus_butt->size=minus_butt->size.get()/2;
            //plus_butt->size=plus_butt->size.get()/2;
            height_size = size_type::MATCH_CHILDREN;
            size =
            { 110, 0 };
            add_child(minus_butt);
            add_child(plus_butt);
            image::ptr counter(new image());
            counter->texture = Skin::get().DefaultEditBox.Normal;
            //  counter->texture.padding =
            // { 10, 10, 10, 10 };
            counter->size =
            { 10, 24 };
            counter->docking = dock::FILL;
            counter->height_size = size_type::FIXED;
            add_child(counter);
            info.reset(new label);
            info->text = "";
            info->docking = dock::FILL;
            info->color = {0, 0, 0, 200};
            counter->add_child(info);
            //   add_child(info);
            minus_butt->on_click = [this](button::ptr)
            {
                if (current_value > min_value)current_value--;

                on_change(current_value, this);
            };
            plus_butt->on_click = [this](button::ptr)
            {
                if (current_value < max_value)current_value++;

                on_change(current_value, this);
            };
        }
    }
}