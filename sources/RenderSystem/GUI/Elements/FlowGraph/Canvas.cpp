module GUI:FlowGraph.Canvas;
import RenderSystem;

import <windows/windows.h>;

import  :FlowGraph.FlowManager;

import :FlowSystem;
import :MenuList;
import :ScrollContainer;
import :Renderer;
import :Skin;
import Graphics;
using namespace HAL;
GUI::Elements::FlowGraph::link_item::link_item()
{
	drag_listener = true;
	set_package("link_item");
	skin = Skin::get().DefaultOptionBox;
}

void GUI::Elements::FlowGraph::link_item::update()
{
	if (type == link_type::LINK_IN)
		inserted = !p->input_connections.empty();

	if (type == link_type::LINK_OUT)
		inserted = !p->output_connections.empty();

	set_checked(inserted);
}

void GUI::Elements::FlowGraph::link_spline::set_selected(bool value)
{
	if (selected != value)
	{
		if (value)
			phase = 0;

		selected = value;
	}
}

namespace
{
	float2 cb_eval(float2 a, float2 b, float2 c, float2 d, float t)
	{
		float u = 1.0f - t;
		return u*u*u*a + 3.0f*u*u*t*b + 3.0f*u*t*t*c + t*t*t*d;
	}

	float2 cb_deriv(float2 a, float2 b, float2 c, float2 d, float t)
	{
		float u = 1.0f - t;
		return 3.0f*u*u*(b - a) + 6.0f*u*t*(c - b) + 3.0f*t*t*(d - c);
	}

	float2 cb_deriv2(float2 a, float2 b, float2 c, float2 d, float t)
	{
		return 6.0f*(1.0f - t)*(c - 2.0f*b + a) + 6.0f*t*(d - 2.0f*c + b);
	}

	float cb_dot(float2 a, float2 b) { return a.x*b.x + a.y*b.y; }

	class toolbar_panel : public GUI::base
	{
	public:
		void draw(GUI::base::Context& c) override
		{
			GUI::Texture shadow = Skin::get().Shadow;
			shadow.margins = { 12, 12, 12, 12 };
			c.renderer->draw(c, shadow, get_render_bounds());

			GUI::Texture bg = Skin::get().FlowWindow.Inactive;
			bg.mul_color = float4(22, 28, 38, 255) / 255.0f;
			c.renderer->draw(c, bg, get_render_bounds());
		}
	};

	// Recursive de Casteljau rect intersection — curve ⊆ convex hull ⊆ AABB of control polygon
	bool cb_vs_rect(float2 a, float2 b, float2 c, float2 d,
	                float x0, float y0, float x1, float y1, int depth)
	{
		float2 mn = float2::min(float2::min(a, b), float2::min(c, d));
		float2 mx = float2::max(float2::max(a, b), float2::max(c, d));

		// Reject: control-polygon AABB entirely outside rect
		if (mx.x < x0 || mn.x > x1 || mx.y < y0 || mn.y > y1)
			return false;

		// Accept: control-polygon AABB entirely inside rect
		if (mn.x >= x0 && mx.x <= x1 && mn.y >= y0 && mx.y <= y1)
			return true;

		// At max depth the sub-curve AABB is tiny and still overlaps — treat as hit
		if (depth >= 8)
			return true;

		// Split at t = 0.5 with de Casteljau
		float2 ab   = (a + b) * 0.5f;
		float2 bc   = (b + c) * 0.5f;
		float2 cd   = (c + d) * 0.5f;
		float2 abc  = (ab + bc) * 0.5f;
		float2 bcd  = (bc + cd) * 0.5f;
		float2 abcd = (abc + bcd) * 0.5f;

		return cb_vs_rect(a, ab, abc, abcd, x0, y0, x1, y1, depth + 1) ||
		       cb_vs_rect(abcd, bcd, cd, d, x0, y0, x1, y1, depth + 1);
	}
}

bool GUI::Elements::FlowGraph::link_spline::test(vec2 from, vec2 to)
{
	return cb_vs_rect(p1, p2, p3, p4, from.x, from.y, to.x, to.y, 0);
}

bool GUI::Elements::FlowGraph::link_spline::test(vec2 p)
{
	constexpr float THRESHOLD    = 10.0f;
	constexpr float THRESHOLD_SQ = THRESHOLD * THRESHOLD;

	// Quick reject using control-polygon AABB expanded by threshold
	float2 mn = float2::min(float2::min(p1, p2), float2::min(p3, p4));
	float2 mx = float2::max(float2::max(p1, p2), float2::max(p3, p4));
	if (p.x < mn.x - THRESHOLD || p.x > mx.x + THRESHOLD ||
	    p.y < mn.y - THRESHOLD || p.y > mx.y + THRESHOLD)
		return false;

	// Coarse scan: 10 uniform samples to find approximate closest t
	float best_t   = 0.0f;
	float best_dsq = std::numeric_limits<float>::max();
	for (int i = 0; i <= 10; i++)
	{
		float t   = float(i) / 10.0f;
		float2 bt = cb_eval(p1, p2, p3, p4, t);
		float2 d  = bt - float2(p);
		float dsq = cb_dot(d, d);
		if (dsq < best_dsq) { best_dsq = dsq; best_t = t; }
	}

	// Newton refinement: minimise |B(t) - P|²
	// derivative = (B(t)-P)·B'(t), second derivative = |B'(t)|² + (B(t)-P)·B''(t)
	for (int iter = 0; iter < 4; iter++)
	{
		float2 bt  = cb_eval  (p1, p2, p3, p4, best_t);
		float2 bd  = cb_deriv (p1, p2, p3, p4, best_t);
		float2 bd2 = cb_deriv2(p1, p2, p3, p4, best_t);
		float2 diff = bt - float2(p);

		float num   = cb_dot(diff, bd);
		float denom = cb_dot(bd, bd) + cb_dot(diff, bd2);
		if (std::abs(denom) < 1e-6f) break;

		best_t = Math::clamp(best_t - num / denom, 0.0f, 1.0f);
	}

	float2 closest = cb_eval(p1, p2, p3, p4, best_t);
	float2 d = closest - float2(p);
	return cb_dot(d, d) < THRESHOLD_SQ;
}

void GUI::Elements::FlowGraph::link_spline::update(float dt)
{
	p1 = vec2(from->get_render_bounds().pos) + vec2(from->get_render_bounds().size) / 2;
	p4 = vec2(to->get_render_bounds().pos) + vec2(to->get_render_bounds().size) / 2;
	float d = abs(p1.x - p4.x);
	float delta = std::max(15.f, d) / 3;
	p2 = p1 + vec2(delta, 0);
	p3 = p4 - vec2(delta, 0);

	if (selected)
	{
		float l = 0.5f + 0.5f * sin(Math::m_2_pi * (phase += dt));
		color = vec4(50, 150, 200, 255) / 255.0f * l + (1 - l) * vec4(16, 46, 100, 255) / 255.0f;
	}

	else
		color = vec4(26, 96, 146, 255) / 255.0f;
}

void GUI::Elements::FlowGraph::canvas::on_remove()
{
	scroll_container::on_remove();
}

void GUI::Elements::FlowGraph::canvas::draw(Context& c)
{
	

	g->cam_pos = contents->pos;
	c.renderer->flush(c);
	auto clip = c.scissors;
	c.command_list->get_graphics().set_scissors(c.ui_clipping);

	Slots::FlowGraph graph_data;
	graph_data.GetSize() = vec4(render_bounds->size, user_ui->size.get());
	graph_data.GetOffset_size() = vec4(contents->pos.get(), 1.0f / contents->scale, 0);
	graph_data.GetInv_pixel() = vec2(1, 1) / user_ui->size.get();
	c.command_list->get_graphics().set(graph_data);

	c.renderer->draw(c, RenderSystem::get().device().get_engine_pso_holder().GetPSO<PSOS::CanvasBack>(), get_render_bounds());


	if (linking.size())
	{

		c.renderer->flush(c);
		//	auto& b = *line_vertex;
		int count = 0;

		std::vector<Table::VSLine> vertexes;
		vertexes.resize(4 * linking.size());

		for (auto& l : linking)
		{
			auto& p = *l;
			p.update(c.delta_time);
			vertexes[4 * count + 0].pos = p.p1 / user_ui->size.get();
			vertexes[4 * count + 0].color = p.color;
			vertexes[4 * count + 1].pos = p.p2 / user_ui->size.get();
			vertexes[4 * count + 1].color = p.color;
			vertexes[4 * count + 2].pos = p.p3 / user_ui->size.get();
			vertexes[4 * count + 2].color = p.color;
			vertexes[4 * count + 3].pos = p.p4 / user_ui->size.get();
			vertexes[4 * count + 3].color = p.color;
			count++;
		}

		c.command_list->get_graphics().set_pipeline<PSOS::CanvasLines>();
		c.command_list->get_graphics().set_topology(HAL::PrimitiveTopologyType::PATCH, HAL::PrimitiveTopologyFeed::LIST, false, 4);

		auto data = c.command_list->place_data(sizeof(Table::VSLine) * vertexes.size(), sizeof(Table::VSLine));
		c.command_list->write<Table::VSLine>(data, vertexes);

		auto view = data.resource->create_view<HAL::StructuredBufferView<Table::VSLine>>(*c.command_list, StructuredBufferViewDesc{ (UINT)data.resource_offset, (UINT)data.size,counterType::NONE });
		{
			Slots::LineRender linedata;
			linedata.GetVb() = view;
			c.command_list->get_graphics().set(linedata);
		}
		c.command_list->get_graphics().draw(count * 4, 0);
	}

	c.command_list->get_graphics().set_scissors(clip);
	//  this->context.renderer->draw(this, c);
}

void GUI::Elements::FlowGraph::canvas::on_register(::FlowGraph::window* w)
{
	::FlowGraph::Node* node = dynamic_cast<::FlowGraph::Node*>(w);

	if (node)
	{
		if (node == g)
		{
			graph_in = std::make_shared<GUI::Elements::FlowGraph::component_window>(this, node, window_type::GRAPH_IN);
			graph_out = std::make_shared<GUI::Elements::FlowGraph::component_window>(this, node, window_type::GRAPH_OUT);
			add_child(graph_in);
			add_child(graph_out);
			return;
		}

		if (!(node->get_graph() == g))
			return;

		if (node->pos == vec2(10, 10))
			g->place_node(node);

		auto elem = std::make_shared<GUI::Elements::FlowGraph::component_window>(this, node, window_type::NODE);
		nodes[node] = (elem);
		run_on_ui([this, elem]() {   add_child(elem); });
	}

	else
	{
		if (!(w->get_graph() == g))
			return;

		auto elem = std::make_shared<GUI::Elements::FlowGraph::comment>(this, w);
		windows[w] = elem;
		run_on_ui([this, elem]() {     comments->add_child(elem); });
		//    comments->add_child(elem);
	}
}

void GUI::Elements::FlowGraph::canvas::on_remove(::FlowGraph::window* w)
{
	::FlowGraph::Node* node = dynamic_cast<::FlowGraph::Node*>(w);

	if (node)
	{
		if (node == g)
		{
			// remove_child(graph_in); graph_in = nullptr;
			// remove_child(graph_out); graph_out = nullptr;
			return;
		}

		if (!(node->get_graph() == g || node == g))
			return;

		run_on_ui([this, node]()
			{
				auto elem = nodes[node];
				elem->remove_from_parent();
				nodes.erase(node);
			});
	}

	else
	{
		if (!(w->get_graph() == g || w == g))
			return;

		run_on_ui([this, w]()
			{
				auto elem = windows[w];
				elem->remove_from_parent();
				windows.erase(w);
			});
	}

	//nodes[node] = nullptr;
}

void GUI::Elements::FlowGraph::canvas::on_add_input(::FlowGraph::parameter* p)
{

	run_on_ui([this, p]()
		{
			auto node = p->owner;

			if (node == g)
			{
				links[p] = graph_in->add_output(p);
				link_node[p] = graph_in;
				return;
			}

			if (!(node->get_graph() == g || node == g))
				return;

			auto elem = nodes[node];
			links[p] = elem->add_input(p);
			link_node[p] = elem;

		});

}

void GUI::Elements::FlowGraph::canvas::on_remove_input(::FlowGraph::parameter* p)
{
	run_on_ui([this, p]()
		{
			auto node = p->owner;

			if (!(node->get_graph() == g || node == g))
				return;

			auto l = links[p]->line.lock();

			if (l)l->remove_from_parent();
			links[p] = nullptr;
		});
}

void GUI::Elements::FlowGraph::canvas::on_add_output(::FlowGraph::parameter* p)
{
	run_on_ui([this, p]()
		{  auto node = p->owner;

	if (node == g)
	{
		links[p] = graph_out->add_input(p);
		link_node[p] = graph_out;
		return;
	}

	if (!(node->get_graph() == g || node == g))
		return;

	auto elem = nodes[node];
	links[p] = elem->add_output(p);
	link_node[p] = elem;

		});
}

void GUI::Elements::FlowGraph::canvas::on_remove_output(::FlowGraph::parameter* p)
{
	run_on_ui([this, p]()
		{
			auto node = p->owner;

			if (!(node->get_graph() == g || node == g))
				return;

			auto l = links[p]->line.lock();

			if (l)
				l->remove_from_parent();
			links[p] = nullptr;
		});
}

void GUI::Elements::FlowGraph::canvas::on_link(::FlowGraph::parameter* p1, ::FlowGraph::parameter* p2)
{
	run_on_ui([this, p1, p2]()
		{
			link_item::ptr pp1, pp2;
			pp1 = links[p1];
			pp2 = links[p2];

			if (!pp1 || !pp2)
				return;

			//	pp1->inserted = true;
			link_spline::ptr s(new link_spline());
			s->from = pp1;
			s->to = pp2;
			linking.push_back(s);
			pp2->update();
			pp1->update();
		});
}

void GUI::Elements::FlowGraph::canvas::on_unlink(::FlowGraph::parameter* p1, ::FlowGraph::parameter* p2)
{
	run_on_ui([this, p1, p2]()
		{
			link_item::ptr pp1, pp2;
			pp1 = links[p1];
			pp2 = links[p2];

			if (!pp1 || !pp2)
				return;

			auto it = std::find_if(linking.begin(), linking.end(), [pp1, pp2](const link_spline::ptr& s)
				{
					return pp1 == s->from && pp2 == s->to;
				});

			if (it != linking.end())
				linking.erase(it);

			pp2->update();
			pp1->update();
		});
}

bool GUI::Elements::FlowGraph::canvas::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (action == mouse_action::DOWN && button == mouse_button::LEFT)
	{
		select_pos = (pos - vec2(contents->get_render_bounds().pos)) / contents->scale;
		selection->pos = select_pos;
		selection->size = { 0, 0 };
		selection->visible = true;
		set_movable(true);
	}

	if (selection->visible && (action == mouse_action::UP || action == mouse_action::CANCEL) && button == mouse_button::LEFT)
	{
		if (selection->size->x > 5 && selection->size->y > 5)
			on_select();
		else
		{
			bool f = false;

			for (auto it = linking.rbegin(); it != linking.rend(); ++it)
			{
				auto& p = *it;

				if (!f)
				{
					f = p->test(pos);
					p->selected = f;
				}

				else
					p->selected = false;
			}

			for (auto n : nodes)
			{
				if (!f)
				{
					f = n.second->test(pos);
					n.second->selected = f;
				}

				else
					n.second->selected = false;
			}

			if (!f)
			{
				f = graph_in->test(pos);
				graph_in->selected = f;
			}

			else
				graph_in->selected = false;

			if (!f)
			{
				f = graph_out->test(pos);
				graph_out->selected = f;
			}

			else
				graph_out->selected = false;
		}

		selection->visible = false;
		set_movable(false);
	}

	if (action == mouse_action::UP && button == mouse_button::RIGHT)
	{
		user_ui->close_menus();
		menu_list::ptr menu(new menu_list());
		auto& nodes = ::FlowGraph::FlowSystem::get().get_all();
		vec2 t = (pos - vec2(contents->get_render_bounds().pos)) / contents->scale;

		for (auto& n : nodes)
		{
			auto nptr = &n;
			menu->add_item(n.name)->on_click = [nptr, t, this](menu_list_element::ptr e)
			{
				::FlowGraph::window::ptr node = nptr->create();

				if (node)
				{
					node->pos = t;
					g->register_node(node);
				}
			};
		}

		/*
		for (auto &n : main_manager->get_all())
		{
			auto nptr = n.second;
			menu->add_item(n.first->name)->on_click = [nptr, t, this](menu_list_element::ptr e)
			{
				::FlowGraph::window::ptr node = Serializer::clone(*nptr->page->get_ptr<GUI::Elements::FlowGraph::canvas>()->g);// .get_ptr();
				node->pos = t;
				g->register_node(node);
			};
		}*/
		/*
		menu->add_item("ololo2");
		menu->add_item("ololo3");*/
		menu->pos = pos;// -vec2(render_bounds->pos);
		menu->self_open(user_ui);
	}

	focus();
	return true;
}

void GUI::Elements::FlowGraph::canvas::on_key_action(key_action action, long key)
{
	if (action != key_action::DOWN) return;
	if (key == VK_DELETE)
	{
		auto copy = linking;

		for (auto& p : copy)
		{
			if (p->selected)
				p->from->p->unlink(p->to->p->get_ptr());
		}

		auto nc = nodes;

		for (auto& p : nc)
		{
			if (p.second->selected)
				p.first->remove();
		}

	}

	if (key == VK_SPACE)
	{
		g->auto_layout();
		for (auto& [node, win] : nodes)
			win->pos = node->pos;
		graph_in->pos  = g->pos_in;
		graph_out->pos = g->pos_out;
	}
}

bool GUI::Elements::FlowGraph::canvas::on_wheel(mouse_wheel type, float value, vec2 pos)
{
	scale_speed += value * 0.1f;
	wheel_pos = pos;
	return true;
	//	contents->scale += pos.y / 10;
	//	contents->scale = Math::clamp(float(int(contents->scale * 100)) / 100.0f ,0.5f,2.0f);
}

void GUI::Elements::FlowGraph::canvas::think(float dt)
{

	if (abs(scale_speed) > 0.03f)
	{
		float pscale = contents->scale;
		scale_speed *= std::exp(-20 * dt);
		//	contents->scale += pos.y / 10;
		contents->scale *= 1 + scale_speed / 10.0f;
		contents->scale = Math::clamp(contents->scale, 0.1f, 1.0f);
		pscale = contents->scale / pscale;
		vec2 mp = wheel_pos - get_render_bounds().pos;
		contents->pos = pscale * (contents->pos.get() - mp) + mp;
		//	moving(100 * speed*dt);
	}
}
void GUI::Elements::FlowGraph::canvas::on_add(base* parent)
{
	scroll_container::on_add(parent);
}

GUI::Elements::FlowGraph::canvas::canvas(manager* main_manager)
{
	this->main_manager = main_manager;
	allow_overflow = true;
	docking = dock::FILL;
	contents->size = { 120, 120 };
	contents->clip_child = false;
	contents->scale = 1;
	//  contents->draw_helper = true;
	contents->map_to_pixels = false;
	//contents->width_size = size_type::MATCH_PARENT_CHILDREN;
	contents->height_size = size_type::FIXED;
	contents->width_size = size_type::FIXED;
	contents->docking = dock::NONE;
	selection.reset(new base());
	selection->draw_helper = true;
	selection->visible = false;
	selection->clickable = false;
	contents->add_child(selection);
	comments.reset(new base());
	comments->clamp_to_parent = ParentClamp::NONE;
	comments->clickable = false;
	comments->clip_child = false;
	comments->docking = dock::FILL;
	contents->add_child(comments);

	toolbar.reset(new toolbar_panel());
	toolbar->docking = dock::NONE;
	toolbar->pos = vec2(8, 8);
	toolbar->height_size = size_type::MATCH_CHILDREN;
	toolbar->width_size = size_type::MATCH_CHILDREN;
	toolbar->padding = { 4, 4, 4, 4 };

	auto rearrange_btn = std::make_shared<button>();
	rearrange_btn->get_label()->text = "Rearrange";
	rearrange_btn->docking = dock::TOP;
	rearrange_btn->size = { 100, 26 };
	rearrange_btn->on_click = [this](button::ptr) {
		g->auto_layout();
		for (auto& [node, win] : nodes)
			win->pos = node->pos;
		graph_in->pos  = g->pos_in;
		graph_out->pos = g->pos_out;
	};
	toolbar->add_child(rearrange_btn);
	base::add_child(toolbar);
}

bool GUI::Elements::FlowGraph::canvas::on_mouse_move(vec2 pos)
{
	if (selection->visible)
	{
		vec2 cur_pos = (pos - vec2(contents->get_render_bounds().pos)) / contents->scale;
		selection->pos = vec2::min(cur_pos, select_pos);
		selection->size = vec2::max(cur_pos, select_pos) - vec2::min(cur_pos, select_pos);

		if (selection->size->x > 5 && selection->size->y > 5)
			on_select();
	}

	return true;
}

void GUI::Elements::FlowGraph::canvas::on_select(component_window* w)
{
	for (auto n : nodes)
		n.second->selected = n.second.get() == w;

	for (auto n : linking)
		n->selected = false;

	graph_in->selected = graph_in.get() == w;
	graph_out->selected = graph_out.get() == w;
}

void GUI::Elements::FlowGraph::canvas::on_select()
{
	for (auto n : nodes)
		n.second->selected = n.second->test(selection->pos.get(), selection->pos.get() + selection->size.get());

	for (auto l : linking)
		l->selected = l->test(selection->get_render_bounds().pos, vec2(selection->get_render_bounds().pos) + vec2(selection->get_render_bounds().size));

	graph_in->selected = graph_in->test(selection->pos.get(), selection->pos.get() + selection->size.get());
	graph_out->selected = graph_out->test(selection->pos.get(), selection->pos.get() + selection->size.get());
}

bool GUI::Elements::FlowGraph::canvas::can_accept(drag_n_drop_package::ptr p)
{
	auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();
	return !!item;
}

bool GUI::Elements::FlowGraph::canvas::on_drop(drag_n_drop_package::ptr p, vec2 pos)
{
	auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();
	MyVariant v;
	v = item;

	if (item)
	{

		auto res = g->on_drop(v);

		vec2 t = (pos - vec2(contents->get_render_bounds().pos)) / contents->scale;

		for (auto& e : res)
			e->pos = t;
	}
	return true;
}

void GUI::Elements::FlowGraph::canvas::on_move(comment* w, vec2 dt)
{
	vec2 old_pos = w->pos.get() - dt;
	auto f = [w, old_pos, dt](base::ptr n)
	{
		//if (n->test(old_pos, old_pos + w->size.get()))
		{
			n->pos = n->pos.get() + dt;
		}
	};

	for (auto n : w->inner_components)
		f(n);

	/*
						for (auto n : nodes)
						f(n.second);

						f(graph_in);
						f(graph_out);
						*/
}

void GUI::Elements::FlowGraph::canvas::on_move(component_window* w, vec2 dt)
{
	auto f = [w, dt](component_window::ptr n)
	{
		if (n->selected && n.get() != w)
			n->pos = n->pos.get() + dt;
	};

	for (auto n : nodes)
		f(n.second);

	f(graph_in);
	f(graph_out);
}

void GUI::Elements::FlowGraph::canvas::on_start_drag(comment* w)
{
	std::vector<base::ptr> res;
	auto f = [w, &res](component_window::ptr n)
	{
		if (n->test_full(w->pos.get(), w->pos.get() + w->size.get()))
			res.push_back(n);
	};
	auto f2 = [w, &res](comment::ptr n)
	{
		if (n.get() != w && n->test_full(w->pos.get(), w->pos.get() + w->size.get()))
			res.push_back(n);
	};

	for (auto n : windows)
		f2(n.second);

	for (auto n : nodes)
		f(n.second);

	f(graph_in);
	f(graph_out);
	w->inner_components = res;
}

void GUI::Elements::FlowGraph::canvas::init(::FlowGraph::graph* g)
{
	this->g = g;
	g->add_listener(this, true);
	contents->pos = g->cam_pos;

	g->auto_layout();
	for (auto& [node, win] : nodes)
		win->pos = node->pos;
	if (graph_in)  graph_in->pos  = g->pos_in;
	if (graph_out) graph_out->pos = g->pos_out;

	if (on_open)
		on_open(g);
}

GUI::Elements::FlowGraph::canvas::~canvas()
{
	if (g && on_close)
		on_close(g);
}

/*
GUI::Elements::FlowGraph::canvas::renderer::renderer(GUI::Renderer* renderer)
{
	HAL::PipelineStateDesc state_desc;
	state_desc.root_signature = context.renderer->root_signature;
	state_desc.pixel = HAL::pixel_shader::get_resource({ "shaders\\gui\\canvas.hlsl", "PS", 0, {} });
	state_desc.vertex = HAL::vertex_shader::get_resource({ "shaders\\gui\\canvas.hlsl", "VS", 0, {} });
	state.reset(new HAL::PipelineState(state_desc));
	{
		HAL::PipelineStateDesc state_desc;
		state_desc.root_signature = context.renderer->root_signature;
		state_desc.pixel = HAL::pixel_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "PS", 0, {} });
		state_desc.vertex = HAL::vertex_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "VS", 0, {} });
		state_desc.geometry = HAL::geometry_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "GS", 0, {} });
		state_desc.domain = HAL::domain_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "DS", 0, {} });
		state_desc.hull = HAL::hull_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "HS", 0, {} });
		state_desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		state_desc.layout.inputs.push_back({ "SV_POSITION", 0, HAL::Format::R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		state_desc.layout.inputs.push_back({ "COLOR", 0, HAL::Format::R32G32B32A32_FLOAT, 0, sizeof(vec2), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		//   state_desc.layout.inputs.emplace_back({ "SV_POSITION", 0, 0, 0, HAL::Format::R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA });
		//   state_desc.layout.inputs.emplace_back({ "COLOR", 0, sizeof(vec2), 0, HAL::Format::R32G32B32A32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA });
		line_state.reset(new HAL::PipelineState(state_desc));
		inv_pixel.reset(new HAL::ConstBuffer<vec2>());
		line_vertex.reset(new HAL::Buffer<line_vertexes>(HAL::HeapType::DEFAULT, 512));
		gs_table = HAL::DescriptorHeapManager::get().get_csu()->create_table(1);
		inv_pixel->place(gs_table[0]);
	}

}

void GUI::Elements::FlowGraph::canvas::renderer::render(Object* obj, Data* data, Context& c)
{
	auto cb_vertex = data->cb_vertex;
	auto cb_pixel = data->cb_pixel;
	auto vertex_table = data->vertex_table;
	auto pixel_table = data->pixel_table;
	rect bounds = obj->get_render_bounds();
	bounds += {c.offset, 0, 0};
	(*cb_vertex)[0].p1 = vec2(bounds.pos) / obj->get_user_ui()->size.get();
	(*cb_vertex)[0].p2 = (vec2(bounds.pos) + vec2(bounds.size)) / obj->get_user_ui()->size.get();
	(*cb_pixel)[0].clipping = vec4(c.ui_clipping.left, c.ui_clipping.top, -c.ui_clipping.right, -c.ui_clipping.bottom);
	(*cb_pixel)[0].size = vec4(obj->render_bounds->size, obj->user_ui->size.get());
	(*cb_pixel)[0].offset_scale = vec3(obj->contents->pos.get(), 1.0f / obj->contents->scale);
	data->update(c.command_list);
	c.command_list->get_graphics().set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	c.command_list->get_graphics().set_pipeline(state);
	c.command_list->get_graphics().set(0, vertex_table);
	c.command_list->get_graphics().set(1, pixel_table);
	c.command_list->get_graphics().draw(4, 0);
	//  return;
	(*inv_pixel)[0] = vec2(1, 1) / obj->user_ui->size.get(); //vec2(obj->contents->scale, obj->contents->scale) / obj->user_ui->size.get();
	{
		auto& b = *line_vertex;
		int count = 0;


		for (auto& l : obj->linking)
		{
			auto& p = *l;
			p.update(c.delta_time);
			b[4 * count + 0].p = p.p1 / obj->user_ui->size.get();
			b[4 * count + 0].color = p.color;
			b[4 * count + 1].p = p.p2 / obj->user_ui->size.get();
			b[4 * count + 1].color = p.color;
			b[4 * count + 2].p = p.p3 / obj->user_ui->size.get();
			b[4 * count + 2].color = p.color;
			b[4 * count + 3].p = p.p4 / obj->user_ui->size.get();
			b[4 * count + 3].color = p.color;
			count++;
		}

		//    line_vertex->update(c.command_list);
		c.command_list->get_graphics().set_pipeline(line_state);
		c.command_list->get_graphics().set(4, gs_table);
		c.command_list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		c.command_list->get_graphics().set_vertex_buffers(0, line_vertex);
		c.command_list->get_graphics().draw(count * 4, 0);

	}
}*/
/*
void GUI::Elements::FlowGraph::link_item::draw(Context& c)
{
//   context.renderer->draw(this, c);
}*/

bool GUI::Elements::FlowGraph::link_item::need_drag_drop()
{
	return dynamic_cast<::FlowGraph::output*>(p) || dynamic_cast<::FlowGraph::graph_input*>(p);
}

bool GUI::Elements::FlowGraph::link_item::can_accept(drag_n_drop_package::ptr e)
{
	if (e->name != "link_item")
		return false;

	auto l = e->element.lock()->get_ptr<link_item>();
	return p->can_link(l->p);
}

bool GUI::Elements::FlowGraph::link_item::on_drop(drag_n_drop_package::ptr e, vec2)
{
	//throw std::exception("The method or operation is not implemented.");
	auto l = e->element.lock()->get_ptr<link_item>();
	l->p->link(p->get_ptr());
	return true;
}

void GUI::Elements::FlowGraph::link_item::on_dragdrop_start(drag_n_drop_package::ptr e)
{
	if (!beaming && can_accept(e))
	{
		beaming = true;
		beam_time = 0;
	}
}

void GUI::Elements::FlowGraph::link_item::on_dragdrop_end()
{
	beaming = false;
}

bool GUI::Elements::FlowGraph::link_item::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (can_delete && button == mouse_button::RIGHT)
	{
		if (action == mouse_action::UP)
		{
			menu_list::ptr menu(new menu_list());
			menu->add_item("remove")->on_click = [this](menu_list_element::ptr e)
			{
				p->remove();
			};
			/*
			menu->add_item("ololo2");
			menu->add_item("ololo3");*/
			menu->pos = pos;// -vec2(render_bounds->pos);
			menu->self_open(user_ui);
		}

		return true;
	}

	if (button == mouse_button::LEFT || action == mouse_action::UP)
	{
		auto l = line.lock();

		if (l)      l->draw_helper = true;
		FlowGraph::manager::get().on_edit(get_ptr<link_item>());
	}

	return true;
}


