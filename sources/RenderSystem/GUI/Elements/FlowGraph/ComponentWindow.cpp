module GUI:FlowGraph.ComponentWindow;

import :FlowGraph.FlowManager;
import :FlowGraph.Canvas;
import :Renderer;


void GUI::Elements::FlowGraph::component_window::draw(Graphics::context& c)
{
	auto texture = selected ? skin.Active : skin.Inactive;

	if (visual_graph)
		texture.mul_color = visual_graph->color;// float4(31, 63, 85, 1);
	else
		texture.mul_color = float4(31, 63, 85, 255) / 255.0f;

	this->renderer->draw(c, texture, get_render_bounds());
}

GUI::Elements::FlowGraph::component_window::component_window(canvas* canva, ::FlowGraph::Node* node, window_type type)
{
	thinkable = true;
	this->node = node;
	this->type = type;
	this->canva = canva;
	skin = Skin::get().FlowWindow;
	size = { 100, 40 };
	margin = { 20, 20, 20, 20 };
	clickable = true;
	clip_child = true;
	label_text.reset(new label());
	label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
	label_text->docking = dock::LEFT;
	title_bar.reset(new dragger());
	title_bar->size = vec2(0, 28);
	title_bar->width_size = size_type::MATCH_PARENT_CHILDREN;
	title_bar->docking = dock::TOP;
	//holder.reset(this);
	title_bar->target = this;
	//title_bar->draw_helper = true;
	title_bar->on_move = [canva, this](vec2, vec2 dt)
	{
		canva->on_move(this, dt);
	};

	if (type == window_type::NODE)
	{
		pos = node->pos;
		label_text->text = node->name;
		title_bar->padding = { 5, 0, 0, 0 };
	}

	if (type == window_type::GRAPH_IN)
	{
		pos = dynamic_cast<::FlowGraph::graph*>(node)->pos_in;
		label_text->text = "graph input";
		title_bar->padding = { 5, 0, 5, 0 };
	}

	if (type == window_type::GRAPH_OUT)
	{
		pos = dynamic_cast<::FlowGraph::graph*>(node)->pos_out;
		label_text->text = "graph output";
		title_bar->padding = { 5, 0, 5, 0 };
	}

	base::add_child(title_bar);
	//title_bar->margin = {};
	minimal_size = { 50, 50 };
	/*  if (type == window_type::NODE)
	  {
		  close_button.reset(new button());
		  close_button->size = { 25, 25 };
		  close_button->padding = { 0, 0, 0, 0 };
		  close_button->background_style = button::view_style::NO_BACKGROUND;
		  close_button->get_label()->visible = false;
		  close_button->get_image()->visible = true;
		  close_button->get_image()->texture = HAL::Texture::get_resource(HAL::texure_header("textures/gui/window_close.png"));
		  close_button->get_image()->docking = dock::FILL;
		  close_button->padding = { 5, 5, 5, 5 };
		  close_button->docking = dock::RIGHT;
		  close_button->on_click = [this, node](button::ptr)
		  {
			  node->remove();
		  };
		  title_bar->add_child(close_button);
	  }
	*/
	title_bar->add_child(label_text);
	contents_left.reset(new base());
	contents_left->docking = dock::LEFT;
	contents_left->padding = { 5, 0, 5, 5 };
	contents_left->width_size = size_type::MATCH_CHILDREN;
	contents_left->height_size = size_type::MATCH_CHILDREN;
	contents_left->y_type = pos_y_type::TOP;
	contents_right.reset(new base());
	//  contents_right->docking = dock::LEFT;
//   if (type == window_type::GRAPH_IN)
	contents_right->docking = dock::RIGHT;
	contents_preview.reset(new base());
	contents_preview->docking = dock::LEFT;
	contents_preview->padding = { 5, 0, 5, 5 };
	//	preview->width_size = size_type::SQUARE;
	//	preview->height_size = size_type::MATCH_PARENT;
	contents_preview->size = { 64, 64 };
	contents_preview->y_type = pos_y_type::TOP;
	contents_preview->x_type = pos_x_type::CENTER;
	contents_preview->width_size = size_type::MATCH_CHILDREN;
	contents_preview->height_size = size_type::MATCH_CHILDREN;
	visual_graph = dynamic_cast<VisualGraph*>(node);
	if (visual_graph)
	{
		auto p = visual_graph->create_editor_window();

		if (p)
		{
			contents_preview->add_child(p);
			contents_right->docking = dock::LEFT;
		}
	}

	/*  preview.reset(new base());
	  preview->docking = dock::TOP;
	  preview->padding = { 5, 0, 5, 5 };
	  preview->size = { 64, 64 };
	  preview->y_type = pos_y_type::TOP;
	  preview->x_type = pos_x_type::CENTER;
	  preview->draw_helper = true;
	  contents_preview->add_child(preview);*/
	  // contents_preview->visible = false;
	contents_right->padding = { 5, 0, 5, 0 };
	contents_right->width_size = size_type::MATCH_CHILDREN;
	contents_right->height_size = size_type::MATCH_CHILDREN;
	contents_right->y_type = pos_y_type::TOP;
	base::add_child(contents_left);

	if (type != window_type::GRAPH_IN)
		base::add_child(contents_preview);

	base::add_child(contents_right);

	if (type == window_type::GRAPH_IN)
	{
		contents_preview->docking = dock::RIGHT;
		base::add_child(contents_preview);
	}

	width_size = size_type::MATCH_CHILDREN;
	height_size = size_type::MATCH_CHILDREN;
	//	contents_left->draw_helper = true;
	//contents_right->draw_helper = true;
}

GUI::Elements::FlowGraph::link_item::ptr GUI::Elements::FlowGraph::component_window::add_input(::FlowGraph::parameter* p)
{
	label::ptr info(new label());
	info->text = p->name;
	info->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
	info->docking = dock::LEFT;
	base::ptr l(new base());
	l->docking = dock::TOP;
	l->margin = { 0, 3, 0, 0 };
	l->width_size = size_type::MATCH_CHILDREN;
	l->height_size = size_type::MATCH_CHILDREN;
	l->x_type = pos_x_type::LEFT;
	//	l->draw_helper = true;
	link_item::ptr link(new link_item());
	link->docking = dock::LEFT;
	link->width_size = size_type::FIXED;
	link->height_size = size_type::FIXED;
	link->size = { 16, 16 };
	l->add_child(link);
	l->add_child(info);
	contents_left->add_child(l);
	link->p = p;
	link->type = link_type::LINK_IN;
	link->update();
	link->line = l;
	link->can_delete = type != window_type::NODE;
	return link;
}

GUI::Elements::FlowGraph::link_item::ptr GUI::Elements::FlowGraph::component_window::add_output(::FlowGraph::parameter* p)
{
	base::ptr l(new base());
	l->docking = dock::TOP;
	l->margin = { 0, 3, 0, 0 };
	l->width_size = size_type::MATCH_CHILDREN;
	l->height_size = size_type::MATCH_CHILDREN;
	l->x_type = pos_x_type::RIGHT;
	link_item::ptr link(new link_item());
	link->docking = dock::RIGHT;
	link->width_size = size_type::FIXED;
	link->height_size = size_type::FIXED;
	link->size = { 16, 16 };
	l->add_child(link);

	if (!p->name.empty())
	{
		label::ptr info(new label());
		info->text = p->name;
		info->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
		info->docking = dock::RIGHT;
		l->add_child(info);
	}

	contents_right->add_child(l);
	link->p = p;
	link->type = link_type::LINK_OUT;
	link->update();
	link->can_delete = type != window_type::NODE;
	link->line = l;
	return link;
}
/*
link_item::ptr GUI::Elements::FlowGraph::component_window::remove_input(::FlowGraph::parameter*p)
{
	canva->links[p]->remove_from_parent();
	canva->links[p] = nullptr;
}

link_item::ptr GUI::Elements::FlowGraph::component_window::remove_output(::FlowGraph::parameter*p)
{
	canva->links[p]->remove_from_parent();
	canva->links[p] = nullptr;
}
*/
void GUI::Elements::FlowGraph::component_window::on_touch()
{
	base::on_touch();

	if (!selected)
		canva->on_select(this);

	to_front();
}

void GUI::Elements::FlowGraph::component_window::on_pos_changed(const vec2& r)
{
	base::on_pos_changed(r);

	if (type == window_type::NODE)
		node->pos = pos.get();

	if (type == window_type::GRAPH_IN)
		dynamic_cast<::FlowGraph::graph*>(node)->pos_in = pos.get();

	if (type == window_type::GRAPH_OUT)
		dynamic_cast<::FlowGraph::graph*>(node)->pos_out = pos.get();
}

bool GUI::Elements::FlowGraph::component_window::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (button == mouse_button::RIGHT)
	{
		if (action == mouse_action::UP)
		{
			menu_list::ptr menu(new menu_list());

			if (type == window_type::GRAPH_IN)
				menu->add_item("add input")->on_click = [this](menu_list_element::ptr e)
			{
				node->register_input(/*::FlowGraph::data_types::INT,*/ "ololo");
			};

			if (type == window_type::GRAPH_OUT)
				menu->add_item("add output")->on_click = [this](menu_list_element::ptr e)
			{
				node->register_output(/*::FlowGraph::data_types::INT, */"ololo");
			};

			if (type == window_type::NODE && dynamic_cast<::FlowGraph::graph*>(node))
				menu->add_item("edit in new tab")->on_click = [this](menu_list_element::ptr e)
			{
				//node->register_output(::FlowGraph::data_types::INT, "ololo");
				canva->main_manager->add_graph(canva->g->get_ptr<::FlowGraph::graph>(), node->get_ptr<::FlowGraph::graph>());
			};

			/*
			menu->add_item("ololo2");
			menu->add_item("ololo3");*/
			menu->pos = pos;// -vec2(render_bounds->pos);

			  menu->self_open(user_ui); 
		}

		return true;
	}

	return false;
}

void GUI::Elements::FlowGraph::component_window::think(float dt)
{
	if (type == window_type::NODE)
		pos = node->pos;

	if (type == window_type::GRAPH_IN)
		pos = dynamic_cast<::FlowGraph::graph*>(node)->pos_in;

	if (type == window_type::GRAPH_OUT)
		pos = dynamic_cast<::FlowGraph::graph*>(node)->pos_out;
}
