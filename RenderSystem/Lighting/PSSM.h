class PSSM
{
	Render::Texture::ptr depth_tex;
	Render::ComputePipelineState::ptr state;
		Render::PipelineState::ptr draw_mask_state;
		Render::PipelineState::ptr draw_result_state;

	ShaderMaterial::ptr mat;
	Render::HandleTable compareSampler;

	ivec2 size = { 1024, 1024 };
	float2 pixel_size = float2(1, 1) / float2(1024, 1024);
	std::vector<camera::shader_params> srv_buffer;

	std::vector<std::future<bool>> results;

	std::vector<int > counts;


	float scaler = 1;

	Render::Texture::ptr full_scene_tex;
	RenderTargetTable full_scene_table;
	MeshRenderContext::ptr full_scene_context;


	struct one_renderer
	{
		RenderTargetTable table;
		std::vector<sizer_long> scissor;
		std::vector<Render::Viewport> viewport;
		camera light_cam;
		MeshRenderContext::ptr context;
	};

	std::array<one_renderer, 6> renders;


	one_renderer main_scene_render;
	std::vector<camera::shader_params> main_scene_srv;

	std::mutex pos_mutex;
	float3 position;
	int counter = 0;
public:
	bool optimize_far = true;
	Render::Texture::ptr screen_light_mask;


	float3 get_position();


	void set_position(float3 p);


	void set(MeshRenderContext::ptr& context);

	void resize(ivec2 size);
	PSSM();

	void generate_shadow_map(MeshRenderContext::ptr& c, main_renderer::ptr renderer, scene_object::ptr scene);

	void end(MeshRenderContext::ptr& context);



	void process(MeshRenderContext::ptr& context);


	/*
	void process(MeshRenderContext::ptr& context, G_Buffer& buffer)
	{
		auto& timer = context->list->start(L"pssm::process");wdqw
		auto& list = context->list->get_compute();
		//   list.assume_state(depth_tex, Render::ResourceState::DEPTH_WRITE);
		list.transition(depth_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
		//    list.assume_state(depth_tex, Render::ResourceState::DEPTH_WRITE);
		context->list->transition(screen_light_mask, Render::ResourceState::UNORDERED_ACCESS);
		//   depth_tex->change_state(context->list, Render::ResourceState::DEPTH_WRITE, Render::ResourceState::PIXEL_SHADER_RESOURCE);
		context->list->get_compute().set_pipeline(state);
		context->list->get_compute().set_dynamic(0, 0, buffer.srv_table);
	//	context->list->get_compute().set_dynamic(1, 0, buffer.result_tex.first()->texture_2d()->get_static_uav());
		context->list->get_compute().set_dynamic(1, 0, screen_light_mask->texture_2d()->get_static_uav());


		context->list->get_compute().set(2, context->cam->get_const_buffer());
		context->list->get_compute().set_dynamic(3, 0, depth_tex->array2d()->get_static_srv());
		context->list->get_compute().set_srv(4, srv_buffer);
		context->list->get_compute().set(5, compareSampler);
		context->list->get_compute().set_constants(6, scaler);
		context->list->get_compute().dispach(buffer.size);

		context->list->transition(screen_light_mask, Render::ResourceState::PIXEL_SHADER_RESOURCE);

		//    context->list->transition_uav(buffer.result_tex.first().get());
		// depth_tex->change_state(context->list, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::DEPTH_WRITE);
	}*/
};
