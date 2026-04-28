module Graphics:StencilRenderer;
import <RenderSystem.h>;
import :EngineAssets;

import :Materials.UniversalMaterial;
import GUI;

using namespace GUI;

using namespace Elements;
using namespace FrameGraph;

import Graphics;


using namespace HAL;
void stencil_renderer::select_current()
{
	selected.clear();

	if (mouse_on_object.first)
	{
		selected.push_back(mouse_on_object);
		auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
		pivot_pos = -(draw.primitive->get_min() + draw.primitive->get_max()) / 2;
		//  center_pos = mouse_on_object.first->nodes[draw.node_index]->mesh_matrix[3] - pivot_pos
		center_pos = vec3(mouse_on_object.first->local_transform[3].xyz) - pivot_pos;
	}
}

bool stencil_renderer::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (action == mouse_action::DOWN && button == mouse_button::LEFT)
	{
		if (selected.empty() || mouse_on_axis == -1)
		{
			select_current();
			selected_axis = -1;
		}

		else
		{
			selected_axis = mouse_on_axis;
			current_plane = Plane(get_normal(selected_axis), center_pos);

			mouse_pos = get_current_pos();

			set_movable(true);
		}
	}

	if (action == mouse_action::UP)
		set_movable(false);
	if (action == mouse_action::UP && button == mouse_button::RIGHT)
	{
		if (selected.size() && mouse_on_object == selected[0])
		{
			menu_list::ptr menu(new menu_list());
			menu->add_item("edit material")->on_click = [this](menu_list_element::ptr e)
			{
				if (mouse_on_object.first)
				{
					window::ptr wnd(new window);
					user_ui->add_child(wnd);
					dock_base::ptr dock(new dock_base);
					wnd->add_child(dock);
					auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
					auto& mat = draw.material;
					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(static_cast<materials::universal_material*>(mat->get_ptr().get())->get_graph()));
					wnd->pos = { 200, 200 };
					wnd->size = { 300, 300 };
				}

			};

			menu->add_item("create material")->on_click = [this](menu_list_element::ptr e)
			{
				if (mouse_on_object.first)
				{

					auto make_material = [](float3 color, float roughness, float metallic) {
						MaterialGraph::ptr graph(new MaterialGraph);


						{

							auto value_node = std::make_shared<VectorNode>(vec4(color, 1));
							graph->register_node(value_node);
							value_node->get_output(0)->link(graph->get_base_color());
						}


						{
							auto value_node = std::make_shared<ScalarNode>(roughness);
							graph->register_node(value_node);
							value_node->get_output(0)->link(graph->get_roughness());
						}

						{
							auto value_node = std::make_shared<ScalarNode>(metallic);
							graph->register_node(value_node);
							value_node->get_output(0)->link(graph->get_mettalic());

						}


						materials::universal_material* m = (new materials::universal_material(graph));
						//m->generate_material();
						m->register_new();
						return m->get_ptr<MaterialAsset>();
					};



					MaterialAsset::ptr base_mat = make_material({ 1,1,1 }, 1, 0);



					window::ptr wnd(new window);
					user_ui->add_child(wnd);
					dock_base::ptr dock(new dock_base);
					wnd->add_child(dock);


					auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
					//mouse_on_object.first->overrided_material.emplace_back();// [draw.mesh->material];

					//draw.mesh->material = mouse_on_object.first->overrided_material.size() - 1;

					// fix me
				//	draw.mesh->material = mouse_on_object.first->register_material(base_mat);

					mouse_on_object.first->override_material(mouse_on_object.second, base_mat);
					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(static_cast<materials::universal_material*>(base_mat->get_ptr().get())->get_graph()));
					wnd->pos = { 200, 200 };
					wnd->size = { 300, 300 };
				}
			};

			menu->pos = vec2(pos);
			run_on_ui([this, menu]()
				{
					menu->self_open(user_ui);
				});
		}
		else
			selected.clear();
	}


	return base::on_mouse_action(action, button, pos) | true;
}

bool stencil_renderer::on_mouse_move(vec2 pos)
{
	vec2 local = (pos - get_render_bounds().pos) / vec2(get_render_bounds().size);
	direction = player_cam->to_direction(local);
	prev_mouse_pos = mouse_pos;
	mouse_pos = get_current_pos();

	if (is_pressed() && selected_axis != -1)
	{
		center_pos += (mouse_pos - prev_mouse_pos) * get_axis(selected_axis);
		if (selected.size())
		{
			auto& draw = selected[0].first->rendering[selected[0].second];
			run_on_ui([this]() {
				selected[0].first->local_transform[3] = float4(center_pos + pivot_pos, 1);
				selected[0].first->update_layout();
				});
		}
	}

	return base::on_mouse_move(pos) | true;;
}

float3 stencil_renderer::get_axis(int axis)
{
	return float3(axis == 0, axis == 1, axis == 2);
}

float3 stencil_renderer::get_current_pos()
{
	//    Plane p(get_normal(selected_axis), center_pos);
	Ray r(cam.position, direction);
	vec3 res;
	r.intersect(current_plane, res);
	return res;
}

float3 stencil_renderer::get_normal(int axis)
{
	auto n1 = float3(axis == 1, (axis == 0 || axis == 2), 0);
	auto n2 = float3(axis == 2, 0, axis == 1 || axis == 0);
	return abs(vec3::dot(n1, direction)) > abs(vec3::dot(n2, direction)) ? n1 : n2;
}

bool stencil_renderer::can_accept(GUI::drag_n_drop_package::ptr p)
{
	if (p->name != "asset") return false;

	return true;
}

void stencil_renderer::on_drop_move(GUI::drag_n_drop_package::ptr p, vec2)
{
	auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();

	if (!item)
		return;

	if (item->asset->get_type() == Asset_Type::MATERIAL)
		select_current();
}

bool stencil_renderer::on_drop(GUI::drag_n_drop_package::ptr p, vec2 m)
{
	auto item = p->element.lock()->get_ptr<GUI::Elements::asset_item>();

	if (!item)
		return false;

	EVENT("start");
	auto asset = item->asset->get_asset();
	EVENT("end");
	if (!asset) return false;

	if (asset->get_type() == Asset_Type::MESH)
	{


		auto mesh = std::static_pointer_cast<MeshAsset>(item->asset->get_asset());


		user_ui->message_box("static?", "static?", [this, mesh](bool v) {
			auto instance = mesh->create_instance();
			instance->type = v ? MESH_TYPE::STATIC : MESH_TYPE::DYNAMIC;
			scene->add_child(instance);
			});


	}

	if (asset->get_type() == Asset_Type::MATERIAL)
	{
		auto material = item->asset->get_asset()->get_ptr<MaterialAsset>();

		//	MeshAssetInstance::ptr m(new MeshAssetInstance(mesh));

		//scene->add_child(m);
		if (mouse_on_object.first)
		{

			mouse_on_object.first->override_material(mouse_on_object.second, material);
			//.	auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
			//	mouse_on_object.first->override_material(draw.mesh->material, material);
		}

	}

	return true;
	//throw std::exception("The method or operation is not implemented.");
}

stencil_renderer::stencil_renderer() : VariableContext(L"stencil")
{


	docking = GUI::dock::PARENT;
	clickable = true;


	cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);
	axis_intersect_cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);

	axis = EngineAssets::axis.get_asset()->create_instance();

	debug_scene = std::make_shared<Scene>();

	debug_scene->add_child(axis);


	std::vector<unsigned int> data = { 3, 1, 0,
	2, 1, 3,
	0, 5, 4,
	1, 5, 0,
	3, 4, 7,
	0, 4, 3,
	1, 6, 5,
	2, 6, 1,
	2, 7, 6,
	3, 7, 2,
	6, 4, 5,
	7, 4, 6,
	};



	std::vector<vec4> verts(8);
	vec3 v0(-0.5, -0.5, 0.5);
	vec3 v1(0.5, 0.5, 0.5);
	verts[0] = vec4(-1.0f, 1.0f, -1.0f, 0);
	verts[1] = vec4(1.0f, 1.0f, -1.0f, 0);
	verts[2] = vec4(1.0f, 1.0f, 1.0f, 0);
	verts[3] = vec4(-1.0f, 1.0f, 1.0f, 0);
	verts[4] = vec4(-1.0f, -1.0f, -1.0f, 0);
	verts[5] = vec4(1.0f, -1.0f, -1.0f, 0);
	verts[6] = vec4(1.0f, -1.0f, 1.0f, 0);
	verts[7] = vec4(-1.0f, -1.0f, 1.0f, 0);
	index_buffer = Helpers::make_buffer<unsigned int>(data);

	vertex_buffer = HAL::StructuredBufferView<vec4>(8);


			auto list = (HAL::Device::get().get_upload_list());

		list->get_copy().update(vertex_buffer, 0, verts);

			list->execute_and_wait();

}
