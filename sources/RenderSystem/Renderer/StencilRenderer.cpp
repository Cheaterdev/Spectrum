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




void stencil_renderer::generate(Graph& graph)
{
	process_tasks();

	debug_scene->update_transforms();
	auto& caminfo = graph.get_context<CameraInfo>();

	cam = *caminfo.cam;
	cam.set_projection_params(0.01f, 1.f, 0.1f, 10000.f);
	cam.target = cam.position + direction;
	cam.update();

	axis_cam = *caminfo.cam;


	vec3 dir = /*axis_cam.position +*/ caminfo.cam->target - caminfo.cam->position;
	dir.normalize();
	axis_cam.set_projection_params(1, 1000);
	axis_cam.position -= (center_pos);
	axis_cam.position.normalize();
	axis_cam.position *= 200;
	axis_cam.target = axis_cam.position + dir;//float3(0, 0, 0);//;
	vec2 local = caminfo.cam->to_local(float3(0, 0, 0));
	axis_cam.update();
	axis_intersect_cam = axis_cam;
	axis_intersect_cam.set_projection_params(1, 1000);
	axis_intersect_cam.target = axis_intersect_cam.position + direction;
	//	cam.set_projection_params(cam.angle)
	axis_intersect_cam.update();


	{
		struct Data
		{
			Handlers::Texture depth_tex = "Stencil::depth_tex";

			Handlers::StructuredBuffer<UINT> H(id_buffer);
			Handlers::StructuredBuffer<UINT> H(axis_id_buffer);

		};

		graph.add_pass<Data>("stencil_renderer::before", [this, &graph](Data& data, TaskBuilder& builder) {

			builder.create(data.depth_tex, { { 1,1,0 }, HAL::Format::R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
			builder.create(data.id_buffer, { 1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.axis_id_buffer, { 1 }, ResourceFlags::UnorderedAccess);
			}, [this, &graph](Data& data, FrameContext& _context) {

				auto& list = *_context.get_list();

				auto& graphics = list.get_graphics();
				auto& copy = list.get_copy();

				auto& sceneinfo = graph.get_context<SceneInfo>();

				auto obj = sceneinfo.scene;

				//SceneFrameManager::get().prepare(_context.get_list(), *debug_scene);
				RT::DepthOnly::Compiled rtv;

				{
						RT::DepthOnly rt;
						rt.GetDepth() = data.depth_tex->depthStencil;
						rtv = rt.compile(list);
					}

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
							graphics.set(m.compiled_mesh_info);
							graphics.set(m.mesh_instance_info);

							{
								Slots::Instance instance;
								instance.GetInstanceId() = (UINT)current.size();
								graphics.set(instance);
							}
							graphics.dispatch_mesh(m.dispatch_mesh_arguments);
						}
					};
					graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
					graphics.set_pipeline<PSOS::DrawStencil>();
					graphics.set(scene->compiledScene);

					list.clear_uav(data.id_buffer->get_uav_clear());
					list.clear_uav(data.axis_id_buffer->get_uav_clear());


					{
						Slots::FrameInfo frameInfo;

						auto& camera = frameInfo.GetCamera();
						//		memcpy(&camera, &cam.camera_cb.current, sizeof(camera));
						camera = cam.camera_cb.current;
						graphics.set(frameInfo);
					}

					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = data.id_buffer->rwStructuredBuffer;
						graphics.set(buffer);
					}

					graphics.set_rtv(rtv, RTOptions::Default | RTOptions::ClearDepth);
					//bool rendered = false;
					
					obj->iterate([&](scene_object* node)
						{
							Graphics::renderable* render_object = dynamic_cast<Graphics::renderable*>(node);

							if (render_object)
							{
								auto instance = dynamic_cast<MeshAssetInstance*>(render_object);
								mesh_func(instance);
							}

							return true;
						});

					graphics.set_rtv(rtv, RTOptions::ClearDepth);

					{
						Slots::FrameInfo frameInfo;

						auto& camera = frameInfo.GetCamera();
						camera = axis_intersect_cam.camera_cb.current;
						//		memcpy(&camera, &axis_intersect_cam.camera_cb.current, sizeof(camera));
						graphics.set(frameInfo);
					}
					{
						Slots::PickerBuffer buffer;
						buffer.GetViewBuffer() = data.axis_id_buffer->rwStructuredBuffer;
						graphics.set(buffer);
					}




					axis->iterate([&](scene_object* node)
						{
							Graphics::renderable* render_object = dynamic_cast<Graphics::renderable*>(node);

							if (render_object)
							{
								auto l = dynamic_cast<MeshAssetInstance*>(render_object);



								for (unsigned int i = 0; i < (UINT)l->rendering.size(); i++)
								{
									auto& m = l->rendering[i];

									graphics.set(m.compiled_mesh_info);
									graphics.set(m.mesh_instance_info);

									{
										Slots::Instance instance;
										instance.GetInstanceId() = i + 1;
										graphics.set(instance);
									}

									graphics.dispatch_mesh(m.dispatch_mesh_arguments);
								}
							}

							return true;
						});

					copy.read_buffer(data.id_buffer->resource.get(), 0, 4, [current, this](std::span<std::byte> memory)
						{

							assert(memory.size());
							//device_fail();

							auto result = *reinterpret_cast<const int*>(memory.data()) - 1;

							run([result, this, current]() {
								mouse_on_object.first = nullptr;

								if (result >= current.size())
									return;

								mouse_on_object = (current[result]);

								});



						});

					copy.read_buffer(data.axis_id_buffer->resource.get(), 0, 4, [this](std::span<std::byte> memory)
						{

							auto result = *reinterpret_cast<const int*>(memory.data()) - 1;
							run([this, result]() {
								mouse_on_axis = result;
								});
						});
				}

			}, PassFlags::Required);

	}

}


void stencil_renderer::generate_after(Graph& graph)
{
	if (selected.empty())
		return;


	{
		struct Data
		{
			Handlers::Texture H(ResultTexture);
			Handlers::Texture H(Stencil_color_tex);
		};

		graph.add_pass<Data>("stencil_renderer::after", [this, &graph](Data& data, TaskBuilder& builder) {

			auto& frame = graph.get_context<ViewportInfo>();

			builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
			builder.create(data.Stencil_color_tex, { ivec3(frame.frame_size,0), HAL::Format::R8_SNORM,1 ,1 }, ResourceFlags::RenderTarget);

			}, [this, &graph](Data& data, FrameContext& _context) {


				auto& list = *_context.get_list();
				auto& graphics = list.get_graphics();

				graphics.set_signature(Layouts::DefaultLayout);

				graphics.set(scene->compiledScene);

				{

					{
						RT::SingleColor rt;
						rt.GetColor() = data.Stencil_color_tex->renderTarget;
						graphics.set_rtv(rt,RTOptions::Default| RTOptions::ClearAll);
					}

					//		data.Stencil_color_tex->renderTarget.clear(list);
				//	list.clear_rtv(data.Stencil_color_tex->renderTarget, float4(0, 0, 0, 0));
					graphics.set_pipeline<PSOS::DrawSelected>();
					graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

					graph.set_slot(SlotID::FrameInfo, graphics);



					for (auto& sel : selected)
					{

						auto l = sel.first;

						int i = sel.second;
						{
							auto& m = l->rendering[i];
							graphics.set(m.compiled_mesh_info);
							graphics.set(m.mesh_instance_info);

							graphics.dispatch_mesh(m.dispatch_mesh_arguments);
						}
					}





				}


				// apply color mask
				{
					graphics.set_pipeline<PSOS::StencilerLast>();
					graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

					{
						Slots::Countour contour;
						contour.GetColor() = { 1,0.5,0,1 };
						contour.GetTex() = data.Stencil_color_tex->texture2D;
						graphics.set(contour);
					}

					graphics.set_viewport(data.ResultTexture->get_viewport());
					graphics.set_scissor(data.ResultTexture->get_scissor());

					{
						RT::SingleColor rt;
						rt.GetColor() = data.ResultTexture->renderTarget;
						graphics.set_rtv(rt);
					}


					{
						PROFILE_GPU(L"blend");
						graphics.draw(4);
					}
				}
				{
					RT::SingleColor rt;
					rt.GetColor() = data.ResultTexture->renderTarget;
					graphics.set_rtv(rt);
				}


				if (draw_aabb) {
					graphics.set_pipeline<PSOS::DrawBox>();
					graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
					graphics.set_index_buffer(index_buffer.get_index_buffer_view());
					{
						Slots::DrawStencil draw;
						draw.GetVertices() = vertex_buffer.structuredBuffer;
						graphics.set(draw);
					}

					for (auto& sel : selected)
					{

						auto l = sel.first;
						int i = sel.second;
						{
							auto& m = l->rendering[i];
							graphics.set(m.compiled_mesh_info);
							graphics.set(m.mesh_instance_info);

							graphics.draw_indexed(36, 0, 0);
						}
					}
				}


				// draw axis
				{

					graphics.set_index_buffer(HAL::Views::IndexBuffer());// universal_index_manager::get().buffer->get_index_buffer_view(true));

					{
						Slots::FrameInfo frameInfo;
						auto& camera = frameInfo.GetCamera();
						camera = axis_cam.camera_cb.current;
						graphics.set(frameInfo);
					}
					graphics.set_pipeline<PSOS::DrawAxis>();
					graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
					int i = 0;

					for (auto& m : axis->rendering)
					{

						float lighted = (mouse_on_axis == i) * 0.7f;
						{
							Slots::Color color;
							color.GetColor() = { i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted , 1 };
							graphics.set(color);
						}
						graphics.set(m.compiled_mesh_info);
						graphics.set(m.mesh_instance_info);

						//graphics.draw(m.draw_arguments);

						graphics.dispatch_mesh(m.dispatch_mesh_arguments);
						i++;
					}

				}



			});
	}
}
