module GUI:FlowGraph.FlowManager;

import :FlowGraph.Canvas;
import :Renderer;

namespace GUI::Elements::FlowGraph
{
	manager::manager()
	{
		visible = false;
	}

	std::map<::FlowGraph::graph::ptr, tab_button::ptr> manager::get_all()
	{
		return canvases;
	}

	void manager::on_edit(link_item::ptr item)
	{
		if (edit)
		{
			auto l = edit->line.lock();
			if (l) l->draw_helper = false;
		}

		{
			auto l = item->line.lock();
			if (l) l->draw_helper = true;
		}

		edit = item;
	}

	void manager::add_graph(::FlowGraph::graph::ptr from, ::FlowGraph::graph::ptr g)
	{
		tab_control::ptr tabs = canvases[from]->owner.lock();
		canvas::ptr canva(new canvas(this));
		canva->init(g.get());
		g->name = "some graph";
		tab_button::ptr but(new tab_button());
		but->get_label()->text = g->name;
		but->page = canva;
		canvases[g] = but;
		tabs->add_button(but);
	}

	tab_button::ptr manager::add_graph(::FlowGraph::graph::ptr g)
	{
		if (!g) return nullptr;

		canvas::ptr canva(new canvas(this));
		canva->init(g.get());
		g->name = "some graph";
		tab_button::ptr but(new tab_button());
		but->get_label()->text = g->name;
		but->page = canva;
		canvases[g] = but;
		return but;
	}
}
