#include "pch.h"

float jit = 0;
/*
void PSSM::generate_shadow_map(MeshRenderContext::ptr& c, main_renderer::ptr renderer, scene_object::ptr scene)
{
//   return;
	const float clearColor[] = { 0, 0, 0, 0 };
	auto cam = c->cam;
	float znear = cam->z_near;
	float zfar = 350;
	results.clear();
	jit += c->delta_time;

	if (jit > 5)
		jit = -5;

	for (int i = 0; i < tables.size(); i++)
	{
		//   zfar = (exp(i + 1)) * 15;
		results.emplace_back(thread_pool::get().enqueue([ cam, znear, zfar, i, this, renderer, scene]()->bool
		{
			auto points = cam->get_points(znear, zfar);
			auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
			list->begin();
			list->get_graphics().set_heaps(Render::DescriptorHeapManager::get().get_csu(), Render::DescriptorHeapManager::get().get_samplers());

			contexts[i]->list = list;
			list->get_graphics().set_viewports(vps);
			list->get_graphics().set_scissors(scissors[0]);
			contexts[i]->pipeline.blend.blend_enabled[0] = false;

			float2 sizes = 2 * float2(zfar - znear, zfar - znear) * (i + 1);
			float2 pixel_size = float2(size) / sizes;
			box bounds = points.get_bounds_in(light_cam[i].get_const_buffer().view);
			bounds.left = floor(bounds.left * pixel_size.x) / pixel_size.x;
			bounds.right = floor(bounds.right * pixel_size.x) / pixel_size.x;
			bounds.top = floor(bounds.top * pixel_size.y) / pixel_size.y;
			bounds.bottom = floor(bounds.bottom * pixel_size.y) / pixel_size.y;

			//float2 sizes = { bounds.right - bounds.left , bounds.bottom - bounds.top };
//
			//   sizes.x = std::max(sizes.x, sizes.y);
			// sizes.y = sizes.x;

			float2 pos = float2(bounds.right + bounds.left, bounds.bottom + bounds.top) / 2;

			//  sizes.x = 10;//floor(sizes.x * size.x) / size.x;
			//  sizes.y = 10;//floor(sizes.y * size.y) / size.y;
			//  sizes.x = floor(sizes.x * pixel_size.x) / pixel_size.x;
			//  sizes.y = floor(sizes.y * pixel_size.y) / pixel_size.y;


			//   pos.x = floor(pos.x * pixel_size.x) / pixel_size.x;
			//  pos.y = floor(pos.y * pixel_size.y) / pixel_size.y;
			//  bounds.left += floor(jit) * pixel_size.x;
			//    bounds = box(-200, -200, 1, 200, 200, 1000);
			//light_cam[i].set_projection_params(bounds.left, bounds.left + sizes.x, bounds.bottom, bounds.bottom + sizes.y, bounds.znear - 500, bounds.zfar);//bounds.znear - 500, bounds.zfar);
			light_cam[i].set_projection_params(bounds.left, bounds.right, bounds.bottom, bounds.top, bounds.znear - 500, bounds.zfar);

			//   sizes *= 1024.0f;
			light_cam[i].update();
			srv_buffer[i] = light_cam[i].get_const_buffer();
			contexts[i]->cam = &light_cam[i];
			auto&& table = tables[i];

			table.clear_depth(contexts[i]);
			table.set(contexts[i]);
			renderer->render(contexts[i], scene);

			list->end();

			list->execute();
			return true;
		}));
		//  znear = zfar;
	}

	for (auto && c : results)
		c.wait();

	//   context->cam = cam;
}

*/


float3 PSSM::get_position()
{
	pos_mutex.lock();
	auto p = position;
	pos_mutex.unlock();
	return p;
}

void PSSM::set_position(float3 p)
{
	pos_mutex.lock();
	position = p;
	pos_mutex.unlock();
}

void PSSM::set(MeshRenderContext::ptr & context)
{
	context->list->transition(full_scene_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);

	context->list->get_compute().set_dynamic(3, 0, full_scene_tex->texture_2d()->get_srv());
	context->list->get_compute().set_srv(4, main_scene_srv);
}

void PSSM::resize(ivec2 size)
{
	screen_light_mask.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));

}

PSSM::PSSM()
{
	mat.reset(new ShaderMaterial());

	//	renders.resize(4);
	srv_buffer.resize(renders.size());
	depth_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, renders.size(), 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));

	full_scene_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));

	//	depth_tex->debug = true;
	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
	root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 1);
	root_desc[4] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[6] = Render::DescriptorConstants(1, 2, Render::ShaderVisibility::ALL);
	Render::ComputePipelineStateDesc desc;
	desc.shader = Render::compute_shader::get_resource({ "shaders\\PSSM.hlsl", "CS", 0,{} });
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	state.reset(new  Render::ComputePipelineState(desc));
	position = float3(200, 400, 200);
	int i = 0;
	for (auto &&r : renders)
	{
		r.table = RenderTargetTable({}, depth_tex, i);
		r.light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
		r.light_cam.position = position;//*800;
		r.light_cam.target = { 0, 0, 0 };
		r.light_cam.update();
		r.context.reset(new MeshRenderContext());


		r.viewport.resize(1);
		r.viewport[0].MinDepth = 0.0f;
		r.viewport[0].MaxDepth = 1.0f;
		r.viewport[0].Width = static_cast<float>(size.x);
		r.viewport[0].Height = static_cast<float>(size.y);
		r.viewport[0].TopLeftX = 0;
		r.viewport[0].TopLeftY = 0;

		r.scissor.resize(1);
		r.scissor[0] = { 0, 0, size.x, size.y };

		i++;
	}

	{
		main_scene_render.table = RenderTargetTable({}, full_scene_tex);
		main_scene_render.light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
		main_scene_render.light_cam.position = position;//*800;
		main_scene_render.light_cam.target = { 0, 0, 0 };
		main_scene_render.light_cam.update();
		main_scene_render.context.reset(new MeshRenderContext());


		main_scene_render.viewport.resize(1);
		main_scene_render.viewport[0].MinDepth = 0.0f;
		main_scene_render.viewport[0].MaxDepth = 1.0f;
		main_scene_render.viewport[0].Width = static_cast<float>(size.x);
		main_scene_render.viewport[0].Height = static_cast<float>(size.y);
		main_scene_render.viewport[0].TopLeftX = 0;
		main_scene_render.viewport[0].TopLeftY = 0;

		main_scene_render.scissor.resize(1);
		main_scene_render.scissor[0] = { 0, 0, size.x, size.y };
	}
	main_scene_srv.resize(1);
	//ds = Render::DescriptorHeapManager::get().get_ds()->create_table(1);
	//  depth_tex->texture_2d()->place_dsv(ds[0], 0, 0, tables.size());
	compareSampler = Render::DescriptorHeapManager::get().get_samplers()->create_table(1);
	D3D12_SAMPLER_DESC wrapSamplerDesc = {};
	wrapSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	wrapSamplerDesc.MinLOD = 0;
	wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	wrapSamplerDesc.MipLODBias = 0.0f;
	wrapSamplerDesc.MaxAnisotropy = 16;
	wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
	Render::Device::get().create_sampler(wrapSamplerDesc, compareSampler[0].cpu);






	{
		Render::PipelineStateDesc desc;
		{
			Render::RootSignatureDesc root_desc;
			//		root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
			//	root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
			//	root_desc[2] = Render::DescriptorConstants(1, 3, Render::ShaderVisibility::PIXEL);

			root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
			root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
			root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 1);
			root_desc[3] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
			//		root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 1);
			root_desc[4] = Render::DescriptorConstants(1, 2, Render::ShaderVisibility::ALL);


			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
			root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
			desc.root_signature.reset(new Render::RootSignature(root_desc));
		}
		desc.rtv.rtv_formats = { DXGI_FORMAT_R8_UNORM };
		desc.blend.render_target[0].enabled = false;
		desc.blend.render_target[0].source = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.blend.render_target[0].dest = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\PSSM.hlsl", "PS", 0,{} });
		desc.vertex = Render::vertex_shader::get_resource({ "shaders\\PSSM.hlsl", "VS", 0,{} });
		draw_mask_state.reset(new  Render::PipelineState(desc));

		desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\PSSM.hlsl", "PS_RESULT", 0,{} });
		draw_result_state.reset(new  Render::PipelineState(desc));
	}
}

void PSSM::generate_shadow_map(MeshRenderContext::ptr& c, main_renderer::ptr renderer, scene_object::ptr scene)
{
	auto timer = c->list->start(L"PSSM::generate_shadow_map");


	auto min = scene->get_min();
	auto max = scene->get_max();

	auto points_all = c->cam->get_points(min, max);


	float one_pixel_size = (max - min).max_element() / size.x;


	float3 center = (max + min) / 2;







	const float clearColor[] = { 0, 0, 0, 0 };
	auto cam = c->cam;
	float znear = cam->z_near;
	float zfar = 50;
	results.clear();


	scaler = cam->z_far / (exp(renders.size()));
	//scaler = cam->z_far / ((renders.size()));

	static float t = 0;
	// t += c->delta_time;
   // position = {sin(t), 5, cos(t)};

	float3 current_pos = get_position();

	//auto state = depth_tex->get_state(0);
//	depth_tex->assume_state(Render::ResourceState::DEPTH_WRITE);
	counter++;
	if (counter == renders.size())
		counter = 0;

	if (!optimize_far)
		counter = renders.size() - 1;

	for (int i = 0; i <= counter; i++)
	{
		zfar = cam->z_near + (exp(i + 1)) * scaler;
		//	zfar = znear + (i + 1)*scaler;
		auto & render = renders[i];

		render.light_cam.position = current_pos;//*800;
		render.light_cam.up = (float3(0.01, 1, 0.023)).normalize();;
		render.light_cam.update();
		auto points = cam->get_points(znear, zfar);

		auto bounds_all = points_all.get_bounds_in(render.light_cam.get_const_buffer().data().current.view);


		box bounds = points.get_bounds_in(render.light_cam.get_const_buffer().data().current.view);
		render.light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, std::min(bounds_all.znear - 10, bounds.znear), bounds.zfar + 5000);

		render.light_cam.update();
		render.light_cam.get_const_buffer().get_for(*c->list->frame_resources);
		srv_buffer[i] = render.light_cam.get_const_buffer().data().current;

		auto list = c->list->get_sub_list();
		results.emplace_back(thread_pool::get().enqueue([cam, znear, zfar, i, this, renderer, scene, list]()->bool
		{
			auto & render = renders[i];
			//		Log::get() << "beg " << i << Log::endl;

			list->begin(std::string("shadow_" + std::to_string(i)));
			list->get_graphics().set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

			list->transition(depth_tex, Render::ResourceState::DEPTH_WRITE, i);
			list->flush_transitions();

			render.context->override_material = mat;
			render.context->list = list;


			list->get_graphics().set_viewports(render.viewport);
			list->get_graphics().set_scissors(render.scissor[0]);
			render.context->pipeline.blend.render_target[0].enabled = false;
			render.context->pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;


			render.context->cam = &render.light_cam;
			auto&& table = render.table;

			table.clear_depth(render.context);
			table.set(render.context);
			renderer->render(render.context, scene);

			list->end();

			list->execute();
			//	Log::get() << "end " << i << Log::endl;


			return true;
		}, c->priority));
		znear = zfar;
	}

	auto & render = main_scene_render;

	render.light_cam.position = current_pos;//*800;
	render.light_cam.up = (float3(0.01, 1, 0.023)).normalize();;
	render.light_cam.update();
	//	auto points = cam->get_points(scene->childs_occluder.primitive->get_min(), scene->childs_occluder.primitive->get_max());
	auto points = cam->get_points(min, max);

	box bounds = points.get_bounds_in(render.light_cam.get_const_buffer().data().current.view);

	render.light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, bounds.znear - 500, bounds.zfar + 5);
	render.light_cam.update();
	render.light_cam.get_const_buffer().get_for(*c->list->frame_resources);


	//	auto type = full_scene_tex->get_state(0);
		//full_scene_tex->assume_state(Render::ResourceState::DEPTH_WRITE);
	auto list = c->list->get_sub_list();
	main_scene_srv[0] = render.light_cam.get_const_buffer().data().current;

	results.emplace_back(thread_pool::get().enqueue([this, cam, renderer, scene, list]()->bool
	{
		auto & render = main_scene_render;


		//auto list = c->list->get_sub_list();
		list->begin(std::string("shadow_full"));
		list->get_graphics().set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

		list->transition(full_scene_tex, Render::ResourceState::DEPTH_WRITE);
		list->flush_transitions();

		render.context->override_material = mat;
		render.context->list = list;


		list->get_graphics().set_viewports(render.viewport);
		list->get_graphics().set_scissors(render.scissor[0]);
		render.context->pipeline.blend.render_target[0].enabled = false;
		render.context->pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;


		render.context->cam = &render.light_cam;
		auto&& table = render.table;

		table.clear_depth(render.context);
		table.set(render.context);
		{
			auto timer = list->start(L"draw");
			renderer->render(render.context, scene);

		}

		list->end();

		list->execute();
		return true;
	}));

	//for (auto && c : results)
	//	c.wait();

	//   context->cam = cam;
}

void PSSM::end(MeshRenderContext::ptr & context)
{
	/*         auto& list = context->list;
	albedo_tex->change_state(list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	normal_tex->change_state(list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	specular_tex->change_state(list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	depth_tex->change_state(list, Render::ResourceState::DEPTH_WRITE, Render::ResourceState::PIXEL_SHADER_RESOURCE);*/
}

void PSSM::process(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"pssm");
	auto& list = context->list->get_graphics();



	for (auto &eye : context->eye_context.eyes)
	{


		list.transition(screen_light_mask, Render::ResourceState::RENDER_TARGET);
		auto& view = screen_light_mask->texture_2d();
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		list.set_rtv(1, view->get_rtv(), Render::Handle());
		list.set_pipeline(draw_mask_state);

		list.set_dynamic(0, 0, eye.g_buffer->srv_table);


		list.set(1, eye.cam->get_const_buffer());
		list.set_dynamic(2, 0, depth_tex->array2d()->get_static_srv());
		list.set_srv(3, srv_buffer);

		//	list.set_dynamic(0, 0, table);
		//	list.set(1, context->cam->get_const_buffer());
	//	list.set_constants(2, context->sky_dir.x, context->sky_dir.y, context->sky_dir.z);
		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for (int i = renders.size() - 1; i >= 0; i--)
		{
			auto timer = context->list->start((std::wstring(L"renders") + std::to_wstring(i)).c_str());

			list.set_constants(4, i, ((context->current_time) % 10000) * 0.001f);

			list.draw(4);

		}



		//	list.set_pipeline(draw_mask_state_local);

		//	list.draw(4);
		list.transition(screen_light_mask, Render::ResourceState::PIXEL_SHADER_RESOURCE);




		////////////////////////////////////////////////////////////////
		list.set_rtv(1, eye.g_buffer->result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
		list.transition(eye.g_buffer->result_tex.first(), Render::ResourceState::RENDER_TARGET);
		list.set_pipeline(draw_result_state);
		list.set_dynamic(2, 0, screen_light_mask->texture_2d()->get_static_srv());

		list.draw(4);
	}
}
