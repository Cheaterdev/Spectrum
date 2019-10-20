#include "pch.h"

float jit = 0;

template <class T>
struct PSSMCompute : public T
{
	using T::T;

	//eye
	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4>		g_buffer = this;
	 typename T::template ConstBuffer	<1, Render::ShaderVisibility::ALL, 0>										camera_info = this;

	//pass

	 typename T::template Constants		<2, Render::ShaderVisibility::ALL, 1, 2>									pass_constants = this;

	//effect
	 typename T::template Table			<3, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 6, 2>		light_buffer_brdf = this;

	 typename T::template Table			<4, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 4, 1>		cameras_depth = this;
	 typename T::template SRV			<5, Render::ShaderVisibility::ALL, 5>										cameras_buffer = this;


	 typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };
	 typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };
	 typename T::template Sampler<2, Render::ShaderVisibility::ALL, 0> aniso{ Render::Samplers::SamplerLinearClampDesc, this };

};


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

void PSSM::set(MeshRenderContext::ptr& context)
{
	context->list->transition(full_scene_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	((SignatureDataSetter*)&context->list->get_compute())->set_dynamic(3, 0, full_scene_tex->texture_2d()->get_srv());
	((SignatureDataSetter*)&context->list->get_compute())->set_srv(4, main_scene_srv);
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
	depth_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, (UINT16)renders.size(), 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));

	full_scene_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::DEPTH_WRITE));

	position = float3(200, 400, 200);
	int i = 0;
	for (auto&& r : renders)
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

	{
		Render::PipelineStateDesc desc;
		desc.root_signature = PSSMCompute<SignatureCreator>().create_root();

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


	scaler = cam->z_far / (exp((float)renders.size()));
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
		zfar = cam->z_near + (exp(float(i + 1))) * scaler;
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
				auto& render = renders[i];
				//		Log::get() << "beg " << i << Log::endl;

				list->begin(std::string("shadow_" + std::to_string(i)));
				list->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

				list->transition(depth_tex, Render::ResourceState::DEPTH_WRITE, i);
				list->flush_transitions();

				render.context->override_material = mat;
				render.context->list = list;


				list->get_graphics().set_viewports(render.viewport);
				list->get_graphics().set_scissors(render.scissor[0]);
				render.context->pipeline.blend.render_target[0].enabled = false;
				render.context->pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;


				render.context->cam = &render.light_cam;
				auto && table = render.table;

				table.clear_depth(render.context);
				table.set(render.context);
				renderer->render(render.context, scene);

				list->end();

				list->execute();
				//	Log::get() << "end " << i << Log::endl;


				return true;
			}));
		znear = zfar;
	}

	auto & render = main_scene_render;

	render.light_cam.position = current_pos;//*800;
	render.light_cam.up = (float3(0.01, 1, 0.023)).normalize();;
	render.light_cam.update();
//		auto points = cam->get_points(scene->childs_occluder.primitive->get_min(), scene->childs_occluder.primitive->get_max());
	auto points = cam->get_points(min, max);

	box bounds = points.get_bounds_in(render.light_cam.get_const_buffer().data().current.view);

	render.light_cam.set_projection_params(bounds.left - 1, bounds.right + 1, bounds.top - 1, bounds.bottom + 1, bounds.znear - 500, bounds.zfar + 5);
	render.light_cam.update();
	render.light_cam.get_const_buffer().get_for(*c->list->frame_resources);

	auto list = c->list->get_sub_list();
	main_scene_srv[0] = render.light_cam.get_const_buffer().data().current;

	results.emplace_back(thread_pool::get().enqueue([this, cam, renderer, scene, list]()->bool
		{
			auto& render = main_scene_render;


			//auto list = c->list->get_sub_list();
			list->begin(std::string("shadow_full"));
			list->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());

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

}

void PSSM::process(MeshRenderContext::ptr & context)
{
	auto timer = context->list->start(L"pssm");
	auto& list = *context->list;

	auto& graphics = context->list->get_graphics();

	PSSMCompute<Signature> sig(&graphics);
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto& view = screen_light_mask->texture_2d();
	graphics.set_viewport(view->get_viewport());
	graphics.set_scissor(view->get_scissor());

	graphics.set_signature(draw_mask_state->desc.root_signature);


	sig.cameras_depth[ 0]= depth_tex->array2d()->get_static_srv();
	sig.cameras_buffer=srv_buffer;
	sig.light_buffer_brdf[0]= screen_light_mask->texture_2d()->get_static_srv();
	sig.light_buffer_brdf[1]=EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();

	for (auto& eye : context->eye_context->eyes)
	{
		list.transition(screen_light_mask, Render::ResourceState::RENDER_TARGET);
		graphics.set_rtv(1, view->get_rtv(), Render::Handle());


		graphics.set_pipeline(draw_mask_state);

	sig.g_buffer[0]=eye.g_buffer->srv_table;
	sig.camera_info=eye.cam->get_const_buffer();



	for (int i = (int)renders.size() - 1; i >= 0; i--) //rangeees
		{
			auto timer = context->list->start((std::wstring(L"renders") + std::to_wstring(i)).c_str());
			sig.pass_constants.set(i, ((context->current_time) % 10000) * 0.001f);

			graphics.draw(4);
		}

		list.transition(screen_light_mask, Render::ResourceState::PIXEL_SHADER_RESOURCE);




		////////////////////////////////////////////////////////////////
		graphics.set_rtv(1, eye.g_buffer->result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
		list.transition(eye.g_buffer->result_tex.first(), Render::ResourceState::RENDER_TARGET);
		graphics.set_pipeline(draw_result_state);
	
		graphics.draw(4);
	}
}
