#include "pch_render.h"
#include "Canvas.h"
#include "FlowSystem.h"

#include "GUI/Elements/AssetExplorer.h"
#include "GUI/Elements/FlowGraph/FlowManager.h"
#include "GUI/Elements/MenuList.h"
#include "GUI/Elements/ScrollContainer.h"
#include "GUI/Renderer/Renderer.h"


void GUI::Elements::FlowGraph::canvas::draw(Graphics::context& c)
{
	g->cam_pos = contents->pos;
	renderer->flush(c);
	auto clip = c.scissors;
	c.command_list->get_graphics().set_scissors(c.ui_clipping);

	Slots::FlowGraph graph_data;
	graph_data.GetSize() = vec4(render_bounds->size, user_ui->size.get());
	graph_data.GetOffset_size() = vec4(contents->pos.get(), 1.0f / contents->scale, 0);
	graph_data.GetInv_pixel() = vec2(1, 1) / user_ui->size.get();
	graph_data.set(c.command_list->get_graphics());

	renderer->draw(c, GetPSO<PSOS::CanvasBack>(), get_render_bounds());


	if (linking.size())
	{

		renderer->flush(c);
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

		c.command_list->get_graphics().set_pipeline(GetPSO<PSOS::CanvasLines>());
		c.command_list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		auto data = c.command_list->place_data(sizeof(Table::VSLine) * vertexes.size(), sizeof(Table::VSLine));
		c.command_list->write<Table::VSLine>(data, vertexes);

		auto view = data.resource->create_view<StructuredBufferView<Table::VSLine>>(*c.command_list->frame_resources, StructuredBufferViewDesc{ (UINT)data.offset, (UINT)data.size,false });
		{
			Slots::LineRender linedata;
			linedata.GetVb() = view.structuredBuffer;
			linedata.set(c.command_list->get_graphics());
		}
		c.command_list->get_graphics().draw(count * 4, 0);
	}

	c.command_list->get_graphics().set_scissors(clip);
	//  this->renderer->draw(this, c);
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

void GUI::Elements::FlowGraph::canvas::on_key_action(long key)
{
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

	//   if (key == VK_HOME)

	  //     FileSystem::get().save_data(L"graph.flg", Serializer::serialize(*g));
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

	g->auto_layout();
	if (abs(scale_speed) > 0.03f)
	{
		float pscale = contents->scale;
		scale_speed *= expf(-20 * dt);
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
	comments->clip_to_parent = ParentClip::NONE;
	comments->clickable = false;
	comments->clip_child = false;
	comments->docking = dock::FILL;
	contents->add_child(comments);
	//comments->to_front();
	//  clip_child = true;
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
}

/*
GUI::Elements::FlowGraph::canvas::renderer::renderer(GUI::Renderer* renderer)
{
	Graphics::PipelineStateDesc state_desc;
	state_desc.root_signature = renderer->root_signature;
	state_desc.pixel = Graphics::pixel_shader::get_resource({ "shaders\\gui\\canvas.hlsl", "PS", 0, {} });
	state_desc.vertex = Graphics::vertex_shader::get_resource({ "shaders\\gui\\canvas.hlsl", "VS", 0, {} });
	state.reset(new Graphics::PipelineState(state_desc));
	{
		Graphics::PipelineStateDesc state_desc;
		state_desc.root_signature = renderer->root_signature;
		state_desc.pixel = Graphics::pixel_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "PS", 0, {} });
		state_desc.vertex = Graphics::vertex_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "VS", 0, {} });
		state_desc.geometry = Graphics::geometry_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "GS", 0, {} });
		state_desc.domain = Graphics::domain_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "DS", 0, {} });
		state_desc.hull = Graphics::hull_shader::get_resource({ "shaders\\gui\\flow_line.hlsl", "HS", 0, {} });
		state_desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		state_desc.layout.inputs.push_back({ "SV_POSITION", 0, Graphics::Format::R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		state_desc.layout.inputs.push_back({ "COLOR", 0, Graphics::Format::R32G32B32A32_FLOAT, 0, sizeof(vec2), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		//   state_desc.layout.inputs.emplace_back({ "SV_POSITION", 0, 0, 0, Graphics::Format::R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA });
		//   state_desc.layout.inputs.emplace_back({ "COLOR", 0, sizeof(vec2), 0, Graphics::Format::R32G32B32A32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA });
		line_state.reset(new Graphics::PipelineState(state_desc));
		inv_pixel.reset(new Graphics::ConstBuffer<vec2>());
		line_vertex.reset(new Graphics::Buffer<line_vertexes>(Graphics::HeapType::DEFAULT, 512));
		gs_table = Graphics::DescriptorHeapManager::get().get_csu()->create_table(1);
		inv_pixel->place(gs_table[0]);
	}

}

void GUI::Elements::FlowGraph::canvas::renderer::render(Object* obj, Data* data, Graphics::context& c)
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
	c.command_list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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
void GUI::Elements::FlowGraph::link_item::draw(Graphics::context& c)
{
//   renderer->draw(this, c);
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


