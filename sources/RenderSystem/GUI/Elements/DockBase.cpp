#include "pch_render.h"
#include "DockBase.h"
#include "TabControl.h"
#include "GUI/Elements/Window.h"


void GUI::Elements::dock_base::draw_after(Graphics::context& c)
{
	if (virt->visible.get() && virt_base->get_parent())
		renderer->draw_virtual(virt, c);
}


void GUI::Elements::dock_base::on_drop_move(drag_n_drop_package::ptr e, vec2 p)
{
	virt->docking = get_docking(p);
	virt->pos = p - vec2(render_bounds->pos);
	virt->visible = virt->docking.get() != dock::NONE;
	// on_drop(e, p);
}

bool GUI::Elements::dock_base::on_drop(drag_n_drop_package::ptr e, vec2 p)
{

	dock d = get_docking(p);

	if (d == dock::FILL)
		get_tabs()->add_button(e->element.lock()->get_ptr<tab_button>());
	else if (d == dock::NONE)
	{
		window::ptr w(new window);
		w->pos = p;
		w->size = { 200, 200 };
		user_ui->add_child(w);
		dock_base::ptr dock(new dock_base);
		w->add_child(dock);
		auto el = e->element.lock()->get_ptr<tab_button>();
		dock->get_tabs()->add_button(el);
		dock->get_tabs()->remove_if_empty = true;
		auto ww = w.get();
		auto ui = user_ui;
		dock->on_empty.register_handler(w.get(), [ww, ui](bool)
			{
				ui->add_task([ww] {   ww->remove_from_parent(); });
			});
	}

	//get_tabs()->add_button(e->element.lock()->get_ptr<tab_button>());
	else
	{
		auto tab = e->element.lock();
		if (tab)
			get_dock(d)->get_tabs()->add_button(tab->get_ptr<tab_button>());
		//		get_dock(d)->get_tabs()->;
	}

	return true;
}

void GUI::Elements::dock_base::on_drop_leave(drag_n_drop_package::ptr p)
{

	virt_base->remove_from_parent();

}

void GUI::Elements::dock_base::on_drop_enter(drag_n_drop_package::ptr p)
{
	virt->size = vec2(render_bounds->size) / 4;
	add_child(virt_base);
}

bool GUI::Elements::dock_base::can_accept(drag_n_drop_package::ptr package)
{
	return package->name == "tab_button";
}

GUI::Elements::dock_base::dock_base()
{
	virt_base.reset(new base());
	virt_base->docking = dock::FILL;
	docking = dock::FILL;
	size = { 50, 50 };
	virt.reset(new base());
	virt->size = { 50, 50 };
	virt_base->clickable = false;
	virt->clickable = false;
	//virt_base->visible = false;
	virt_base->add_child(virt);
}

GUI::dock GUI::Elements::dock_base::get_docking(vec2 p)
{
	p -= vec2(render_bounds->pos);

	if (p.x < render_bounds->w / 4)
		return dock::LEFT;
	else if (p.x > 3 * render_bounds->w / 4)
		return dock::RIGHT;
	else if (p.y < render_bounds->h / 4)
		return dock::TOP;
	else if (p.y > 3 * render_bounds->h / 4)
		return dock::BOTTOM;
	else if ((p.x < 2.5f * render_bounds->w / 4)
		&& (p.x > 1.5f * render_bounds->w / 4)
		&& (p.y < 2.5f * render_bounds->h / 4)
		&& (p.y > 1.5f * render_bounds->h / 4))
		return dock::FILL;
	else
		return dock::NONE;
}

GUI::Elements::dock_base::ptr GUI::Elements::dock_base::get_dock(dock d)
{
	/* auto fun = [this](dock d)->GUI::Elements::dock_base::ptr &
	 {
		 if (d == dock::LEFT)
		 {
			 return left;
		 }
		 else if (d == dock::RIGHT)
		 {
			 return right;
		 }
		 else if (d == dock::TOP)
		 {
			 return top;
		 }
		 else //if (d == dock::BOTTOM)
		 {
			 return bottom;
		 }
	 };*/
	GUI::Elements::dock_base::ptr res;// = fun(d);

	if (!res)
	{
		res.reset(new dock_base());
		res->docking = d;
		res->size = vec2(render_bounds->size) / 4;
		res->original = false;
		add_child(res);
	}

	if (!res->get_parent())
		add_child(res);

	return res;
}

GUI::Elements::tab_control::ptr GUI::Elements::dock_base::get_tabs()
{
	if (!tabs)
	{
		tabs.reset(new GUI::Elements::tab_control());
		tabs->docking = dock::FILL;
		tabs->remove_if_empty = true;
		add_child(tabs);
	}

	if (!tabs->get_parent())
		add_child(tabs);

	return tabs;
}


void GUI::Elements::dock_base::remove_child(base::ptr obj)
{
	base::remove_child(obj);
	bool need_test = false;

	if (std::dynamic_pointer_cast<dock_base>(obj))
		need_test = true;
	else if (std::dynamic_pointer_cast<tab_control>(obj))
		need_test = true;

	auto obj_tab = std::dynamic_pointer_cast<dock_base>(obj);

	if (!original && need_test)
	{
		for (auto it = childs.rbegin(); it != childs.rend(); ++it)
		{
			auto elem = std::dynamic_pointer_cast<dock_base>(*it);

			if (elem)
			{
				if (obj->docking == dock::FILL)
					elem->docking = dock::FILL;

				return;
			}
		}

		if (!tabs || !tabs->get_parent())
		{
			on_empty(true);
			remove_from_parent();
		}
	}

	else if (childs.empty())     on_empty(true);
}

void GUI::Elements::dock_base::on_dock_changed(const dock& r)
{
	base::on_dock_changed(r);
	update_resizer();
}
