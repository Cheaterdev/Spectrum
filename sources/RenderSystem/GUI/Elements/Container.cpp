#include "pch.h"
#include "Container.h"

namespace GUI
{

    void Elements::container::on_bounds_changed(const rect& r)
    {
        recalculate();
    }
    void Elements::container::recalculate()
    {
        float w = get_render_bounds().w;// -vert->size->x;
        float elem_size = std::min(128.0f, w);
        unsigned int best_count = Math::clamp<unsigned int>(unsigned int(w / elem_size), 1u, unsigned int(elements.size()));
        elem_size = std::min(128.0f, (w - 20) / best_count);
        float offset = std::max<float>((w - elem_size * best_count) / (best_count + 1), 0);
        vec2 cursor = { offset, 0 };
        int c = 0;

        for (auto elem : elements)
        {
            c++;
            elem->pos = cursor;
            elem->size = { elem_size, elem_size };
            cursor.x += offset + elem_size;

            if (c == best_count)
            {
                cursor.x = offset;
                cursor.y += elem_size;
                c = 0;
            }
        }
    }

    void Elements::container::remove_elements()
    {
        for (auto c : elements)
            scroll_container::remove_child(c);

        elements.clear();
    }

    void Elements::container::add_child(base::ptr obj)
    {
        obj->docking = dock::FILL;
        base::ptr c(new base());
        c->size = { 50, 50 };
        c->padding = {2, 2, 2, 2};
        //		c->draw_helper = true;
        scroll_container::add_child(c);
        c->add_child(obj);
        elements.push_back(c);
        recalculate();
    }

    Elements::container::container()
    {
        contents->size = { 20, 20 };
        contents->width_size = size_type::MATCH_CHILDREN;
        contents->height_size = size_type::MATCH_CHILDREN;
        //padding = { 5, 5, 5, 5 };
    }

}