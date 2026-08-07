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
		auto owner_tab = canvases[from];
		if (!owner_tab) return;

		tab_control::ptr tabs = owner_tab->owner.lock();
		if (!tabs) return;

		auto canva = create_canvas(g);
		if (!canva) return;

		canvases[g] = tabs->add_page(g->name, canva);
	}

	canvas::ptr manager::create_canvas(::FlowGraph::graph::ptr g)
	{
		if (!g) return nullptr;

		canvas::ptr canva(new canvas(this));
		canva->init(g.get());
		g->name = "some graph";
		return canva;
	}

	void manager::register_tab(::FlowGraph::graph::ptr g, tab_button::ptr tab)
	{
		canvases[g] = tab;
	}
}
