#include "pch.h"
#include "Resizer.h"




bool GUI::Elements::resizer::on_mouse_move(vec2 pos)
{
    if (!dragging) return true;

    auto real_target = target;

    if (!real_target)
        return false;

    rect pBounds = rect(real_target->pos.get(), real_target->size.get());
    vec2 delta = (pos - start_pos) / real_target->result_scale;

    if (static_cast<int>(dir)& static_cast<int>(direction::LEFT))
    {
        pBounds.x = orig_pos.x + delta.x;
        pBounds.w = orig_size.x - delta.x;

        if (pBounds.w < real_target->minimal_size.x)
        {
            pBounds.x = orig_pos.x + orig_size.x - real_target->minimal_size.x;
        }
    }

    if (static_cast<int>(dir)& static_cast<int>(direction::TOP))
    {
        pBounds.y = orig_pos.y + delta.y;
        pBounds.h = orig_size.y - delta.y;

        if (pBounds.h < real_target->minimal_size.y)
        {
            pBounds.y = orig_pos.y + orig_size.y - real_target->minimal_size.y;
        }
    }

    if (static_cast<int>(dir)& static_cast<int>(direction::RIGHT))
    {
        pBounds.w = std::max(real_target->minimal_size.x, orig_size.x + delta.x);
    }

    if (static_cast<int>(dir)& static_cast<int>(direction::BOTTOM))
    {
        pBounds.h = std::max(real_target->minimal_size.y, orig_size.y + delta.y);
    }

    real_target->pos = vec2(pBounds.pos);
    real_target->size = vec2(pBounds.size);
    return true;
}

void GUI::Elements::resizer::on_mouse_leave(vec2)
{
    cursor = cursor_style::ARROW;
}

void GUI::Elements::resizer::on_mouse_enter(vec2)
{
    if (dir == direction::LEFT_TOP || dir == direction::RIGHT_BOTTOM)
        cursor = cursor_style::NWSE;

    if (dir == direction::LEFT_BOTTOM || dir == direction::RIGHT_TOP)
        cursor = cursor_style::NESW;

    if (dir == direction::LEFT || dir == direction::RIGHT)
        cursor = cursor_style::WE;

    if (dir == direction::TOP || dir == direction::BOTTOM)
        cursor = cursor_style::NS;
}

GUI::Elements::resizer::resizer()
{
    size = {5, 5 };
    //	draw_helper = true;
}
