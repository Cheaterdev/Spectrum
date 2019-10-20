#include "pch.h"
using namespace GUI;

using namespace Elements;



template <class T>
struct StencilSignature : public T
{
	using T::T;

	typename T::template SRV			<0, Render::ShaderVisibility::VERTEX, 0>								vertex_buffer = this;
	typename T::template ConstBuffer	<1, Render::ShaderVisibility::VERTEX, 0>								camera_data = this;
	typename T::template Constants		<2, Render::ShaderVisibility::PIXEL, 0, 3>								pixel_constants = this;
	typename T::template UAV			<3, Render::ShaderVisibility::PIXEL, 0>									id_buffer = this;

	typename T::template Constants		<4, Render::ShaderVisibility::VERTEX, 2, 3>								vertex_constants = this;
	typename T::template SRV			<5, Render::ShaderVisibility::VERTEX, 1, 1>								instance_data = this;

	typename T::template Table			<6, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1>pixel_source = this;


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };
};


void stencil_renderer::render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj)
{

	axis->update_transforms();
	cam = *mesh_render_context->cam;
	cam.set_projection_params(0.01f, 1.f, 0.1f, 10000.f);
	cam.target = cam.position + direction;
	cam.update();

	axis_cam = *mesh_render_context->cam;


	vec3 dir = /*axis_cam.position +*/ mesh_render_context->cam->target - mesh_render_context->cam->position;
	dir.normalize();
	axis_cam.set_projection_params(1, 1000);
	axis_cam.position -= (center_pos);
	axis_cam.position.normalize();
	axis_cam.position *= 200;
	axis_cam.target = axis_cam.position + dir;//float3(0, 0, 0);//;
	vec2 local = mesh_render_context->cam->to_local(float3(0, 0, 0));
	axis_cam.update();
	axis_intersect_cam = axis_cam;
	axis_intersect_cam.set_projection_params(1, 1000);
	axis_intersect_cam.target = axis_intersect_cam.position + direction;
	//	cam.set_projection_params(cam.angle)
	axis_intersect_cam.update();
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	StencilSignature<Signature> shader_data(&graphics);
	{
		current_frame = (current_frame + 1) % 2;
		auto& current = all[current_frame];
		current.clear();
		auto mesh_func = [&](MeshAssetInstance* l)
		{
			auto nodes = l->node_buffer.get_for(*list.get_manager());
			shader_data.instance_data = nodes.resource->get_gpu_address() + nodes.offset;
			shader_data.vertex_buffer = l->mesh_asset->vertex_buffer->get_gpu_address();
			graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));


			for (unsigned int i = 0; i < l->rendering.size(); i++)
			{
				auto& e = l->rendering[i];
				auto& node = l->nodes[e.node_index];

				auto in = intersect(cam, e.primitive_global.get());

				if (in == INTERSECT_TYPE::FULL_OUT)
					continue;

				current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);
				shader_data.vertex_constants.set(0, e.node_index);
				shader_data.pixel_constants.set(current.size());

				graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
			}
		};
		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics.set_pipeline(draw_state);
		list.transition(depth_tex, Render::ResourceState::DEPTH_WRITE);
		list.transition(id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		list.transition(axis_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		list.flush_transitions();
		list.clear_uav(id_buffer, id_buffer->get_raw_uav());
		list.clear_uav(axis_id_buffer, axis_id_buffer->get_raw_uav());

		shader_data.camera_data = cam.get_const_buffer();
		shader_data.id_buffer = id_buffer->get_gpu_address();

		table.clear_depth(graphics);
		table.set(graphics);
		graphics.set_viewports({ depth_tex->texture_2d()->get_viewport() });
		graphics.set_scissors(depth_tex->texture_2d()->get_scissor());
		obj->iterate([&](scene_object* node)
			{
				Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

				if (render_object)
				{
					auto instance = dynamic_cast<MeshAssetInstance*>(render_object);
					mesh_func(instance);
				}

				return true;
			});
		table.clear_depth(graphics);
		shader_data.id_buffer = axis_id_buffer->get_gpu_address();
		shader_data.camera_data = axis_intersect_cam.get_const_buffer();

		axis->iterate([&](scene_object* node)
			{
				Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

				if (render_object)
				{
					auto l = dynamic_cast<MeshAssetInstance*>(render_object);
					auto nodes = l->node_buffer.get_for(*list.get_manager());
					shader_data.vertex_buffer = l->mesh_asset->vertex_buffer->get_gpu_address();
					shader_data.instance_data = nodes.resource->get_gpu_address() + nodes.offset;

					graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));


					for (unsigned int i = 0; i < l->rendering.size(); i++)
					{
						auto& e = l->rendering[i];
						auto& node = l->nodes[e.node_index];

						shader_data.vertex_constants.set(0, e.node_index);
						shader_data.pixel_constants.set(i + 1);
						graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
					}
				}

				return true;
			});

		copy.read_buffer(id_buffer.get(), 0, 4, [current, this](const char* data, UINT64 size)
			{
				if (!data) device_fail();

				UINT result = *reinterpret_cast<const UINT*>(data) - 1;
				mouse_on_object.first = nullptr;

				if (result >= current.size())
					return;

				mouse_on_object = (current[result]);
			});
		copy.read_buffer(axis_id_buffer.get(), 0, 4, [current, this](const char* data, UINT64 size)
			{
				mouse_on_axis = *reinterpret_cast<const UINT*>(data) - 1;
			});
	}
}


void stencil_renderer::select_current()
{
	selected.clear();

	if (mouse_on_object.first)
	{
		selected.push_back(mouse_on_object);
		auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
		pivot_pos = -(draw.mesh->primitive->get_min() + draw.mesh->primitive->get_max()) / 2;
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
					run_on_ui([this]()
						{
							window::ptr wnd(new window);
							user_ui->add_child(wnd);
							dock_base::ptr dock(new dock_base);
							wnd->add_child(dock);
							auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
							auto& mat = mouse_on_object.first->overrided_material[draw.mesh->material];
							dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(static_cast<materials::universal_material*>(mat->get_ptr().get())->get_graph()));
							wnd->pos = { 200, 200 };
							wnd->size = { 300, 300 };
						});
			};

			menu->add_item("create material")->on_click = [this](menu_list_element::ptr e)
			{
				if (mouse_on_object.first)
					run_on_ui([this]()
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
							draw.mesh->material = mouse_on_object.first->register_material(base_mat);


							dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(static_cast<materials::universal_material*>(base_mat->get_ptr().get())->get_graph()));
							wnd->pos = { 200, 200 };
							wnd->size = { 300, 300 };
						});
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

	run_on_ui([this, item]()
		{
			auto asset = item->asset->get_asset();

			if (!asset) return;

			if (asset->get_type() == Asset_Type::MESH)
			{
				auto mesh = std::static_pointer_cast<MeshAsset>(item->asset->get_asset());

				user_ui->message_box("static?", "static?", [this, mesh](bool v) {
					MeshAssetInstance::ptr m(new MeshAssetInstance(mesh));

					/*	m->iterate([](scene_object*obj) {
							obj->resizes_parent = false;

							return true;

						});*/
					m->type = v ? MESH_TYPE::STATIC : MESH_TYPE::DYNAMIC;
					run_on_ui([this, mesh, m]()
						{
							scene->add_child(m);
						});


					});


			}

			if (asset->get_type() == Asset_Type::MATERIAL)
			{
				auto material = item->asset->get_asset()->get_ptr<MaterialAsset>();
				run_on_ui([this, material]()
					{
						//	MeshAssetInstance::ptr m(new MeshAssetInstance(mesh));

						//scene->add_child(m);
						if (mouse_on_object.first)
						{
							auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
							mouse_on_object.first->override_material(draw.mesh->material, material);
						}
					});
			}
		});
	return true;
	//throw std::exception("The method or operation is not implemented.");
}

stencil_renderer::stencil_renderer()
{


	docking = GUI::dock::PARENT;
	clickable = true;
	//catch_clicks = true;
//   draw_helper = true;
	depth_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));
	table = RenderTargetTable({}, depth_tex);
	Render::PipelineStateDesc state_desc;

	auto root_signature = StencilSignature<SignatureCreator>().create_root();
	state_desc.root_signature = root_signature;

	state_desc.vertex = Render::vertex_shader::get_resource({ "shaders/stencil.hlsl", "VS", 0, {} });
	state_desc.pixel = Render::pixel_shader::get_resource({ "shaders/stencil.hlsl", "PS", 0, {} });
	state_desc.rtv.rtv_formats = {};
	state_desc.rtv.enable_depth = true;
	state_desc.rtv.enable_depth_write = true;
	state_desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	state_desc.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	state_desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	draw_state.reset(new Render::PipelineState(state_desc));

	state_desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	state_desc.pixel = Render::pixel_shader::get_resource({ "shaders/stencil.hlsl", "PS_RESULT", 0, {} });
	state_desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8_SNORM };
	state_desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
	state_desc.rtv.enable_depth_write = false;
	draw_selected_state.reset(new Render::PipelineState(state_desc));

	state_desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
	state_desc.vertex = Render::vertex_shader::get_resource({ "shaders/stencil.hlsl", "VS_COLOR", 0, {} });
	state_desc.pixel = Render::pixel_shader::get_resource({ "shaders/stencil.hlsl", "PS_COLOR", 0, {} });
	axis_render_state.reset(new Render::PipelineState(state_desc));

	id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
	axis_id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));


	id_buffer->set_name("stencil_renderer::id_buffer");
	axis_id_buffer->set_name("stencil_renderer::axis_id_buffer");
	depth_tex->set_name("stencil_renderer::depth_tex");


	cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);
	axis_intersect_cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);
	
	{
		Render::PipelineStateDesc state_desc;

		state_desc.root_signature = root_signature;

		state_desc.blend.render_target[0].enabled = true;
		state_desc.blend.render_target[0].source = D3D12_BLEND_ONE;
		state_desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
		state_desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\contour.hlsl", "PS", 0, {} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\contour.hlsl", "VS", 0, {} });
		state_desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		last_render_state.reset(new Render::PipelineState(state_desc));
	}

	axis.reset(new MeshAssetInstance(EngineAssets::axis.get_asset()));

}
void stencil_renderer::on_bounds_changed(const rect& r)
{
	base::on_bounds_changed(r);
	auto size = ivec2::max(ivec2(r.size), ivec2{ 1,1 });

	color_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_SNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::RENDER_TARGET));
	render_color_table = RenderTargetTable({ color_tex }, nullptr);
}
void stencil_renderer::draw_after(MeshRenderContext::ptr mesh_render_context, G_Buffer& buffer)
{
	if (selected.empty()) return;

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& list = *mesh_render_context->list;
	StencilSignature<Signature> shader_data(&graphics);

	// draw mesh mask
	{
		list.transition(color_tex, ResourceState::RENDER_TARGET);
		graphics.set_rtv(1, color_tex->texture_2d()->get_rtv(), Handle());
		color_tex->texture_2d()->get_rtv().clear(*mesh_render_context->list);
		graphics.set_pipeline(draw_selected_state);
		shader_data.camera_data = mesh_render_context->cam->get_const_buffer();
		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& sel : selected)
		{
			auto l = sel.first;
			auto nodes = l->node_buffer.get_for(*list.get_manager());
			int i = sel.second;
			{
				auto& e = l->rendering[i];
				auto& node = l->nodes[e.node_index];

				auto nodes = l->node_buffer.get_for(*list.get_manager());
				shader_data.instance_data = nodes.resource->get_gpu_address() + nodes.offset;
				shader_data.vertex_buffer = l->mesh_asset->vertex_buffer->get_gpu_address();

				graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
				shader_data.vertex_constants.set(0, e.node_index);

				graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
			}
		}
	}

	// apply color mask
	{
		graphics.set_pipeline(last_render_state);
		list.transition(color_tex, ResourceState::PIXEL_SHADER_RESOURCE);
		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport());
		graphics.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor());
		graphics.set_rtv(1, buffer.result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
		shader_data.pixel_source[0] = color_tex->texture_2d()->get_srv();
		{
			auto timer = list.start(L"blend");
			graphics.draw(4);
		}
	}

	// draw axis
	{
		graphics.set_pipeline(axis_render_state);
		shader_data.camera_data= axis_cam.get_const_buffer();
		auto nodes = axis->node_buffer.get_for(*list.get_manager());
		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		int i = 0;

		shader_data.vertex_constants.set(0, 0, 0);
		shader_data.instance_data = nodes.resource->get_gpu_address() + nodes.offset;
		shader_data.vertex_buffer = axis->mesh_asset->vertex_buffer->get_gpu_address();
		graphics.set_index_buffer(axis->mesh_asset->index_buffer->get_index_buffer_view(true));
		for (auto& e : axis->rendering)
		{
			auto& node = axis->nodes[e.node_index];

			float lighted = (mouse_on_axis == i) * 0.7f;
			shader_data.pixel_constants.set(i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted);
			i++;
			shader_data.vertex_constants.set(0, e.node_index);

			graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);

		}
	}
}

void stencil_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&) > f)
{
	throw std::logic_error("The method or operation is not implemented.");
}
