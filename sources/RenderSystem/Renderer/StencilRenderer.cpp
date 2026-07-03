module Graphics:StencilRenderer;
import RenderSystem;

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
	if (action == mouse_action::DOWN)
		focus(); // clicking the viewport focuses it, so WASD/QE reach us

	// Right-drag = camera look (Unreal-style). A right-click without a drag falls
	// through to the context-menu logic below.
	if (action == mouse_action::DOWN && button == mouse_button::RIGHT)
	{
		looking     = true;
		rmb_dragged = false;
		look_last   = pos;
		set_movable(true);
	}

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
			if (is_rotate())
			{
				// rotate about the picked axis: drag plane is perpendicular to it
				vec3 axis = get_axis(selected_axis - 3);
				current_plane = Plane(axis, center_pos);
				rot_prev = get_current_pos() - center_pos;
				rot_prev.normalize();
			}
			else
			{
				current_plane = Plane(get_normal(selected_axis), center_pos);
				mouse_pos = get_current_pos();
			}

			set_movable(true);
		}
	}

	if (action == mouse_action::UP || action == mouse_action::CANCEL)
		set_movable(false);
	if ((action == mouse_action::UP || action == mouse_action::CANCEL) && button == mouse_button::RIGHT)
		looking = false;
	if (action == mouse_action::UP && button == mouse_button::RIGHT && !rmb_dragged)
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
	if (looking)
	{
		vec2 delta = pos - look_last;
		look_last  = pos;
		float adx = delta.x < 0 ? -delta.x : delta.x;
		float ady = delta.y < 0 ? -delta.y : delta.y;
		if (adx + ady > 2.0f)
			rmb_dragged = true; // it's a look-drag, not a click
		if (rmb_dragged && player_cam)
			player_cam->add_look(delta);
		return true;
	}

	vec2 local = (pos - get_render_bounds().pos) / vec2(get_render_bounds().size);
	direction = player_cam->to_direction(local);
	prev_mouse_pos = mouse_pos;
	mouse_pos = get_current_pos();

	if (is_pressed() && selected_axis != -1 && is_rotate())
	{
		vec3 axis = get_axis(selected_axis - 3);
		vec3 cur  = get_current_pos() - center_pos;
		cur.normalize();

		float cosA = Math::clamp(vec3::dot(rot_prev, cur), -1.0f, 1.0f);
		float sinA = vec3::dot(axis, vec3::cross(rot_prev, cur));
		float dphi = Math::acos(cosA) * (sinA < 0 ? 1.0f : -1.0f);
		rot_prev = cur;

		if (selected.size() && dphi != 0.0f)
		{
			quat q(axis, dphi);
			run_on_ui([this, q]() {
				quat  lq = q;
				auto& T  = selected[0].first->local_transform;
				for (int c = 0; c < 3; c++)
				{
					vec3 b = vec3(T[c].x, T[c].y, T[c].z);
					vec3 r = lq.rotate(b);
					T[c] = float4(r, T[c].w);
				}
				selected[0].first->update_layout();
				});
		}
	}
	else if (is_pressed() && selected_axis != -1)
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

void stencil_renderer::on_key_action(key_action action, long key)
{
	if (key >= 0 && key < 256) camera_keys[key] = (action == key_action::DOWN);
	update_move_input();
}

void stencil_renderer::update_move_input()
{
	if (!player_cam) return;
	auto k = [this](int vk) { return camera_keys[vk] ? 1.0f : 0.0f; };
	player_cam->move_input = {
		k('D') - k('A'),   // right
		k('E') - k('Q'),   // up
		k('W') - k('S')    // forward
	};
	player_cam->fast_move = camera_keys[0x10]; // VK_SHIFT
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

void stencil_renderer::build_rings(float radius, float thickness)
{
	const int   N  = 64;
	const float r1 = radius;             // outer
	const float r0 = radius - thickness; // inner

	// For the ring that rotates about axis a, its plane is spanned by the other two axes.
	static const vec3 U[3] = { vec3(0,1,0), vec3(0,0,1), vec3(1,0,0) };
	static const vec3 V[3] = { vec3(0,0,1), vec3(1,0,0), vec3(0,1,0) };

	std::vector<vec4>         verts;
	std::vector<unsigned int> indices;
	verts.reserve(3 * 2 * N);
	indices.reserve(3 * 6 * N);

	for (int a = 0; a < 3; a++)
	{
		UINT base = (UINT)verts.size();
		UINT ioff = (UINT)indices.size();

		for (int i = 0; i < N; i++)
		{
			float ang = (float(i) / N) * Math::m_2_pi;
			vec3  dir = U[a] * Math::cos(ang) + V[a] * Math::sin(ang);
			verts.push_back(vec4(dir * r0, 0)); // inner
			verts.push_back(vec4(dir * r1, 0)); // outer
		}
		for (int i = 0; i < N; i++)
		{
			UINT i0 = base + (i * 2);
			UINT i1 = base + (i * 2 + 1);
			UINT i2 = base + (((i + 1) % N) * 2);
			UINT i3 = base + (((i + 1) % N) * 2 + 1);
			indices.push_back(i0); indices.push_back(i1); indices.push_back(i3);
			indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
		}
		ring_ranges[a] = { ioff, (UINT)indices.size() - ioff };
	}

	ring_index_buffer  = Helpers::make_buffer<unsigned int>(RenderSystem::get().device(), indices);
	ring_vertex_buffer = HAL::StructuredBufferView<vec4>(RenderSystem::get().device(), (UINT)verts.size());

	auto list = RenderSystem::get().device().get_upload_list();
	list->get_copy().update(ring_vertex_buffer, 0, verts);
	list->execute_and_wait();
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
	verts[0] = vec4(-1.0f,  1.0f, -1.0f, 0);
	verts[1] = vec4( 1.0f,  1.0f, -1.0f, 0);
	verts[2] = vec4( 1.0f,  1.0f,  1.0f, 0);
	verts[3] = vec4(-1.0f,  1.0f,  1.0f, 0);
	verts[4] = vec4(-1.0f, -1.0f, -1.0f, 0);
	verts[5] = vec4( 1.0f, -1.0f, -1.0f, 0);
	verts[6] = vec4( 1.0f, -1.0f,  1.0f, 0);
	verts[7] = vec4(-1.0f, -1.0f,  1.0f, 0);
	index_buffer = Helpers::make_buffer<unsigned int>(RenderSystem::get().device(), data);

	vertex_buffer = HAL::StructuredBufferView<vec4>(RenderSystem::get().device(), 8);

	auto list = RenderSystem::get().device().get_upload_list();
	list->get_copy().update(vertex_buffer, 0, verts);
	list->execute_and_wait();

	// ---- Pass function members -----------------------------------------------

	m_before_setup = [this](Passes::stencil_renderer_before::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		process_tasks();
		debug_scene->update_transforms();

		// Size the rotation rings from the arrows' actual (post-import-scale) bounds,
		// once they are available. The arrows are drawn through the mesh node transform,
		// so a hardcoded radius in raw model units would not match their rendered size.
		if (!rings_sized && axis && !axis->rendering.empty())
		{
			float ext = 0.0f;
			auto  fabs2 = [](float v) { return v < 0 ? -v : v; };
			for (auto& r : axis->rendering)
			{
				auto mn = r.primitive->get_min();
				auto mx = r.primitive->get_max();
				ext = std::max(ext, std::max(std::max(fabs2(mx.x), fabs2(mx.y)), fabs2(mx.z)));
				ext = std::max(ext, std::max(std::max(fabs2(mn.x), fabs2(mn.y)), fabs2(mn.z)));
			}
			if (ext > 0.0f)
			{
				build_rings(ext * 0.9f, ext * 0.12f);
				rings_sized = true;
			}
		}

		auto& caminfo = builder.graph->get_context<CameraInfo>();
		cam = *caminfo.cam;
		cam.set_projection_params(0.01f, 1.f, 0.1f, 10000.f);
		cam.target = cam.position + direction;
		cam.update();

		axis_cam = *caminfo.cam;
		vec3 dir = caminfo.cam->target - caminfo.cam->position;
		dir.normalize();
		axis_cam.set_projection_params(1, 1000);
		axis_cam.position -= center_pos;
		axis_cam.position.normalize();
		axis_cam.position *= 200;
		axis_cam.target = axis_cam.position + dir;
		axis_cam.update();

		axis_intersect_cam = axis_cam;
		axis_intersect_cam.set_projection_params(1, 1000);
		axis_intersect_cam.target = axis_intersect_cam.position + direction;
		axis_intersect_cam.update();

		builder.create(data.depth_tex,      { { 1,1,0 }, HAL::Format::R32_TYPELESS, 1 }, FrameGraph::ResourceFlags::DepthStencil);
		builder.create(data.id_buffer,      { 1 }, FrameGraph::ResourceFlags::UnorderedAccess);
		builder.create(data.axis_id_buffer, { 1 }, FrameGraph::ResourceFlags::UnorderedAccess);
		return true;
	};

	m_before_render = [this](Passes::stencil_renderer_before::Context& data, FrameGraph::FrameContext& context)
	{
		auto& list     = *context.get_list();
		auto& graphics = list.get_graphics();
		auto& compute  = list.get_compute();
		auto& copy     = list.get_copy();

		auto obj = context.graph->get_context<SceneInfo>().scene;

		RT::DepthOnly::Compiled rtv;
		{
			RT::DepthOnly rt;
			rt.GetDepth() = data.depth_tex->depthStencil;
			rtv = rt.compile(list);
		}

		std::vector<std::pair<MeshAssetInstance::ptr, int>> current;
		auto mesh_func = [&](MeshAssetInstance* l)
		{
			for (unsigned int i = 0; i < l->rendering.size(); i++)
			{
				auto& m = l->rendering[i];
				if (intersect(cam, m.primitive_global.get()) == INTERSECT_TYPE::FULL_OUT)
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

		compute.clear(*data.id_buffer);
		compute.clear(*data.axis_id_buffer);

		{
			Slots::FrameInfo frameInfo;
			frameInfo.GetCamera() = cam.camera_cb.current;
			graphics.set(frameInfo);
		}
		{
			Slots::PickerBuffer buffer;
			buffer.GetViewBuffer() = *data.id_buffer;
			graphics.set(buffer);
		}

		graphics.set_rtv(rtv, RTOptions::Default | RTOptions::ClearDepth);

		obj->iterate([&](scene_object* node)
		{
			auto render_object = dynamic_cast<Graphics::renderable*>(node);
			if (render_object)
				mesh_func(dynamic_cast<MeshAssetInstance*>(render_object));
			return true;
		});

		graphics.set_rtv(rtv, RTOptions::ClearDepth);

		{
			Slots::FrameInfo frameInfo;
			frameInfo.GetCamera() = axis_intersect_cam.camera_cb.current;
			graphics.set(frameInfo);
		}
		{
			Slots::PickerBuffer buffer;
			buffer.GetViewBuffer() = *data.axis_id_buffer;
			graphics.set(buffer);
		}

		axis->iterate([&](scene_object* node)
		{
			auto render_object = dynamic_cast<Graphics::renderable*>(node);
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

		// pick rotation rings (ids 4/5/6 -> mouse_on_axis 3/4/5); shares the depth
		// buffer with the arrows so the nearest handle along the ray wins.
		if (rings_sized)
		{
			graphics.set_pipeline<PSOS::DrawRingPick>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_index_buffer(ring_index_buffer.get_index_buffer_view());
			{
				Slots::DrawStencil draw;
				draw.GetVertices() = ring_vertex_buffer;
				graphics.set(draw);
			}
			for (int a = 0; a < 3; a++)
			{
				{
					Slots::Instance instance;
					instance.GetInstanceId() = 4 + a;
					graphics.set(instance);
				}
				graphics.draw_indexed(ring_ranges[a].count, ring_ranges[a].offset, 0);
			}
		}

		copy.read<uint>(*data.id_buffer, 0, 1, [current, this](std::span<uint> memory)
		{
			auto result = *memory.data() - 1;
			run([result, this, current]()
			{
				mouse_on_object.first = nullptr;
				if (result < current.size())
					mouse_on_object = current[result];
			});
		});

		copy.read<uint>(*data.axis_id_buffer, 0, 1, [this](std::span<uint> memory)
		{
			auto result = *memory.data() - 1;
			run([this, result]() { mouse_on_axis = result; });
		});
	};

	m_after_setup = [this](Passes::stencil_renderer_after::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		if (selected.empty())
			return false;
		auto& frame = builder.graph->get_context<ViewportInfo>();
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::RenderTarget);
		builder.create(data.Stencil_color_tex,
			{ ivec3(frame.frame_size, 0), HAL::Format::R8_SNORM, 1, 1 },
			FrameGraph::ResourceFlags::RenderTarget);
		return true;
	};

	m_after_render = [this](Passes::stencil_renderer_after::Context& data, FrameGraph::FrameContext& context)
	{
		auto& list     = *context.get_list();
		auto& graphics = list.get_graphics();

		graphics.set_signature(Layouts::DefaultLayout);
		graphics.set(scene->compiledScene);

		{
			RT::SingleColor rt;
			rt.GetColor() = data.Stencil_color_tex->renderTarget;
			graphics.set_rtv(rt, RTOptions::Default | RTOptions::ClearAll);
		}

		graphics.set_pipeline<PSOS::DrawSelected>();
		graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

		context.graph->set_slot(SlotID::FrameInfo, graphics);

		for (auto& sel : selected)
		{
			auto& m = sel.first->rendering[sel.second];
			graphics.set(m.compiled_mesh_info);
			graphics.set(m.mesh_instance_info);
			graphics.dispatch_mesh(m.dispatch_mesh_arguments);
		}

		// apply color mask
		{
			graphics.set_pipeline<PSOS::StencilerLast>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			{
				Slots::Countour contour;
				contour.GetColor() = { 1, 0.5f, 0, 1 };
				contour.GetTex()   = *data.Stencil_color_tex;
				graphics.set(contour);
			}
			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());
			{
				RT::SingleColor rt;
				rt.GetColor() = data.ResultTexture->renderTarget;
				graphics.set_rtv(rt);
			}
			graphics.draw(4);
		}

		{
			RT::SingleColor rt;
			rt.GetColor() = data.ResultTexture->renderTarget;
			graphics.set_rtv(rt);
		}

		if (draw_aabb)
		{
			graphics.set_pipeline<PSOS::DrawBox>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_index_buffer(index_buffer.get_index_buffer_view());
			{
				Slots::DrawStencil draw;
				draw.GetVertices() = vertex_buffer;
				graphics.set(draw);
			}
			for (auto& sel : selected)
			{
				auto& m = sel.first->rendering[sel.second];
				graphics.set(m.compiled_mesh_info);
				graphics.set(m.mesh_instance_info);
				graphics.draw_indexed(36, 0, 0);
			}
		}

		// draw axis
		{
			graphics.set_index_buffer(HAL::Views::IndexBuffer());
			{
				Slots::FrameInfo frameInfo;
				frameInfo.GetCamera() = axis_cam.camera_cb.current;
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
					color.GetColor() = { i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted, 1 };
					graphics.set(color);
				}
				graphics.set(m.compiled_mesh_info);
				graphics.set(m.mesh_instance_info);
				graphics.dispatch_mesh(m.dispatch_mesh_arguments);
				i++;
			}
		}

		// draw rotation rings (camera-facing half; back half discarded in the PS)
		if (rings_sized)
		{
			graphics.set_pipeline<PSOS::DrawRing>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_index_buffer(ring_index_buffer.get_index_buffer_view());
			{
				Slots::FrameInfo frameInfo;
				frameInfo.GetCamera() = axis_cam.camera_cb.current;
				graphics.set(frameInfo);
			}
			{
				Slots::DrawStencil draw;
				draw.GetVertices() = ring_vertex_buffer;
				graphics.set(draw);
			}
			for (int a = 0; a < 3; a++)
			{
				float lighted = (mouse_on_axis == 3 + a) * 0.7f;
				{
					Slots::Color color;
					color.GetColor() = { a == 0 ? 1.0f : lighted, a == 1 ? 1.0f : lighted, a == 2 ? 1.0f : lighted, 1 };
					graphics.set(color);
				}
				graphics.draw_indexed(ring_ranges[a].count, ring_ranges[a].offset, 0);
			}
		}
	};
}
