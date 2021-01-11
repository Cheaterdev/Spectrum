#include "pch.h"

class c_contents : public GUI::base
{
        GUI::Elements::scroll_container* owner;
    public:
        c_contents(GUI::Elements::scroll_container* owner)
        {
            this->owner = owner;
            clickable = false;
        }

        virtual void on_size_changed(const vec2& r) override
        {
            base::on_size_changed(r);
            owner->resized();
        }
        /*
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
        	return 	owner->on_mouse_action(action, button, pos);
        }
        virtual void on_key_action(long key) override
        {
        	owner->on_key_action(key);
        }
        virtual bool on_mouse_move(vec2 pos) override
        {
        	return 	owner->on_mouse_move(pos);
        }*/
};


class mover : public GUI::base
{
        GUI::Elements::scroll_container* owner;
        bool dragging = false;
        bool pressed = false;
        vec2 start_pos;
        vec2 prev_pos;
        vec2 speed;
//	float inertion;
    public:
        mover(GUI::Elements::scroll_container* owner)
        {
            this->owner = owner;
            thinkable = true;
        }

        virtual bool on_mouse_move(vec2 pos) override
        {
            if (pressed && !dragging && (pos - start_pos).length() > 5)
            {
                dragging = true;
                prev_pos = pos;
                set_movable(true);
            }

            if (dragging)
            {
                speed = pos - prev_pos;
                owner->moving(speed);
                prev_pos = pos;
            }

            return dragging;
        }
        virtual bool on_wheel(mouse_wheel type, float value, vec2 pos) override
        {
            if (type == mouse_wheel::VERTICAL)
                owner->on_wheel(type, value, pos);

            return true;
        };
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::RIGHT)
            {
                pressed = action == mouse_action::DOWN;
                start_pos = pos;

                if (!pressed)
                {
                    dragging = false;
                    set_movable(false);
                }
            }

            return false;
        }


        virtual void on_key_action(long key) override
        {
        }


};



sizer GUI::Elements::scroll_container::update_layout(sizer r, float scale)
{
    resized();
    return base::update_layout(r, scale);
}

void GUI::Elements::scroll_container::resized()
{
    hor->set_sizes(filled->get_render_bounds().w, contents->scaled_size->x, contents->pos->x);
    vert->set_sizes(filled->get_render_bounds().h, contents->scaled_size->y, contents->pos->y);

    if (!allow_overflow)
        contents->pos = vec2::min(vec2(0, 0), vec2::max(contents->pos.get(), -contents->scaled_size.get() + vec2(filled->get_render_bounds().size)));
}

void GUI::Elements::scroll_container::remove_child(base::ptr obj)
{
    contents->remove_child(obj);
}

void GUI::Elements::scroll_container::add_child(base::ptr obj)
{
    contents->add_child(obj);
}

GUI::Elements::scroll_container::scroll_container()
{
    thinkable = true;
    docking = GUI::dock::FILL;
    vert.reset(new GUI::Elements::scroll_bar(GUI::Elements::scroll_bar::scroll_type::VERTICAL));
    vert->docking = GUI::dock::RIGHT;
    base::add_child(vert);
    hor.reset(new GUI::Elements::scroll_bar(GUI::Elements::scroll_bar::scroll_type::HORIZONTAL));
    hor->docking = GUI::dock::BOTTOM;
    base::add_child(hor);
    filled.reset(new c_contents(this));
    filled->docking = GUI::dock::FILL;
    base::add_child(filled);
    over_filled.reset(new mover(this));
    over_filled->docking = GUI::dock::FILL;
    contents.reset(new c_contents(this));
    contents->docking = GUI::dock::NONE;
    contents->size = { 00, 00 };
    contents->clip_to_parent = false;
    contents->width_size = size_type::MATCH_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;
    filled->add_child(contents);
    filled->add_child(over_filled);
    vert->on_move = [this](float y)
    {
        contents->pos = { contents->pos->x, -y* (contents->size->y - filled->get_render_bounds().h) };
    };
    hor->on_move = [this](float x)
    {
        contents->pos = { -x* (contents->size->x - filled->get_render_bounds().w), contents->pos->y };
    };
}

void GUI::Elements::scroll_container::moving(vec2 pos)
{
    vec2 p = contents->pos.get() + pos;

    if (!allow_overflow)
        contents->pos = vec2::min(vec2(0, 0), vec2::max(p, -contents->scaled_size.get() + vec2(filled->get_render_bounds().size)));
    else
        contents->pos = p;
}


bool GUI::Elements::scroll_container::on_wheel(mouse_wheel type, float value, vec2 pos)
{
    speed.y += value * 20;
    return true;
}
