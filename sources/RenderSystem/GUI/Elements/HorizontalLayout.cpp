#include "pch.h"
#include "HorizontalLayout.h"




sizer GUI::Elements::layouts::horizontal::update_layout(sizer r, float scale)
{
    return base::update_layout(r, scale);
    /*if (!need_update_layout)
    {
    	for (auto c : childs)
    		c->update_layout();
    	return;
    }

    rect bounds = render_bounds.get();


    bounds.x += padding->left;
    bounds.y += padding->top;
    bounds.w -= padding->right + padding->left;
    bounds.h -= padding->bottom + padding->top;
    rect orig_bounds = bounds;

    for (auto &c : childs)
    {
    	if (!c->visible)
    		continue;

    	const sizer &m = c->margin;
    	rect new_bounds = c->render_bounds.get();


    	if (c->docking == dock::LEFT || c->docking == dock::TOP)
    	{
    		new_bounds = { bounds.x + m.left, bounds.y + m.top, c->size->x, bounds.h - m.top - m.bottom };
    		float iWidth = m.left + m.right + c->size->x;
    		bounds.x += iWidth;
    		bounds.w -= iWidth;
    	}

    	if (c->docking == dock::RIGHT || c->docking == dock::BOTTOM)
    	{
    		new_bounds = { (bounds.x + bounds.w) - c->size->x - m.right, bounds.y + m.top, c->size->x, bounds.h - m.top - m.bottom };
    		float iWidth = m.left + m.right + c->size->x;
    		bounds.w -= iWidth;
    	}

    	c->render_bounds = new_bounds;
    }
    for (auto &c : childs)
    {

    	if (!c->visible)
    		continue;
    	rect new_bounds = c->render_bounds.get();
    	const sizer &m = c->margin;

    	if ((c->docking == dock::NONE))
    	{
    		new_bounds = { bounds.x + m.left, bounds.y + m.top, c->size->x, bounds.h - m.top - m.bottom };
    		float iWidth = m.left + m.right + c->size->x;
    		bounds.x += iWidth;
    		bounds.w -= iWidth;
    	}
    	c->render_bounds = new_bounds;
    }

    for (auto &c : childs)
    {
    	if (!c->visible)
    		continue;

    	if ((c->docking == dock::FILL))
    	{
    		const sizer &m = c->margin;
    		c->render_bounds = { bounds.x + m.left, bounds.y + m.top, bounds.w - m.left - m.right, bounds.h - m.top - m.bottom };
    	}

    }		need_update_layout = false;
    for (auto c : childs)
    	c->update_layout();*/
}

void GUI::Elements::layouts::horizontal::on_bounds_changed(const rect& r)
{
    base::on_bounds_changed(r);
    //label_text->size = vec2(label_text->size->x,80);
}

GUI::Elements::layouts::horizontal::horizontal()
{
    size = { 0, 0 };
    width_size = size_type::MATCH_CHILDREN;
    height_size = size_type::MATCH_CHILDREN;
}

void GUI::Elements::layouts::horizontal::add_child(base::ptr obj)
{
    base::add_child(obj);
    obj->docking = dock::LEFT;
}
