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
					run_on_ui([this]()
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
						//	draw.mesh->material = mouse_on_object.first->register_material(base_mat);

							mouse_on_object.first->override_material(mouse_on_object.second, base_mat);
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
			EVENT("start");
			auto asset = item->asset->get_asset();
			EVENT("end");
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

							mouse_on_object.first->override_material(mouse_on_object.second, material);
						//.	auto& draw = mouse_on_object.first->rendering[mouse_on_object.second];
						//	mouse_on_object.first->override_material(draw.mesh->material, material);
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
	/*
	id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
	axis_id_buffer.reset(new Render::StructuredBuffer<UINT>(1, Render::counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));


	id_buffer->set_name("stencil_renderer::id_buffer");
	axis_id_buffer->set_name("stencil_renderer::axis_id_buffer");

	*/

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
	verts[0] = vec3(-1.0f, 1.0f, -1.0f);
	verts[1] = vec3(1.0f, 1.0f, -1.0f);
	verts[2] = vec3(1.0f, 1.0f, 1.0f);
	verts[3] = vec3(-1.0f, 1.0f, 1.0f);
	verts[4] = vec3(-1.0f, -1.0f, -1.0f);
	verts[5] = vec3(1.0f, -1.0f, -1.0f);
	verts[6] = vec3(1.0f, -1.0f, 1.0f);
	verts[7] = vec3(-1.0f, -1.0f, 1.0f);
	index_buffer.reset(new Render::IndexBuffer(data));

	vertex_buffer.reset(new Render::StructuredBuffer<vec4>(8));
	vertex_buffer->set_raw_data(verts);


}




void stencil_renderer::generate(FrameGraph& graph)
{
	process_tasks();

	debug_scene->update_transforms();
	
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
			ResourceHandler* id_buffer;
			ResourceHandler* axis_id_buffer;

		};

		graph.add_pass<Data>("stencil_renderer::before", [this, &graph](Data& data, TaskBuilder& builder) {
			//			data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
			data.depth_tex = builder.create_texture("Stencil::depth_tex", { 1,1 }, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil);
			data.id_buffer = builder.create_buffer("id_buffer", 4, ResourceFlags::UnorderedAccess);
			data.axis_id_buffer = builder.create_buffer("axis_id_buffer", 4, ResourceFlags::UnorderedAccess);
			}, [this, &graph](Data& data, FrameContext& _context) {

				auto depth_tex = _context.get_texture(data.depth_tex);
				auto& list = *_context.get_list();

				auto id_buffer = _context.get_buffer(data.id_buffer).resource->create_view<StructuredBufferView<UINT>>(*list.frame_resources);
				auto axis_id_buffer = _context.get_buffer(data.axis_id_buffer).resource->create_view<StructuredBufferView<UINT>>(*list.frame_resources);


				auto& graphics = list.get_graphics();
				auto& copy = list.get_copy();


				auto obj = graph.scene;

				//SceneFrameManager::get().prepare(_context.get_list(), *debug_scene);

				RenderTargetTable table = RenderTargetTable(graphics, {}, depth_tex);
				{
					std::vector<std::pair<MeshAssetInstance::ptr, int>> current;
					auto mesh_func = [&](MeshAssetInstance* l)
					{
			
			
						for (unsigned int i = 0; i < l->rendering.size(); i++)
						{
							auto& m = l->rendering[i];

							auto in = intersect(cam, m.primitive_global.get());

							if (in == INTERSECT_TYPE::FULL_OUT)
							continue;

							current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);
							m.mesh_info.set(graphics);

							{
								Slots::Instance instance;
								instance.GetInstanceId() = (UINT)current.size();
								instance.set(graphics);
							}
							graphics.draw(m.draw_arguments);
						}
					};
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					graphics.set_pipeline(GetPSO<PSOS::DrawStencil>());

					list.clear_uav(id_buffer.get_uav_clear());
					list.clear_uav(axis_id_buffer.get_uav_clear());


					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
				//		memcpy(&camera.cb, &cam.camera_cb.current, sizeof(camera.cb));
						camera.cb = cam.camera_cb.current;
						frameInfo.set(graphics);
					}

					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = id_buffer.rwStructuredBuffer;
						buffer.set(graphics);
					}

					{
						Slots::SceneData sceneData;
						sceneData.GetNodes() = universal_nodes_manager::get().buffer->structuredBuffer;
						sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
						sceneData.GetVertexes() = universal_vertex_manager::get().buffer->structuredBuffer;


						sceneData.set(graphics);
					}
					graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));



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
						camera.cb = axis_intersect_cam.camera_cb.current;
				//		memcpy(&camera.cb, &axis_intersect_cam.camera_cb.current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}
					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = axis_id_buffer.rwStructuredBuffer;
						buffer.set(graphics);
					}
		
					{
						Slots::SceneData sceneData;
						sceneData.GetNodes() = universal_nodes_manager::get().buffer->structuredBuffer;
						sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
						sceneData.GetVertexes() = universal_vertex_manager::get().buffer->structuredBuffer;

						sceneData.set(graphics);
					}
					graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));


					axis->iterate([&](scene_object* node)
						{
							Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

							if (render_object)
							{
								auto l = dynamic_cast<MeshAssetInstance*>(render_object);

						
					
								for (unsigned int i = 0; i < (UINT)l->rendering.size(); i++)
								{
									auto& m = l->rendering[i];
					
									m.mesh_info.set(graphics);
								
									{
										Slots::Instance instance;
										instance.GetInstanceId() = i + 1;
										instance.set(graphics);
									}
						
									graphics.draw(m.draw_arguments);
								}
							}

							return true;
						});

					copy.read_buffer(id_buffer.resource.get(), 0, 4, [current, this](const char* data, UINT64 size)
						{

							if (!data) device_fail();

							auto result = *reinterpret_cast<const int*>(data) - 1;

							run([result,this,current]() {
								mouse_on_object.first = nullptr;

								if (result >= current.size())
									return;

								mouse_on_object = (current[result]);
			
								});
						
					

						});

					copy.read_buffer(axis_id_buffer.resource.get(), 0, 4, [ this](const char* data, UINT64 size)
						{

							auto result = *reinterpret_cast<const int*>(data) - 1;
							run([this, result]() {
								mouse_on_axis = result;
								});
						});
				}

			}, PassFlags::Required);

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

				graphics.set_signature(get_Signature(Layouts::DefaultLayout));
				{
					Slots::SceneData sceneData;
					sceneData.GetNodes() = universal_nodes_manager::get().buffer->structuredBuffer;
					sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
					sceneData.GetVertexes() = universal_vertex_manager::get().buffer->structuredBuffer;

					sceneData.set(graphics);
				}
				graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));



				{
					graphics.set_rtv(1, color_tex.get_rtv(), Handle());
					color_tex.get_rtv().clear(list);

					graphics.set_pipeline(GetPSO<PSOS::DrawSelected>());
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					graphics.set_viewports({ color_tex.get_viewport() });
					graphics.set_scissors(color_tex.get_scissor());

					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						camera.cb = graph.cam->camera_cb.current;
						//memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}



					for (auto& sel : selected)
					{

						auto l = sel.first;
				
						int i = sel.second;
						{
							auto& m = l->rendering[i];
							m.mesh_info.set(graphics);

							graphics.draw(m.draw_arguments);
						}
					}



					

				}
				

				// apply color mask
			{
					graphics.set_pipeline(GetPSO<PSOS::StencilerLast>());
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

					{
						Slots::Countour contour;
						contour.GetColor() = { 1,0.5,0,1 };
						contour.GetTex() = color_tex.texture2D;
						contour.set(graphics);
					}

					graphics.set_viewport(target_tex.get_viewport());
					graphics.set_scissor(target_tex.get_scissor());
					graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());
					{
						PROFILE_GPU(L"blend");
						graphics.draw(4);
					}
				}

			graphics.set_rtv(1, target_tex.get_rtv(), Handle());

			{
				graphics.set_pipeline(GetPSO<PSOS::DrawBox>());
				graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));
				{
					Slots::DrawStencil draw;
					draw.GetVertices() = vertex_buffer->structuredBuffer;
					draw.set(graphics);
				}

				for (auto& sel : selected)
				{

					auto l = sel.first;
					int i = sel.second;
					{
						auto& m = l->rendering[i];
						m.mesh_info.set(graphics);

						graphics.draw_indexed(36, 0, 0);
					}
				}
			}
		

				// draw axis
			{

				graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));

					{
						Slots::FrameInfo frameInfo;

						auto camera = frameInfo.MapCamera();
						camera.cb = axis_cam.camera_cb.current;
						//memcpy(&camera.cb, &axis_cam.camera_cb.current, sizeof(camera.cb));
						frameInfo.set(graphics);
					}
					graphics.set_pipeline(GetPSO<PSOS::DrawAxis>());
					graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					int i = 0;

					for (auto& m : axis->rendering)
					{
				
						float lighted = (mouse_on_axis == i) * 0.7f;
						{
							Slots::Color color;
							color.GetColor() = { i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted , 1};
							color.set(graphics);
						}
						m.mesh_info.set(graphics);
						graphics.draw(m.draw_arguments);
						i++;
					}

				

				}



			});
	}
}
