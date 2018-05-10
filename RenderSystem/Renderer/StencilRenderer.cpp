#include "pch.h"
using namespace GUI;

using namespace Elements;

void stencil_renderer::render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj)
{

	axis->update_transforms();
    cam = *mesh_render_context->cam;
	cam.set_projection_params(0.1, 10000);

    cam.target = cam.position + direction;
//	cam.set_projection_params(cam.angle)
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
    {
        // int counter = 1;
        current_frame = (current_frame + 1) % 2;
        auto& current = all[current_frame];
        current.clear();
        auto mesh_func = [&](MeshAssetInstance * l)
        {
       //     auto nodes = l->node_buffer.get_for(*graphics.get_manager());

			auto nodes = l->node_buffer.get_for(*graphics.get_manager());
			graphics.set_srv(6, nodes.resource->get_gpu_address() + nodes.offset);


            for (unsigned int i = 0; i < l->rendering.size(); i++)
            {
                auto& e = l->rendering[i];
                auto& node = l->nodes[e.node_index];

               auto in = intersect(cam, e.mesh->primitive.get());

       //        if (!in) continue;

                current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);
                graphics.set(0, l->mesh_asset->vertex_buffer->get_gpu_address());
                graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
              //  graphics.set_const_buffer(2, nodes.resource->get_gpu_address() + nodes.offset + e.node_index * sizeof(decltype(l->node_buffer)::type));
				graphics.set_constants(5, 0, e.node_index);

                graphics.set_constants(3, current.size());
                graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
                //    mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
            }
        }; 
        graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        graphics.set_pipeline(draw_state);
        graphics.transition(depth_tex, Render::ResourceState::DEPTH_WRITE);
	    graphics.transition(id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		graphics.transition(axis_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		graphics.flush_transitions();
	//	UINT values[4] = { 0 };
	///	graphics.get_native_list()->ClearUnorderedAccessViewUint(id_buffer->get_uav().get_base().gpu, id_buffer->get_static_uav().get_base().cpu, id_buffer->get_native().Get(), values, 0, nullptr);
	//	graphics.get_native_list()->ClearUnorderedAccessViewUint(axis_id_buffer->get_uav().get_base().gpu, axis_id_buffer->get_static_uav().get_base().cpu, axis_id_buffer->get_native().Get(), values, 0, nullptr);
		graphics.clear_uav(id_buffer, id_buffer->get_uav()[0]);
		graphics.clear_uav(axis_id_buffer, axis_id_buffer->get_uav()[0]);

		graphics.set(1, cam.get_const_buffer());
        graphics.set_uav(4, id_buffer->get_gpu_address());
        table.clear_depth(graphics);
        table.set(graphics);
        graphics.set_viewports({ depth_tex->texture_2d()->get_viewport() });
        graphics.set_scissors(depth_tex->texture_2d()->get_scissor());
        //    if (need_update)
        obj->iterate([&](scene_object * node)
        {
            Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

            if (render_object)
            {
                auto instance = dynamic_cast<MeshAssetInstance*>(render_object);
                //	thread_pool::get().enqueue(std::bind(mesh_func, instance));
                mesh_func(instance);
            }

            return true;
        });
        // if (need_update)
        table.clear_depth(graphics);
        graphics.set_uav(4, axis_id_buffer->get_gpu_address());
        graphics.set(1, axis_intersect_cam.get_const_buffer());
        axis->iterate([&](scene_object * node)
        {
            Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

            if (render_object)
            {
                auto l = dynamic_cast<MeshAssetInstance*>(render_object);
                //	thread_pool::get().enqueue(std::bind(mesh_func, instance));
				auto nodes = l->node_buffer.get_for(*graphics.get_manager());
				graphics.set_srv(6, nodes.resource->get_gpu_address() + nodes.offset);
				graphics.set(0, l->mesh_asset->vertex_buffer->get_gpu_address());
				graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));


                for (unsigned int i = 0; i < l->rendering.size(); i++)
                {
                    auto& e = l->rendering[i];
                    auto& node = l->nodes[e.node_index];
                 //    auto in = intersect(cam, e.mesh->primitive.get());
                    //    if (!in) continue;
                    //	current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);
                     //graphics.set_const_buffer(2, nodes.resource->get_gpu_address() + nodes.offset + e.node_index * sizeof(decltype(l->node_buffer)::type));
					graphics.set_constants(5,0, e.node_index);
                    graphics.set_constants(3, i + 1);
                    graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
                    //    mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
                }
            }

            return true;
        });
        //     if (need_update)
        graphics.read_buffer(id_buffer.get(), 0, 4, [current, this](const char* data, UINT64 size)
        {
            //  current_selected.clear();
            if (!data) device_fail();

            UINT result = *reinterpret_cast<const UINT*>(data) - 1;
            mouse_on_object.first = nullptr;

            if (result >= current.size())
                return;

            mouse_on_object = (current[result]);
        });
        graphics.read_buffer(axis_id_buffer.get(), 0, 4, [current, this](const char* data, UINT64 size)
        {
            mouse_on_axis = *reinterpret_cast<const UINT*>(data) - 1;
        });
        //    need_update = false;
    }
}


bool stencil_renderer::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    if (action == mouse_action::DOWN&&button == mouse_button::LEFT)
    {
        if (selected.empty()||mouse_on_axis == -1)
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
					draw.mesh->material = mouse_on_object.first->register_material( base_mat);


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
    mouse_pos = get_current_pos() ;

    if (is_pressed() && selected_axis != -1)
    {
	//	Log::get() << "axis " << selected_axis << Log::endl;


        center_pos += (mouse_pos - prev_mouse_pos) * get_axis(selected_axis);
   //     Log::get() << NVP(center_pos) << Log::endl;
		  if (selected.size())
        {
            auto& draw = selected[0].first->rendering[selected[0].second];
      //      selected[0].first->nodes[draw.node_index]->mesh_matrix[3] = float4(center_pos + pivot_pos, 1);

			run_on_ui([this]() {
			

				selected[0].first->local_transform[3] = float4(center_pos + pivot_pos, 1);
				selected[0].first->update_layout();
				//selected[0].first->update_nodes();
			
			});


        }
    }

    return base::on_mouse_move(pos) | true;;
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
    Render::RootSignatureDesc root_desc;
    root_desc[0] = Render::DescriptorSRV(0, Render::ShaderVisibility::VERTEX); //vertex buffer
    root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::VERTEX);
    root_desc[2] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::VERTEX);
    root_desc[3] = Render::DescriptorConstants(0, 3, Render::ShaderVisibility::PIXEL);
    root_desc[4] = Render::DescriptorUAV(0, Render::ShaderVisibility::PIXEL); // camera
    root_desc[5] = Render::DescriptorConstants(2, 3, Render::ShaderVisibility::VERTEX);
	root_desc[6] = Render::DescriptorSRV(1, Render::ShaderVisibility::VERTEX, 1); // instance data
	//root_desc[7] = Render::DescriptorConstants(2, 2, Render::ShaderVisibility::ALL); // material offsets

    state_desc.root_signature.reset(new Render::RootSignature(root_desc));
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
    state_desc.rtv.rtv_formats = {DXGI_FORMAT::DXGI_FORMAT_R8_SNORM };
    state_desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
    state_desc.rtv.enable_depth_write = false;
    //state_desc.blend.render_target[0].enabled = true;
    //state_desc.blend.render_target[0].enabled = true;
    draw_selected_state.reset(new Render::PipelineState(state_desc));
    state_desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
    state_desc.vertex = Render::vertex_shader::get_resource({ "shaders/stencil.hlsl", "VS_COLOR", 0, {} });
    state_desc.pixel = Render::pixel_shader::get_resource({ "shaders/stencil.hlsl", "PS_COLOR", 0, {} });
    axis_render_state.reset(new Render::PipelineState(state_desc));
    id_buffer.reset(new Render::StructuredBuffer<UINT>(1, false, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
    axis_id_buffer.reset(new Render::StructuredBuffer<UINT>(1, false, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));


	id_buffer->set_name("stencil_renderer::id_buffer");
	axis_id_buffer->set_name("stencil_renderer::axis_id_buffer");
	depth_tex->set_name("stencil_renderer::depth_tex");


    cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);
    axis_intersect_cam.set_projection_params(0, 0.01f, 0, 0.01f, 0.1f, 1000);
    //  axis_cam.set_projection_params(-100, 100, -100, 100, -100, 100);
    {
        Render::PipelineStateDesc state_desc;
        Render::RootSignatureDesc root_desc;
        root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1);
        root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
        state_desc.root_signature.reset(new Render::RootSignature(root_desc));
        state_desc.blend.render_target[0].enabled = true;
        state_desc.blend.render_target[0].source = D3D12_BLEND_ONE;
        state_desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
        state_desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
        state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\contour.hlsl", "PS", 0, {} });
        state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\contour.hlsl", "VS", 0, {} });
        state_desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        last_render_state.reset(new Render::PipelineState(state_desc));
        //	state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\contour.hlsl", "PS", 0,{} });
        //	state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\triangle.hlsl", "VS", 0,{} });
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
    //selected.clear();
    if (selected.empty()) return;


    /*
    for (int i = 0; i < 3; i++)
    {
        selected.emplace_back(axis, i);
        //selected.first = axis;
        //selected.second = 0;
    }
    */
    auto& graphics = mesh_render_context->list->get_graphics();
    graphics.transition(color_tex, ResourceState::RENDER_TARGET);
    graphics.set_rtv(1, color_tex->texture_2d()->get_rtv(), Handle());
    color_tex->texture_2d()->get_rtv().clear(graphics);
    graphics.set_pipeline(draw_selected_state);
    graphics.set(1, mesh_render_context->cam->get_const_buffer());
    //graphics.set(1, axis_cam.get_const_buffer());
    graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

//	mesh_render_context->cam->update();
    for (auto& sel : selected)
    {
        auto l = sel.first;
        //  render_color_table.set(graphics);
        auto nodes = l->node_buffer.get_for(*graphics.get_manager());
        int i = sel.second;
        //    for (int i = 0; i < l->rendering.size(); i++)
        {
            auto& e = l->rendering[i];
            auto& node = l->nodes[e.node_index];

			auto nodes = l->node_buffer.get_for(*graphics.get_manager());
			graphics.set_srv(6, nodes.resource->get_gpu_address() + nodes.offset);

            // auto in = intersect(cam, e.mesh->primitive.get(), node->mesh_matrix);
            // if (!in) continue;
            graphics.set(0, l->mesh_asset->vertex_buffer->get_gpu_address());
            graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
          //  graphics.set_const_buffer(2, nodes.resource->get_gpu_address() + nodes.offset + e.node_index * sizeof(decltype(l->node_buffer)::type));
			graphics.set_constants(5, 0, e.node_index);

			graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
            //    mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
        }
    }

    graphics.set_pipeline(last_render_state);
    graphics.transition(color_tex, ResourceState::PIXEL_SHADER_RESOURCE);
    graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    graphics.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport());
    graphics.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor());
    graphics.set_rtv(1, buffer.result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
    graphics.set_dynamic(0,0, color_tex->texture_2d()->get_srv());
    {
        auto timer = graphics.start(L"blend");
        graphics.draw(4);
    }
    graphics.set_pipeline(axis_render_state);
    // graphics.set_pipeline(draw_selected_state);
    graphics.set(1, axis_cam.get_const_buffer());
    auto nodes = axis->node_buffer.get_for(*graphics.get_manager());
    graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    int i = 0;
    //  vec3 p = center_pos;
    // p += added;
    graphics.set_constants(5, 0, 0, 0);
	//mesh_render_context->cam->set_projection_params(0.1,1000);
	//mesh_render_context->set_frame_data(mesh_render_context->cam->get_const_buffer());

	graphics.set_srv(6, nodes.resource->get_gpu_address() + nodes.offset);


	for (auto& e : axis->rendering)
    {
        auto& node = axis->nodes[e.node_index];


        // auto in = intersect(cam, e.mesh->primitive.get(), node->mesh_matrix);
        // if (!in) continue;
        float lighted = (mouse_on_axis == i) * 0.7f;
        graphics.set_constants(3,  i == 0 ? 1.0f : lighted,  i == 1 ? 1.0f : lighted,  i == 2 ? 1.0f : lighted);
        i++;
        graphics.set(0, axis->mesh_asset->vertex_buffer->get_gpu_address());
        graphics.set_index_buffer(axis->mesh_asset->index_buffer->get_index_buffer_view(true));
    //    graphics.set_const_buffer(2, nodes.resource->get_gpu_address() + nodes.offset + e.node_index * sizeof(decltype(axis->node_buffer)::type));
		graphics.set_constants(5, 0, e.node_index);

        graphics.draw_indexed(e.mesh->index_count, e.mesh->index_offset, e.mesh->vertex_offset);
        //    mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
    }
}

void stencil_renderer::iterate(MESH_TYPE mesh_type,  std::function<void(scene_object::ptr &) > f)
{
	throw std::logic_error("The method or operation is not implemented.");
}
