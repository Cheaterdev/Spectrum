#include "pch.h"
using namespace GUI;

using namespace Elements;

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
	Render::PipelineStateDesc state_desc;

	auto root_signature = get_Signature(Layouts::DefaultLayout);// <SignatureCreator>().create_root();
	state_desc.root_signature = root_signature;

	state_desc.vertex = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
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
	state_desc.rtv.enable_depth = false;

	draw_selected_state.reset(new Render::PipelineState(state_desc));

	state_desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
	//state_desc.vertex = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
	state_desc.pixel = Render::pixel_shader::get_resource({ "shaders/stencil.hlsl", "PS_COLOR", 0, {} });
	axis_render_state.reset(new Render::PipelineState(state_desc));

	id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
	axis_id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));


	id_buffer->set_name("stencil_renderer::id_buffer");
	axis_id_buffer->set_name("stencil_renderer::axis_id_buffer");



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




void stencil_renderer::generate(FrameGraph& graph)
{

	process_tasks();

	axis->update_transforms();
	cam = *graph.cam;
	cam.set_projection_params(0.01f, 1.f, 0.1f, 10000.f);
	cam.target = cam.position + direction;
	cam.update();

	axis_cam = *graph.cam;


	vec3 dir = /*axis_cam.position +*/ graph.cam->target - graph.cam->position;
	dir.normalize();
	axis_cam.set_projection_params(1, 1000);
	axis_cam.position -= (center_pos);
	axis_cam.position.normalize();
	axis_cam.position *= 200;
	axis_cam.target = axis_cam.position + dir;//float3(0, 0, 0);//;
	vec2 local = graph.cam->to_local(float3(0, 0, 0));
	axis_cam.update();
	axis_intersect_cam = axis_cam;
	axis_intersect_cam.set_projection_params(1, 1000);
	axis_intersect_cam.target = axis_intersect_cam.position + direction;
	//	cam.set_projection_params(cam.angle)
	axis_intersect_cam.update();


	{
		struct Data
		{
			//		ResourceHandler* target_tex;
			ResourceHandler* depth_tex;

		};

		graph.add_pass<Data>("stencil_renderer::before", [this, &graph](Data& data, TaskBuilder& builder) {
			//			data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
			data.depth_tex = builder.create_texture("Stencil::depth_tex", { 1,1 }, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil);

			}, [this, &graph](Data& data, FrameContext& _context) {

				//	auto color_tex = _context.get_texture(data.color_tex);
				auto depth_tex = _context.get_texture(data.depth_tex);



				auto& list = *_context.get_list();
				auto& graphics = list.get_graphics();
				auto& copy = list.get_copy();

				graphics.use_dynamic = false;

				auto obj = graph.scene;


				RenderTargetTable table = RenderTargetTable(graphics, {}, depth_tex);
				{
					std::vector<std::pair<MeshAssetInstance::ptr, int>> current;
					auto mesh_func = [&](MeshAssetInstance* l)
					{
						auto nodes = l->node_buffer.get_for(*list.get_manager());
						auto buffer_view = nodes.resource->create_view<StructuredBufferView<MeshAssetInstance::node_data>>(*list.frame_resources, nodes.offset, nodes.size);
						graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
						{

							Slots::MeshData data;
							data.GetNodes() = buffer_view.get_srv();
							data.GetVb() = l->mesh_asset->vertex_buffer->get_srv()[0];
							data.set(graphics);
						}
						for (unsigned int i = 0; i < l->rendering.size(); i++)
						{
							auto& m = l->rendering[i];

							auto in = intersect(cam, m.primitive_global.get());

							if (in == INTERSECT_TYPE::FULL_OUT)
							continue;

							current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);

							{
								Slots::MeshInfo info;

								info.GetNode_offset() = m.node_index;
								info.GetTexture_offset() = 0;
								info.GetVertex_offset() = m.mesh->vertex_offset;

								info.set(graphics);
							}

							{
								Slots::Instance instance;
								instance.GetInstanceId() = current.size();
								instance.set(graphics);
							}


							graphics.draw_indexed(m.mesh->index_count, m.mesh->index_offset, 0);
						}
					};
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					graphics.set_pipeline(draw_state);
		//			list.transition(depth_tex.resource, Render::ResourceState::DEPTH_WRITE);
		//			list.transition(id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		//			list.transition(axis_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
			//		list.flush_transitions();
					list.clear_uav(id_buffer, id_buffer->get_raw_uav());
					list.clear_uav(axis_id_buffer, axis_id_buffer->get_raw_uav());


					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						memcpy(&camera.cb, &cam.get_raw_cb().current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}

					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = id_buffer->get_raw_uav();
						buffer.set(graphics);
					}
					//////////////	shader_data.camera_data = cam.get_const_buffer();
					/////////////////	shader_data.id_buffer = id_buffer->get_gpu_address();

					table.clear_depth(graphics);
					table.set(graphics);
					graphics.set_viewports({ depth_tex.get_viewport() });
					graphics.set_scissors(depth_tex.get_scissor());
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

					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						memcpy(&camera.cb, &axis_intersect_cam.get_raw_cb().current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}
					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = axis_id_buffer->get_raw_uav();
						buffer.set(graphics);
					}
		
					axis->iterate([&](scene_object* node)
						{
							Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

							if (render_object)
							{
								auto l = dynamic_cast<MeshAssetInstance*>(render_object);

								graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
								auto nodes = l->node_buffer.get_for(*list.get_manager());
								auto buffer_view = nodes.resource->create_view<StructuredBufferView<MeshAssetInstance::node_data>>(*list.frame_resources, nodes.offset, nodes.size);

								{
									Slots::MeshData data;
									data.GetNodes() = buffer_view.get_srv();
									data.GetVb() = l->mesh_asset->vertex_buffer->get_srv()[0];
									data.set(graphics);
								}

								for (unsigned int i = 0; i < l->rendering.size(); i++)
								{
									auto& m = l->rendering[i];
					
									{
										Slots::MeshInfo info;

										info.GetNode_offset() = m.node_index;
										info.GetTexture_offset() = 0;
										info.GetVertex_offset() = m.mesh->vertex_offset;

										info.set(graphics);
									}

									{
										Slots::Instance instance;
										instance.GetInstanceId() = i + 1;
										instance.set(graphics);
									}
						
									graphics.draw_indexed(m.mesh->index_count, m.mesh->index_offset, 0 * m.mesh->vertex_offset);
								}
							}

							return true;
						});

					copy.read_buffer(id_buffer.get(), 0, 4, [current, this](const char* data, UINT64 size)
						{

							if (!data) device_fail();

							UINT result = *reinterpret_cast<const UINT*>(data) - 1;

							run([result,this,current]() {
								mouse_on_object.first = nullptr;

								if (result >= current.size())
									return;

								mouse_on_object = (current[result]);
			
								});
						
					

						});

					copy.read_buffer(axis_id_buffer.get(), 0, 4, [ this](const char* data, UINT64 size)
						{

							auto result = *reinterpret_cast<const UINT*>(data) - 1;
							run([this, result]() {
								mouse_on_axis = result;
								});
						});
				}

			});

	}

}


void stencil_renderer::generate_after(FrameGraph& graph)
{
	if (selected.empty())
		return;


	{
		struct Data
		{
			ResourceHandler* target_tex;
			ResourceHandler* color_tex;

		};

		graph.add_pass<Data>("stencil_renderer::after", [this, &graph](Data& data, TaskBuilder& builder) {
			data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
			data.color_tex = builder.create_texture("Stencil::ColorTex", graph.frame_size, 1, DXGI_FORMAT::DXGI_FORMAT_R8_SNORM, ResourceFlags::RenderTarget);

			}, [this, &graph](Data& data, FrameContext& _context) {

				auto color_tex = _context.get_texture(data.color_tex);
				auto target_tex = _context.get_texture(data.target_tex);

				auto& list = *_context.get_list();
				auto& graphics = list.get_graphics();
				graphics.use_dynamic = false;

				{
			//		list.transition(color_tex.resource, ResourceState::RENDER_TARGET);
					graphics.set_rtv(1, color_tex.get_rtv(), Handle());
					color_tex.get_rtv().clear(list);

					graphics.set_pipeline(draw_selected_state);
					//////////////////////		shader_data.camera_data = mesh_render_context->cam->get_const_buffer();
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					graphics.set_viewports({ color_tex.get_viewport() });
					graphics.set_scissors(color_tex.get_scissor());

					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						memcpy(&camera.cb, &graph.cam->get_raw_cb().current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}



					for (auto& sel : selected)
					{

						auto l = sel.first;
						auto nodes = l->node_buffer.get_for(*list.get_manager());
						auto buffer_view = nodes.resource->create_view<StructuredBufferView<MeshAssetInstance::node_data>>(*list.frame_resources, nodes.offset, nodes.size);
						graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));


						Slots::MeshData data;
						data.GetNodes() = buffer_view.get_srv();
						data.GetVb() = l->mesh_asset->vertex_buffer->get_srv()[0];
						data.set(graphics);


						int i = sel.second;
						{
							auto& m = l->rendering[i];
							{
								Slots::MeshInfo info;

								info.GetNode_offset() = m.node_index;
								info.GetTexture_offset() = 0;
								info.GetVertex_offset() = m.mesh->vertex_offset;

								info.set(graphics);
							}
							graphics.draw_indexed(m.mesh->index_count, m.mesh->index_offset, 0);
						}
					}
				}
				

				// apply color mask
			{
					graphics.set_pipeline(last_render_state);
					list.transition(color_tex.resource, ResourceState::PIXEL_SHADER_RESOURCE);
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

					{
						Slots::Countour contour;
						contour.GetColor() = { 1,0.5,0,1 };
						contour.GetTex() = color_tex.get_srv();
						contour.set(graphics);
					}

					graphics.set_viewport(target_tex.get_viewport());
					graphics.set_scissor(target_tex.get_scissor());
					graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());
					{
						auto timer = list.start(L"blend");
						graphics.draw(4);
					}
				}

				// draw axis
			{


					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						memcpy(&camera.cb, &axis_cam.get_raw_cb().current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}





					graphics.set_pipeline(axis_render_state);
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					int i = 0;

			
					auto nodes = axis->node_buffer.get_for(*list.get_manager());
					auto buffer_view = nodes.resource->create_view<StructuredBufferView<MeshAssetInstance::node_data>>(*list.frame_resources, nodes.offset, nodes.size);
					graphics.set_index_buffer(axis->mesh_asset->index_buffer->get_index_buffer_view(true));

					Slots::MeshData data;
					data.GetNodes() = buffer_view.get_srv();
					data.GetVb() = axis->mesh_asset->vertex_buffer->get_srv()[0];
					data.set(graphics);

					for (auto& m : axis->rendering)
					{
				
						float lighted = (mouse_on_axis == i) * 0.7f;
						//////////////////	shader_data.pixel_constants.set(i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted);
				
					
						{
							Slots::Color color;
							color.GetColor() = { i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted , 1};
							color.set(graphics);
						}
						{
							Slots::MeshInfo info;

							info.GetNode_offset() = m.node_index;
							info.GetTexture_offset() = 0;
							info.GetVertex_offset() = m.mesh->vertex_offset;

							info.set(graphics);
						}
						graphics.draw_indexed(m.mesh->index_count, m.mesh->index_offset, 0);
						i++;
					}
				}



			});
	}
}
