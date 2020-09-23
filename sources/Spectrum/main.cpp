#include "pch.h"
#include <filesystem>

#ifdef OCULUS_SUPPORT
// Include the Oculus SDK
#include "OVR_CAPI_D3D.h"
ovrEyeRenderDesc eyeRenderDesc[2];

#endif


HRESULT device_fail()
{
	auto hr = DX12::Device::get().get_native_device()->GetDeviceRemovedReason();
	Log::get().crash_error(hr);
	return hr;
}


class tick_timer
{
	std::chrono::time_point<std::chrono::system_clock> last_tick;

public:
	tick_timer()
	{
		last_tick = std::chrono::system_clock::now();
	}

	double tick()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = now - last_tick;
		last_tick = now;
		return elapsed_seconds.count();
	}
};

class count_meter
{
	double time = 0;
	tick_timer t;
	unsigned int ticks = 0;

	double average = 0;
public:

	bool tick()
	{
		time += t.tick();
		ticks++;

		if (time > 1)
		{
			average = ticks / time;
			ticks = 0;
			time = 0;
			return true;
		}

		return false;
	}

	float get()
	{
		return (float)average;
	}
};


class triangle_drawer : public GUI::Elements::image, public FrameGraphGenerator
{
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;
	stencil_renderer::ptr stenciler;





	GUI::Elements::label::ptr info;

	size_t time = 0;
	//ViewPortRenderer last_renderer;
	struct EyeData : public Events::prop_handler
	{

		using ptr = std::shared_ptr<EyeData>;
		//	G_Buffer g_buffer;
		first_person_camera cam;

		//	TemporalAA temporal;

		EyeData(Render::RootSignature::ptr sig)
		{
			//	g_buffer.init(sig);// gpu_meshes_renderer_static->my_signature);

			/*	g_buffer.size.register_change(this, [this](const ivec2& size) {
					if(size.x>0)
					temporal.resize(size);

					});*/
		}
	};

	std::vector<EyeData::ptr> eyes;
	first_person_camera cam;
public:
	using ptr = std::shared_ptr<triangle_drawer>;
	//	PostProcessGraph::ptr render_graph;


	Variable<bool> enable_gi = Variable<bool>(true, "enable_gi");
	Variable<bool> debug_draw = Variable<bool>(false, "debug_draw");
	//	VoxelGI::ptr voxel_renderer;

	int visible_count;

	mesh_renderer::ptr meshes_renderer;

	//	gpu_cached_renderer::ptr gpu_meshes_renderer_static;
	//	gpu_cached_renderer::ptr gpu_meshes_renderer_dynamic;

	Scene::ptr scene;
	Render::QueryHeap::ptr query_heap;
	float draw_time;
	//	std::shared_ptr<LightingNode> lighting;

	Variable<bool> realtime_debug = Variable<bool>(false, "realtime_debug");
	MeshAssetInstance::ptr instance;


	Render::RaytracingAccelerationStructure::ptr scene_as;


	// Build acceleration structures needed for raytracing.
	void BuildAccelerationStructures()
	{
		std::vector<D3D12_RAYTRACING_INSTANCE_DESC>  desc;

		scene_as = std::make_shared<RaytracingAccelerationStructure>(desc);

		RTX::get().CreateSharedCollection();
		RTX::get().CreateGlobalCollection();
		RTX::get().CreateRaytracingPipelineStateObject();
	}

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	PSSM pssm;
	SMAA smaa;
	SkyRender sky;
	VoxelGI::ptr voxel_gi;
	std::string debug_view;
	triangle_drawer()
	{
		texture.srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);

		BuildAccelerationStructures();

		auto t = CounterManager::get().start_count<triangle_drawer>();
		thinkable = true;
		clickable = true;

		scene.reset(new Scene());
		scene->name = L"Scene";

		query_heap = std::make_shared<  Render::QueryHeap>(2, D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS);
		scene_renderer = std::make_shared<main_renderer>();
		scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());


		gpu_scene_renderer = std::make_shared<main_renderer>();

		gpu_scene_renderer->register_renderer(std::make_shared<mesh_renderer>());


		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_static = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::STATIC));
		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_dynamic = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::DYNAMIC));
		//cam.position = vec3(0, 5, -30);

		cam.position = vec3(0, 0, 0);
		Render::PipelineStateCache::create();
		stenciler.reset(new stencil_renderer());
		stenciler->player_cam = &cam;
		stenciler->scene = scene;
		add_child(stenciler);

		info.reset(new GUI::Elements::label);
		info->docking = GUI::dock::TOP;
		info->x_type = GUI::pos_x_type::LEFT;
		info->magnet_text = FW1_LEFT;
		add_child(info);
		base::ptr props(new base);
		props->docking = GUI::dock::FILL;
		props->x_type = GUI::pos_x_type::LEFT;
		props->y_type = GUI::pos_y_type::TOP;
		props->width_size = GUI::size_type::MATCH_CHILDREN;
		props->height_size = GUI::size_type::MATCH_CHILDREN;
		add_child(props);

		//props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_parrallel));
		////props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_cpu_culling));
		//props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_gpu_culling));
	//	props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->clear_depth));
	//	props->add_child(std::make_shared<GUI::Elements::check_box_text>(enable_gi));
	//	props->add_child(std::make_shared<GUI::Elements::check_box_text>(realtime_debug));


		props->add_child(std::make_shared<GUI::Elements::check_box_text>(debug_draw));

		{
			auto combo = std::make_shared<GUI::Elements::combo_box>();

			combo->size = { 200, 25 };
			combo->on_click = [this](GUI::Elements::button::ptr butt) {

				GUI::Elements::combo_box::ptr combo = butt->get_ptr<GUI::Elements::combo_box>();
				combo->remove_items();


				combo->add_item("Normal")->on_click = [this](GUI::Elements::menu_list_element::ptr) {debug_view = ""; };


				for (auto& e : last_graph->builder.resources)
				{

					if (e.second.info->type != ::ResourceType::Texture) continue;

					std::string str = e.first;
					combo->add_item(str)->on_click = [this, str](GUI::Elements::menu_list_element::ptr) {debug_view = str; };

				}

			};




			props->add_child(combo);
		}


		GUI::Elements::circle_selector::ptr circle(new  GUI::Elements::circle_selector);
		circle->docking = GUI::dock::FILL;
		circle->x_type = GUI::pos_x_type::RIGHT;
		circle->y_type = GUI::pos_y_type::TOP;


		add_child(circle);
		//	lighting = std::make_shared<LightingNode>();

		circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
				run_on_ui([this, v]() {
					float3 dir = { v.x,sqrt(1.001 - v.length_squared()),-v.y };

					pssm.set_position(dir);
					//lighting->lighting.pssm.set_position(dir);
					});

			});

		circle->set_value({ 1,0 });



		MeshAsset::ptr asset_ptr = EngineAssets::material_tester.get_asset();



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



		//	auto base_mat = make_material({ 1,1,1 }, 1, 0);

		int count = -2;
		float distance = 5;
		for (int i = 0; i <= count; i++)
			for (int j = 0; j <= count; j++)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(asset_ptr));
				//	instance->override_material(0, base_mat);
				//	instance->override_material(1, make_material({ 1,0,0 }, float(i) / count, float(j) / count));

				instance->local_transform[3] = { i * distance - count * distance / 2,0,j * distance - count * distance / 2,1 };

				scene->add_child(instance);
			}

		{
			MeshAsset::ptr ruins_ptr = EngineAssets::plane.get_asset();


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform = mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}

		}


		{
			MeshAsset::ptr ruins_ptr = nullptr;// AssetManager::get().find_by_name<MeshAsset>(L"cubes.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform=mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}

		}

		eyes.emplace_back(new EyeData(nullptr));

		voxel_gi = std::make_shared<VoxelGI>(scene);
	}
	float scale_speed = 0;
	vec2 wheel_pos;
	bool on_wheel(mouse_wheel type, float value, vec2 pos)
	{
		scale_speed += value * 0.1f;
		wheel_pos = pos;
		return true;

	}

	void think(float dt)
	{
		//	if (!g_buffer.size.has_value())
			//	return;

		scene->update_transforms();

		auto min = scene->get_min();
		auto max = scene->get_max();


		auto points = cam.get_points(min, max);
		auto bounds = points.get_bounds_in(cam.get_view());

		//cam.set_projection_params(std::max(1.0f, bounds.znear - 1), bounds.zfar + 1);
		cam.set_projection_params(1, 1500);


		cam.frame_move(dt);

	}



	void draw_eye(Render::CommandList::ptr _list, float dt, EyeData& data, Render::Texture::ptr target)
	{

	}

	void update_texture(Render::CommandList::ptr list, float dt, const std::shared_ptr<OVRContext>& vr)
	{


	}

	FrameGraph* last_graph = nullptr;
	tick_timer my_timer;
	std::future<Render::Handle> result_tex;

	void generate(FrameGraph& graph)
	{

		last_graph = &graph;
		vr_context->eyes.resize(1);
		vr_context->eyes[0].dir = quat();

		vr_context->eyes[0].offset = vec3(0, 0, 0);

		ivec2 size = get_render_bounds().size;
		struct pass_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* sky_cubemap;

		};

		scene->update(*graph.builder.current_frame);
		graph.frame_size = size;
		graph.scene = scene.get();
		graph.renderer = gpu_scene_renderer.get();
		graph.cam = &cam;
		graph.time = my_timer.tick();
		graph.sunDir = pssm.get_position();
		cam.update({ 0,0 });


		for (int i = 0; i < eyes.size(); i++)
		{
			eyes[i]->cam = cam;
			eyes[i]->cam.update({ 0,0 });
		}


		/*
		graph.add_pass<pass_data>("UPDATE",[](pass_data& data, TaskBuilder& builder) {
			//	data.o_texture = builder.read_texture("swapchain");

			}, [this](pass_data& data, FrameContext& _context) {

				auto& command_list = _context.get_list();

					MeshRenderContext::ptr context_gbuffer(new MeshRenderContext());
					context_gbuffer->list = command_list;

			});
		*/
		{

			CommandList::ptr command_list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();

			command_list->begin("pre");
			{

				SceneFrameManager::get().prepare(command_list, *scene);

				bool need_rebuild = scene->init_ras(command_list);
				SceneFrameManager::get().prepare(command_list, *scene);
				command_list->transition(scene->raytrace->buffer, ResourceState::NON_PIXEL_SHADER_RESOURCE);
				command_list->flush_transitions();

				//if (GetAsyncKeyState('O'))
				scene_as->update(command_list, scene->raytrace->max_size(), scene->raytrace->buffer->get_gpu_address(), need_rebuild);

				RTX::get().prepare(command_list);

			}

			command_list->end();

			command_list->execute();
		}


		{
			struct GBufferData
			{
				GBufferViewDesc gbuffer;

				ResourceHandler* hiz;
				ResourceHandler* hiz_uav;


			};

			graph.add_pass<GBufferData>("GBUFFER", [this, size](GBufferData& data, TaskBuilder& builder) {
				data.gbuffer.create(size, builder);
				data.gbuffer.create_mips(size, builder);
				data.gbuffer.create_quality(size, builder);


				data.hiz = builder.create_texture("GBuffer_HiZ", size / 8, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil | ResourceFlags::Static);
				data.hiz_uav = builder.create_texture("GBuffer_HiZ_UAV", size / 8, 1, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, ResourceFlags::UnorderedAccess);

				}, [this](GBufferData& data, FrameContext& _context) {

					auto& command_list = _context.get_list();

					//std::this_thread::sleep_for(1ms);
				//	gpu_scene_renderer->render(context_gbuffer, scene);
					MeshRenderContext::ptr context(new MeshRenderContext());


					context->current_time = time;
					//		context->sky_dir = lighting->lighting.pssm.get_position();
					context->priority = TaskPriority::HIGH;
					context->list = command_list;
					context->eye_context = vr_context;

					context->cam = &eyes[0]->cam;



					//				gpu_meshes_renderer_static->update(context);
						//			gpu_meshes_renderer_dynamic->update(context);

					GBuffer gbuffer = data.gbuffer.actualize(_context);

					gbuffer.rtv_table = RenderTargetTable(context->list->get_graphics(), { gbuffer.albedo, gbuffer.normals, gbuffer.specular, gbuffer.speed }, gbuffer.depth);

					gbuffer.HalfBuffer.hiZ_depth = _context.get_texture(data.hiz);
					gbuffer.HalfBuffer.hiZ_table = RenderTargetTable(context->list->get_graphics(), {  }, gbuffer.HalfBuffer.hiZ_depth);
					gbuffer.HalfBuffer.hiZ_depth_uav = _context.get_texture(data.hiz_uav);

					context->g_buffer = &gbuffer;

					gbuffer.rtv_table.set(context, true, true);
					gbuffer.rtv_table.set_window(context->list->get_graphics());

					RT::Slot::GBuffer rt_gbuffer;
					rt_gbuffer.GetAlbedo() = gbuffer.albedo.get_rtv();
					rt_gbuffer.GetNormals() = gbuffer.normals.get_rtv();
					rt_gbuffer.GetSpecular() = gbuffer.specular.get_rtv();
					rt_gbuffer.GetMotion() = gbuffer.speed.get_rtv();

					rt_gbuffer.GetDepth() = gbuffer.depth.get_dsv();

					context->gbuffer_compiled = rt_gbuffer.compile(*context->list);


					gpu_scene_renderer->render(context, scene);

					//	stenciler->render(context, scene);

					command_list->get_copy().copy_texture(gbuffer.depth_prev_mips.resource, 0, gbuffer.depth_mips.resource, 0);
					command_list->get_copy().copy_texture(gbuffer.depth_mips.resource, 0, gbuffer.depth.resource, 0);

					//	
				});
		}



		struct no
		{

		};
		graph.add_pass<no>("no", [this, &graph](no& data, TaskBuilder& builder) {
			graph.builder.create_texture("ResultTexture", graph.frame_size, 1, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
			}, [](no& data, FrameContext& _context) {});



		pssm.generate(graph);
		sky.generate(graph);
		voxel_gi->generate(graph);



		graph.add_pass<pass_data>("RAYTRACE", [](pass_data& data, TaskBuilder& builder) {
			data.o_texture = builder.need_texture("ResultTexture", ResourceFlags::UnorderedAccess);
			data.sky_cubemap = builder.need_texture("sky_cubemap", ResourceFlags::PixelRead);

			}, [this, &graph](pass_data& data, FrameContext& _context) {


				auto& command_list = _context.get_list();
				//SceneFrameManager::get().prepare(command_list, *scene);

				MeshRenderContext::ptr context(new MeshRenderContext());
				auto sky_cubemap = _context.get_texture(data.sky_cubemap);


				context->current_time = time;
				//	context->sky_dir = lighting->lighting.pssm.get_position();
				context->priority = TaskPriority::HIGH;
				context->list = command_list;
				context->eye_context = vr_context;

				auto output_tex = _context.get_texture(data.o_texture);



				command_list->get_compute().set_signature(RTX::get().global_sig);
				command_list->get_graphics().set_signature(RTX::get().global_sig);

				Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

				Slots::FrameInfo::Compiled compiledFrame;

				{
					auto timer = Profiler::get().start(L"FrameInfo");
					Slots::FrameInfo frameInfo;

					frameInfo.GetSky() = sky_cubemap.get_srv();

					auto camera = frameInfo.MapCamera();
					camera.cb = graph.cam->camera_cb.current;

					compiledFrame = frameInfo.compile(*command_list);
				}


				compiledScene.set(command_list->get_compute());
				compiledFrame.set(command_list->get_compute());


				for (int i = 0; i < eyes.size(); i++)
				{
					eyes[i]->cam = cam;

					//	eyes[i]->cam.eye_rot = vr->eyes[i].dir;
					//	eyes[i]->cam.offset = vr->eyes[i].offset;
					context->eye_context->eyes[i].cam = &eyes[i]->cam;
					eyes[i]->cam.update({ 0,0 });

					context->cam = &eyes[i]->cam;
				}
				if (GetAsyncKeyState('B'))
					RTX::get().render(context, output_tex, scene_as);
			});

		stenciler->generate_after(graph);

		smaa.generate(graph);

		struct debug_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* debug_texture;
		};

		//

		std::string res_tex = "ResultTexture";
		if (!debug_view.empty())
			res_tex = debug_view;
		auto promise = std::make_shared<std::promise<Render::Handle>>();
		result_tex = promise->get_future();

		graph.add_pass<debug_data>("DEBUG", [this, res_tex](debug_data& data, TaskBuilder& builder) {
			data.o_texture = builder.need_texture("swapchain");
			data.debug_texture = builder.need_texture(res_tex);

			}, [this, promise](debug_data& data, FrameContext& context) {

				auto output_tex = context.get_texture(data.o_texture);
				auto debug_tex = context.get_texture(data.debug_texture);

				if (!debug_tex) return;

				promise->set_value(debug_tex.get_srv());
				/*
				auto& list = context.get_list();

				list->get_graphics().set_viewport(vec4{0,0,100,100});
				list->get_graphics().set_scissor({ 0,0,100,100 });

				MipMapGenerator::get().copy_texture_2d_slow(list->get_graphics(), texture.texture, debug_tex);*/
			});

	}

	virtual void draw(Render::context& t) override
	{

		texture.srv[0].place(result_tex.get());
		image::draw(t);
	}





	virtual void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);
		//	std::this_thread::sleep_for(1s);
		if (r.w <= 64 || r.h <= 64) return;

		ivec2 size = r.size;
		//	texture.texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));

			//	g_buffer.size = { r.w, r.h };
		cam.set_projection_params(pi / 4, float(r.w) / r.h, 1, 1500);
		//	for (auto& e : eyes)
	//			e->g_buffer.size = { r.w,r.h };
	}



};



class UIWindow : public Window, public GUI::user_interface
{
	Render::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	std::future<void> task_future;
protected:
	virtual	void render()
	{

		{
			auto& timer = Profiler::get().start(L"start new frame");
			swap_chain->start_next();

		}


		GUI::user_interface::size = new_size;
		auto& timer = Profiler::get().start(L"draw");

		auto command_list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();

		{
			command_list->begin("main window", &timer);

			//	command_list->transition(swap_chain->get_current_frame(), Render::ResourceState::RENDER_TARGET);
			command_list->get_graphics().set_rtv(1, swap_chain->get_current_frame()->texture_2d()->get_rtv(), Render::Handle());
			command_list->get_graphics().set_viewports({ swap_chain->get_current_frame()->texture_2d()->get_viewport() });


			Render::context render_context(command_list, vr_context);
			render_context.delta_time = static_cast<float>(main_timer.tick());

			render_context.ovr_context->eyes.resize(1);
			render_context.ovr_context->eyes[0].dir = quat();

			render_context.ovr_context->eyes[0].offset = vec3(0, 0, 0);
			//		render_context.ovr_context.eyes[0].color_buffer = swap_chain->get_current_frame();


			{
				auto timer = command_list->start(L"draw ui");
				//draw_ui(render_context);
			}
			command_list->transition(swap_chain->get_current_frame(), Render::ResourceState::PRESENT);
			command_list->end();
		}

		{
			auto& timer = Profiler::get().start(L"execute");


			command_list->when_send([this](UINT64 res) {
				auto& timer = Profiler::get().start(L"present");

				swap_chain->present(res);
				if (Application::get().is_alive())
				{
					auto ptr = get_ptr();
					task_future = scheduler::get().enqueue([ptr, this]() {
						render();
						}, std::chrono::steady_clock::now());
				}

				});
			command_list->execute();
		}
	}

	UIWindow()
	{
		Window::input_handler = this;
		DX12::swap_chain_desc desc;
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.fullscreen = false;
		desc.stereo = false;
		desc.window = this;
		swap_chain = Render::Device::get().create_swap_chain(desc);



		set_capture = [this](bool v)
		{
			if (v)
				SetCapture(get_hwnd());
			else
				ReleaseCapture();
		};

		//	GUI::user_interface::size.register_handler(this, [this](ivec2 size) {	if (swap_chain)	swap_chain->resize(size); });
	}

	virtual ~UIWindow()
	{

		if (task_future.valid())
			task_future.wait();
	}
	void on_resize(vec2 size) override
	{
		new_size = size;
	}


	virtual void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
		if (swap_chain)	swap_chain->resize(r);
	}
};
#ifdef OCULUS_SUPPORT

//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D12 rendering.
struct OculusEyeTexture
{
	ovrSession               Session;
	ovrTextureSwapChain      TextureChain;
	ovrTextureSwapChain      DepthTextureChain;

	//	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> TexRtv;
	std::vector<Render::Texture::ptr> TexResource;

	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> DepthTexDsv;
	//std::vector<ID3D12Resource*>             DepthTex;

	OculusEyeTexture() :
		Session(nullptr),
		TextureChain(nullptr),
		DepthTextureChain(nullptr)
	{
	}

	bool Init(ovrSession session, int sizeW, int sizeH)
	{
		Session = session;

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = 1;
		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.StaticImage = ovrFalse;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;

		ovrResult result = ovr_CreateTextureSwapChainDX(session, Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_native().Get(), &desc, &TextureChain);
		if (!OVR_SUCCESS(result))
			return false;

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(Session, TextureChain, &textureCount);
		//TexRtv.resize(textureCount);
		TexResource.resize(textureCount);
		for (int i = 0; i < textureCount; ++i)
		{

			ComPtr<ID3D12Resource> res;
			result = ovr_GetTextureSwapChainBufferDX(Session, TextureChain, i, IID_PPV_ARGS(&res));
			if (!OVR_SUCCESS(result))
				return false;
			TexResource[i] = std::make_shared<Texture>(res);

			//	TexResource[i]->SetName(L"EyeColorRes");






			D3D12_RENDER_TARGET_VIEW_DESC rtvd = {};
			rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvd.ViewDimension = (1 > 1) ? D3D12_RTV_DIMENSION_TEXTURE2DMS
				: D3D12_RTV_DIMENSION_TEXTURE2D;
			//	TexRtv[i] = DIRECTX.RtvHandleProvider.AllocCpuHandle();
			//	DIRECTX.Device->CreateRenderTargetView(TexResource[i], &rtvd, TexRtv[i]);
		}

		return true;
	}
	Render::Texture::ptr GetD3DColorResource()
	{
		int index = 0;
		ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &index);
		return TexResource[index];
	}
	~OculusEyeTexture()
	{
		if (TextureChain)
		{

			ovr_DestroyTextureSwapChain(Session, TextureChain);
		}


	}



	// Commit changes
	void Commit()
	{
		ovr_CommitTextureSwapChain(Session, TextureChain);


	}
};
#endif
UINT64 frame_counter = 0;
#ifdef OCULUS_SUPPORT
class OVRRender
{
public:

	tick_timer main_timer;

	using ptr = std::shared_ptr<OVRRender>;
	triangle_drawer::ptr drawer;

	long long frameIndex = 0;



	Render::SwapChain::ptr swap_chain;

	ovrSession session;
	OculusEyeTexture* pEyeRenderTexture[2] = { nullptr, nullptr };
	// Setup VR components, filling out description
	ovrRecti eyeRenderViewport[2];

	ovrHmdDesc hmdDesc;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	virtual	void render()
	{


		ovrSessionStatus sessionStatus;
		ovr_GetSessionStatus(session, &sessionStatus);
		if (sessionStatus.ShouldQuit)
		{
			// Because the application is requested to quit, should not request retry
	//		retryCreate = false;
			return;
		}
		if (sessionStatus.ShouldRecenter)
			ovr_RecenterTrackingOrigin(session);

		if (sessionStatus.IsVisible)
		{

			// Get both eye poses simultaneously, with IPD offset already included. 
			ovrPosef EyeRenderPose[2];
			ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

			double sensorSampleTime;    // sensorSampleTime is fed into the layer later;lkjiwwwwjlwjdjjjjjjjj
			ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);
			auto command_list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();


			command_list->begin("main window");
			std::shared_ptr<Render::CommandList> label_list;

			Render::context render_context(command_list, label_list, vr_context);

			// Render Scene to Eye Buffers
			for (int eye = 0; eye < 2; ++eye)
			{

				auto tex = pEyeRenderTexture[eye]->GetD3DColorResource();

				command_list->transition(tex, Render::ResourceState::RENDER_TARGET);
				//command_list->get_graphics().set_rtv(1, tex->texture_2d()->get_rtv(), Render::Handle());
				//command_list->get_graphics().set_viewports({ tex->texture_2d()->get_viewport() });
				command_list->clear_rtv(tex->texture_2d()->get_rtv(), { eye,0,0,1 });




			}
			vr_context->eyes.resize(2);

			vr_context->eyes[0].dir = { EyeRenderPose[0].Orientation.x, EyeRenderPose[0].Orientation.y,
				-EyeRenderPose[0].Orientation.z, EyeRenderPose[0].Orientation.w };
			vr_context->eyes[0].offset = { EyeRenderPose[0].Position.x, EyeRenderPose[0].Position.y,
				-EyeRenderPose[0].Position.z };


			vr_context->eyes[1].dir = { EyeRenderPose[1].Orientation.x, EyeRenderPose[1].Orientation.y,
				-EyeRenderPose[1].Orientation.z, EyeRenderPose[1].Orientation.w };

			vr_context->eyes[1].offset = { EyeRenderPose[1].Position.x, EyeRenderPose[1].Position.y,
				-EyeRenderPose[1].Position.z };

			//	render_context.ovr_context.eyes[0].fov = eyeRenderDesc[0].Fov.;
			//render_context.ovr_context.eyes[1].fov = eyeRenderDesc[1].Fov;




			vr_context->eyes[0].color_buffer = pEyeRenderTexture[0]->GetD3DColorResource();
			vr_context->eyes[1].color_buffer = pEyeRenderTexture[1]->GetD3DColorResource();

			render_context.delta_time = static_cast<float>(main_timer.tick());


			{
				//	auto timer = command_list->start(L"draw ui");
			//		drawer->draw(render_context);
				drawer->think(render_context.delta_time);
				drawer->update_texture(command_list, render_context.delta_time, vr_context);
			}


			for (int eye = 0; eye < 2; ++eye)
			{

				auto tex = pEyeRenderTexture[eye]->GetD3DColorResource();

				command_list->transition(tex, Render::ResourceState::PRESENT);


			}
			//		command_list->execute_and_wait();



			command_list->end();

			// Initialize our single full screen Fov layer.
			ovrLayerEyeFov ld = {};
			ld.Header.Type = ovrLayerType_EyeFov;
			ld.Header.Flags = 0;

			for (int eye = 0; eye < 2; ++eye)
			{
				ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureChain;
				ld.Viewport[eye] = eyeRenderViewport[eye];
				ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = EyeRenderPose[eye];
				ld.SensorSampleTime = sensorSampleTime;
			}

			command_list->when_send([this, ld](UINT64 res) {

				pEyeRenderTexture[0]->Commit();
				pEyeRenderTexture[1]->Commit();



				const	ovrLayerHeader* layers = &ld.Header;

				ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);



				frameIndex++;

				scheduler::get().enqueue(std::bind(&OVRRender::render, this), std::chrono::steady_clock::now());
				});
			command_list->execute();

		}
		else
			scheduler::get().enqueue(std::bind(&OVRRender::render, this), std::chrono::steady_clock::now());


		Profiler::get().on_frame(frame_counter++);

		if (GetAsyncKeyState('R'))
		{
			//   AssetManager::get().reload_resources();
			Render::pixel_shader::reload_all();
			Render::vertex_shader::reload_all();
			Render::geometry_shader::reload_all();
			Render::hull_shader::reload_all();
			Render::domain_shader::reload_all();
			Render::compute_shader::reload_all();
			Render::Texture::reload_all();
		}
		{
			auto& timer = Profiler::get().start(L"AssetManager");
			AssetManager::get().tact();
		}
		//		DIRECTX.Run(MainLoop);

	}



	OVRRender()
	{


		drawer = std::make_shared<triangle_drawer>();

		GUI::Elements::image::ptr back(new GUI::Elements::image);
		back->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/background2.jpg", false, false));
		back->texture.tiled = true;
		back->width_size = GUI::size_type::MATCH_PARENT;
		back->height_size = GUI::size_type::MATCH_PARENT;
		//	iface->add_child(back);
		ovrMirrorTextureDesc        mirrorDesc = {};

		// Initializes LibOVR, and the Rift
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		if (!OVR_SUCCESS(result))
			return;

		ovrGraphicsLuid luid;
		result = ovr_Create(&session, &luid);
		if (!OVR_SUCCESS(result))
			return;

		hmdDesc = ovr_GetHmdDesc(session);







		for (int eye = 0; eye < 2; ++eye)
		{
			ovrSizei idealSize = ovr_GetFovTextureSize(session, (ovrEyeType)eye, hmdDesc.DefaultEyeFov[eye], 1.0f);
			pEyeRenderTexture[eye] = new OculusEyeTexture();
			if (!pEyeRenderTexture[eye]->Init(session, idealSize.w, idealSize.h))
			{
				//	if (retryCreate) goto Done;
				//	FATALERROR("Failed to create eye texture.");
			}

			eyeRenderViewport[eye].Pos.x = 0;
			eyeRenderViewport[eye].Pos.y = 0;
			eyeRenderViewport[eye].Size = idealSize;
			if (!pEyeRenderTexture[eye]->TextureChain)
			{
				//	if (retryCreate) goto Done;
				//	FATALERROR("Failed to create texture.");
			}

			drawer->size = { idealSize.w, idealSize.h };

		}

		drawer->update_layout({ 0,0,drawer->size.get() }, 1);

		eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);
		/*	// Create a mirror to see on the monitor.
			mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			mirrorDesc.Width = 256;
			mirrorDesc.Height = 256;
			mirrorDesc.MiscFlags = ovrTextureMisc_None;
			result = ovr_CreateMirrorTextureDX(session, DIRECTX.CommandQueue, &mirrorDesc, &mirrorTexture);
			if (!OVR_SUCCESS(result))
			{
				if (retryCreate) goto Done;
				FATALERROR("Failed to create mirror texture.");
			}

			*/





	}
	~OVRRender()
	{

		ovr_Shutdown();
	}
};
#endif



class WindowRender : public UIWindow
{
	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr instance_info;

	std::shared_ptr<triangle_drawer> drawer;

	GUI::base::ptr area;

public:




	WindowRender()
	{
		Profiler::create();
		EVENT("Start WindowRender");

		//	EngineAssets::brdf.get_asset();

		GUI::Elements::image::ptr back(new GUI::Elements::image);
		back->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/back_fill.png", false, false));
		back->texture.tiled = true;
		back->width_size = GUI::size_type::MATCH_PARENT;
		back->height_size = GUI::size_type::MATCH_PARENT;
		add_child(back);
		//  GUI::Elements::check_box::ptr butt(new GUI::Elements::check_box());
		//   butt->docking = GUI::dock::FILL;
		//   ui->add_child(butt);

		on_resize(get_size());
		EVENT("Start UI");
		{
			area.reset(new GUI::base());
			area->docking = GUI::dock::FILL;
			add_child(area);
			GUI::Elements::dock_base::ptr  d = std::make_shared<GUI::Elements::dock_base>();
			d->docking = GUI::dock::FILL;
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;
				//	area->add_child(drawer);
				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
			{
				GUI::Elements::list_box::ptr l(new GUI::Elements::list_box());
				auto& dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());
				//			dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
							//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
			//                    t->init(drawer->scene.get());
			//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);


				}
			}
			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help");// ->get_menu();
					file->add_item("New");
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								try
								{
									auto f = FileSystem::get().get_file("scene.dat")->load_all();

									Scene::ptr scene(new Scene());

									Serializer::deserialize(f, *scene);
									//	drawer->scene = scene;
								}
								catch (std::exception e)
								{
									message_box("error", "cant open", [](bool) {});
								}


							});
					};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						//	auto data = Serializer::serialize(*drawer->scene);
						//	FileSystem::get().save_data(L"scene.dat", data);
					};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						on_destroy();
					};
					auto add = edit->add_item("Add smth")->get_menu();
					add->add_item("Mesh");
					add->add_item("Material");
					add->add_item("Sound");
					//->get_menu()->add_item("12454");
					add_child(menu);
				}
				{
					GUI::Elements::status_bar::ptr bar(new GUI::Elements::status_bar());
					label_fps = GUI::Elements::label::ptr(new GUI::Elements::label());
					instance_info = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_tiles = GUI::Elements::label::ptr(new GUI::Elements::label());

					bar->add_child(label_fps);
					bar->add_child(label_tiles);

					bar->add_child(instance_info);
					instance_info->docking = GUI::dock::RIGHT;
					label_tiles->margin = { 20,0,0,0 };
					add_child(bar);
				}

				auto dock = d->get_dock(GUI::dock::LEFT);
				auto m = GUI::Elements::FlowGraph::manager::get().get_ptr<GUI::Elements::FlowGraph::manager>();
				dock->get_tabs()->add_button(m->create_parameter_view());
				add_child(m);
				dock->size = { 400, 400 };
				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };
					EVENT("End Asset Explorer");
				}


				{
					GUI::Elements::window::ptr wnd(new GUI::Elements::window);
					add_child(wnd);
					GUI::Elements::dock_base::ptr dock(new GUI::Elements::dock_base);
					wnd->add_child(dock);
					//					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(drawer->render_graph));
					wnd->pos = { 200, 200 };
					wnd->size = { 300, 300 };
				}

			}
		}

	}

	UINT64 frame_counter = 0;

	void draw_ui(Render::context& c)
	{
		{
			auto timer = c.command_list->start(L"read timings");
			auto ptr = get_ptr();
			Render::GPUTimeManager::get().read_buffer(c.command_list, [ptr, this]() {
				run_on_ui([]() {	Profiler::get().update(); });

				});
		}

		//	user_interface::draw_ui(c);

	}
	virtual void render() override
	{
		Profiler::get().on_frame(frame_counter++);

		if (GetAsyncKeyState('R'))
		{
			//   AssetManager::get().reload_resources();
			Render::pixel_shader::reload_all();
			Render::vertex_shader::reload_all();
			Render::geometry_shader::reload_all();
			Render::hull_shader::reload_all();
			Render::domain_shader::reload_all();
			Render::compute_shader::reload_all();
			Render::Texture::reload_all();
		}
		{
			auto& timer = Profiler::get().start(L"AssetManager");
			AssetManager::get().tact();
		}


		if (fps.tick())
		{
			label_fps->text = std::to_string(fps.get()) + " " + std::to_string(Device::get().get_vram());

		}

		UIWindow::render();

	}
	void on_destroy() override
	{
		Application::get().shutdown();
	}

};



class FrameGraphRender : public Window, public GUI::user_interface
{
	Render::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	std::future<void> task_future;

	FrameGraph graph;

	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr instance_info;

	std::shared_ptr<triangle_drawer> drawer;

	GUI::base::ptr area;


	size_t graph_usage = 0;

public:
	void on_destroy() override
	{
		Application::get().shutdown();
	}

	virtual	void render()
	{
		std::lock_guard<std::mutex> g(m);

		if (GetAsyncKeyState('R'))
		{
			//   AssetManager::get().reload_resources();
			Render::pixel_shader::reload_all();
			Render::vertex_shader::reload_all();
			Render::geometry_shader::reload_all();
			Render::hull_shader::reload_all();
			Render::domain_shader::reload_all();
			Render::compute_shader::reload_all();
			Render::Texture::reload_all();
		}

		Profiler::get().on_frame(frame_counter++);

		GUI::user_interface::size = new_size;
		if (fps.tick())
		{
			size_t total = 0;

			total += DescriptorHeapManager::get().cpu_dsv->used_size();
			total += DescriptorHeapManager::get().cpu_srv->used_size();
			total += DescriptorHeapManager::get().cpu_rtv->used_size();
			total += DescriptorHeapManager::get().cpu_smp->used_size();


			size_t total_gpu = 0;

			total_gpu += DescriptorHeapManager::get().gpu_srv->used_size();

			total_gpu += DescriptorHeapManager::get().gpu_smp->used_size();


			label_fps->text = std::to_string(fps.get()) + " " + std::to_string(Device::get().get_vram()) + " " + std::to_string(total) + " " + std::to_string(total_gpu) + " " + std::to_string(graph_usage);
		}


		process_ui(main_timer.tick());
		{
			auto& timer = Profiler::get().start(L"Wait next");
			swap_chain->start_next();
		}
		setup_graph();
		graph.render();

		{

			auto& timer = Profiler::get().start(L"reset");

			graph.reset();
		}
		swap_chain->present(Render::Device::get().get_queue(Render::CommandListType::DIRECT)->signal());





		if (Application::get().is_alive())
		{
			auto ptr = get_ptr();
			task_future = scheduler::get().enqueue([ptr, this]() {

				render();
				}, std::chrono::steady_clock::now());
		}
	}



	void setup_graph()
	{


		struct pass_data
		{
			ResourceHandler* o_texture;
		};


		struct pass_no
		{

		};

		graph.start_new_frame();

		graph.builder.pass_texture("swapchain", swap_chain->get_current_frame());


		create_graph(graph);


		auto ptr = get_ptr();

		graph.add_pass<pass_data>("PROFILER", [](pass_data& data, TaskBuilder& builder) {
			data.o_texture = builder.need_texture("swapchain");
			}, [this, ptr](pass_data& data, FrameContext& context) {

				context.get_list()->transition(context.get_texture(data.o_texture).resource, ResourceState::PRESENT);

				Render::GPUTimeManager::get().read_buffer(context.get_list(), [ptr, this]() {
					run_on_ui([this, ptr]() {	Profiler::get().update(); });

					});

			});


		graph.setup();
		graph.compile(swap_chain->m_frameIndex);

		graph_usage = 0;
		graph_usage += graph.builder.allocator.heap_rtv.get_max_usage();
		graph_usage += graph.builder.allocator.heap_uav.get_max_usage();
		graph_usage += graph.builder.allocator.heap_srv.get_max_usage();

		for (auto& e : graph.builder.allocator.frames)
		{
			graph_usage += e.heap_readback.get_max_usage();
			graph_usage += e.heap_upload_buffer.get_max_usage();
			graph_usage += e.heap_upload_texture.get_max_usage();

		}
		graph_usage /= (1024 * 1024);
	}

	FrameGraphRender()
	{
		Window::input_handler = this;
		DX12::swap_chain_desc desc;
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.fullscreen = false;
		desc.stereo = false;
		desc.window = this;
		swap_chain = Render::Device::get().create_swap_chain(desc);
		/*
		auto texture = Render::Texture::get_resource(Render::texure_header("textures/gui/back_fill.png", false, false));;


		DescriptorHeap::ptr heap = std::make_shared<DescriptorHeap>(32768,DescriptorHeapType::CBV_SRV_UAV);
		DescriptorHeap::ptr gpu_heap = std::make_shared<DescriptorHeap>(32768, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE);
		DescriptorHeap::ptr cpu_heap = std::make_shared<DescriptorHeap>(32768, DescriptorHeapType::CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = get_default_mapping(texture->get_desc().Format);
		srvDesc.Format = to_srv(texture->get_desc().Format);


		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texture->get_desc().MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;


		auto res = texture->get_native().Get();



		auto device = Device::get().get_native_device();
		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view = heap->get_table_view(0, 32768);



			for (int i = 0; i < 32768; i++)
			{
				//texture->texture_2d()->place_srv(heap_view[i]);

		//		auto h = heap_view[i];
				device->CreateShaderResourceView(res, &srvDesc, heap->get_handle(i));


			}
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();

			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);

			Log::get() << "CreateShaderResourceView CPU: " <<  elapsed_seconds.count() / 32768 << "ns"<<Log::endl;
		}

		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view = gpu_heap->get_table_view(0, 32768);
			for (int i = 0; i < 32768; i++)
			{
			//	texture->texture_2d()->place_srv(heap_view[i]);

			//	auto h = heap_view[i];
				device->CreateShaderResourceView(res, &srvDesc, gpu_heap->get_handle(i));

			}
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();

			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);
			Log::get() << "CreateShaderResourceView GPU: "  << elapsed_seconds.count() / 32768 << "ns" << Log::endl;
		}


		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view1 = heap->get_table_view(0, 32768);

			auto heap_view2 = gpu_heap->get_table_view(0, 32768);
			for (int i = 0; i < 32768; i++)
			{
				device->CopyDescriptorsSimple(1, heap_view2[i].cpu, heap_view1[i].cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();

			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);
			Log::get() << "CopyDescriptorsSimple from CPU to GPU 1 by 1: "  << elapsed_seconds.count() / 32768 << "ns" << Log::endl;
		}

		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view1 = heap->get_table_view(0, 32768);

			auto heap_view2 = cpu_heap->get_table_view(0, 32768);
			for (int i = 0; i < 32768; i++)
			{
				device->CopyDescriptorsSimple(1, heap_view2[i].cpu, heap_view1[i].cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();

			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);
			Log::get() << "CopyDescriptorsSimple from CPU to CPU 1 by 1: " << elapsed_seconds.count() / 32768 << "ns" << Log::endl;
		}



		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view1 = heap->get_table_view(0, 32768);

			auto heap_view2 = gpu_heap->get_table_view(0, 32768);
			device->CopyDescriptorsSimple(32768, heap_view2[0].cpu, heap_view1[0].cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();
			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);
			Log::get() << "CopyDescriptorsSimple from all CPU to GPU: " << elapsed_seconds.count() /32768<< "ns" << Log::endl;
		}

		{
			std::chrono::steady_clock::time_point start_tick = std::chrono::steady_clock::now();
			auto heap_view1 = heap->get_table_view(0, 32768);

			auto heap_view2 = cpu_heap->get_table_view(0, 32768);
			device->CopyDescriptorsSimple(32768, heap_view2[0].cpu, heap_view1[0].cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();
			auto elapsed_seconds = duration_cast<std::chrono::nanoseconds>(last_tick - start_tick);
			Log::get() << "CopyDescriptorsSimple from all CPU to CPU: " << elapsed_seconds.count() / 32768 << "ns" << Log::endl;


		}*/
		/*	auto shader = Render::pixel_shader::get_resource({ "shaders\\autogen_test.hlsl", "PS", 0, {} });
			Slots::Bobo bobo;
			Slots::Bobo2 bobo2;
			Slots::Bobo3 bobo3;

			bobo.GetB() = 5;
			bobo.GetRw();

			get_Signature(Layouts::FrameLayout);
		*/
		set_capture = [this](bool v)
		{
			if (v)
				SetCapture(get_hwnd());
			else
				ReleaseCapture();
		};



		//	EngineAssets::brdf.get_asset();


			//  GUI::Elements::check_box::ptr butt(new GUI::Elements::check_box());
			//   butt->docking = GUI::dock::FILL;
			//   ui->add_child(butt);

		on_resize(get_size());

		{
			GUI::Elements::image::ptr back(new GUI::Elements::image);
			back->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/back_fill.png", false, false));
			back->texture.tiled = true;
			back->width_size = GUI::size_type::MATCH_PARENT;
			back->height_size = GUI::size_type::MATCH_PARENT;
			add_child(back);
		}

		EVENT("Start UI");
		// 		if (false)
		{
			area.reset(new GUI::base());
			area->docking = GUI::dock::FILL;
			add_child(area);
			GUI::Elements::dock_base::ptr  d = std::make_shared<GUI::Elements::dock_base>();
			d->docking = GUI::dock::FILL;
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;

				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
			{
				GUI::Elements::list_box::ptr l(new GUI::Elements::list_box());
				auto& dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());
				//			dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
							//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
			//                    t->init(drawer->scene.get());
			//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);


				}
			}

			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help");// ->get_menu();
					file->add_item("New");
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								try
								{
									auto f = FileSystem::get().get_file("scene.dat")->load_all();

									Scene::ptr scene(new Scene());

									Serializer::deserialize(f, *scene);
									//	drawer->scene = scene;
								}
								catch (std::exception e)
								{
									message_box("error", "cant open", [](bool) {});
								}


							});
					};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						//	auto data = Serializer::serialize(*drawer->scene);
						//	FileSystem::get().save_data(L"scene.dat", data);
					};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						on_destroy();
					};
					auto add = edit->add_item("Add smth")->get_menu();
					add->add_item("Mesh");
					add->add_item("Material");
					add->add_item("Sound");
					//->get_menu()->add_item("12454");
					add_child(menu);
				}
				{
					GUI::Elements::status_bar::ptr bar(new GUI::Elements::status_bar());
					label_fps = GUI::Elements::label::ptr(new GUI::Elements::label());
					instance_info = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_tiles = GUI::Elements::label::ptr(new GUI::Elements::label());

					bar->add_child(label_fps);
					bar->add_child(label_tiles);

					bar->add_child(instance_info);
					instance_info->docking = GUI::dock::RIGHT;
					label_tiles->margin = { 20,0,0,0 };
					add_child(bar);
				}

				auto dock = d->get_dock(GUI::dock::LEFT);
				//auto m = GUI::Elements::FlowGraph::manager::get().get_ptr<GUI::Elements::FlowGraph::manager>();
				//dock->get_tabs()->add_button(m->create_parameter_view());
			//	add_child(m);
				dock->size = { 400, 400 };
				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };
					EVENT("End Asset Explorer");
				}


				{
					GUI::Elements::window::ptr wnd(new GUI::Elements::window);
					add_child(wnd);
					GUI::Elements::dock_base::ptr dock(new GUI::Elements::dock_base);
					wnd->add_child(dock);
					MaterialGraph::ptr graph(new MaterialGraph);


					{

						auto value_node = std::make_shared<VectorNode>(vec4(1, 0, 0, 1));
						graph->register_node(value_node);
						value_node->get_output(0)->link(graph->get_base_color());
					}


					{
						auto value_node = std::make_shared<ScalarNode>(0.4f);
						graph->register_node(value_node);
						value_node->get_output(0)->link(graph->get_roughness());
					}

					{
						auto value_node = std::make_shared<ScalarNode>(1.0f);
						graph->register_node(value_node);
						value_node->get_output(0)->link(graph->get_mettalic());

					}
					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(graph));
					wnd->pos = { 200, 200 };
					wnd->size = { 300, 300 };
				}

			}
		}
	}

	virtual ~FrameGraphRender()
	{

		if (task_future.valid())
			task_future.wait();
	}
	void on_resize(vec2 size) override
	{
		new_size = size;
	}


	virtual void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
		if (swap_chain)	swap_chain->resize(r);
	}
};

class RenderApplication : public Application
{

	friend class Singleton<Application>;


	std::shared_ptr<FrameGraphRender> main_window;
	//std::shared_ptr<WindowRender> main_window;
#ifdef OCULUS_SUPPORT
	std::shared_ptr<OVRRender> ovr;
#endif
	// TODO:: shared_objects
	std::atomic<bool> need_reload_resource;
	std::atomic<bool> need_print_screen;

protected:
	RenderApplication()
	{
	
	//	assert(ppp.inited);

		EVENT("Device");
		Render::Device::create();
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());
		EVENT("AssetManager");
		AssetManager::create();
		EVENT("WindowRender");

#ifdef OCULUS_SUPPORT
		//ovr = std::make_shared<OVRRender>();
#endif
		init_signatures();
//		init_pso();



	//	get_PSO(PSO::Lighting);
		
	//	auto ppp = GetPSO<Lighting>(Lighting::SecondBounce | Lighting::Count(3) | Lighting::Smth(222));


		//	main_window = std::make_shared<WindowRender>();
		main_window = std::make_shared<FrameGraphRender>();


		create_task([this]() {

			if (main_window)main_window->render();

#ifdef OCULUS_SUPPORT
			if (ovr)ovr->render();
#endif

			});
		EVENT("good");
	}

	virtual ~RenderApplication()
	{
		shutdown();// really need this?

		main_window = nullptr;

		std::this_thread::sleep_for(100ms);
		scheduler::reset();
		//


	//	
		//   Render::Device::get().get_queue(Render::CommandListType::DIRECT)->stop_all();
		Render::Device::get().stop_all();
		Skin::reset();
		Render::Texture::reset_manager();
		Render::pixel_shader::reset_manager();
		Render::vertex_shader::reset_manager();
		Render::domain_shader::reset_manager();
		Render::hull_shader::reset_manager();
		Render::geometry_shader::reset_manager();
		Render::compute_shader::reset_manager();
		GUI::Elements::FlowGraph::manager::reset();
		Profiler::reset();
		Render::GPUTimeManager::reset();
		///    main_window2 = nullptr;
		Fonts::FontSystem::reset();
		AssetRenderer::reset();
		Render::BufferCache::reset();
		TextureAssetRenderer::reset();
		AssetManager::reset();




		Render::Device::reset();
		//   Render::Device::reset();



	}


	virtual void on_tick() override
	{
		if (GetAsyncKeyState(VK_ESCAPE))
			shutdown();

		Window::process_messages();
	}

};

void SetupDebug()
{
	Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	//  ConsoleLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
	// Here we can disable some of notification types
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<singleton_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<Render::Resource>::get().set_logging_level(Log::LEVEL_ALL);
	Log::get() << Log::LEVEL_INFO << "info text" << Log::endl;
	Log::get() << Log::LEVEL_WARNING << "warning text" << Log::endl;
	Log::get() << Log::LEVEL_DEBUG << "debug text" << Log::endl;
	Log::get() << Log::LEVEL_ERROR << "error text" << Log::endl;
}


#include <DbgHelp.h>
int APIENTRY WinMain(_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
{
	setlocale(LC_ALL, "");
	CoInitialize(NULL);
	static	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);



	//entry->SetCompressionStream();
	//	archive->WriteToStream();
//    FlowGraph::FlowSystem::get().register_node<FlowGraph::graph>("empty graph");
//   FlowGraph::FlowSystem::get().register_node<test_node>("test_node");
	// FlowGraph::FlowSystem::get().register_node<start_test_node>("start_test_node");
	// FlowGraph::FlowSystem::get().register_node<test_graph>("ololol");
//   FlowGraph::FlowSystem::get().register_node<TextureNode>("TextureNode");
	FlowGraph::FlowSystem::get().register_node<PowerNode>("PowerNode");
	FlowGraph::FlowSystem::get().register_node<SumNode>("SumNode");
	FlowGraph::FlowSystem::get().register_node<MulNode>("MulNode");
	FlowGraph::FlowSystem::get().register_node<SpecToMetNode>("SpecToMetNode");


	FlowGraph::FlowSystem::get().register_node("Scalar", []()->ScalarNode::ptr
		{
			auto res = std::make_shared<ScalarNode>(1.0f);
			res->name = "Scalar";
			return res;
		});



	FlowGraph::FlowSystem::get().register_node("ZeroColor", []()->VectorNode::ptr
		{
			auto res = std::make_shared<VectorNode>(float4{ 0,0,0,0 });
			res->name = "ZeroColor";
			return res;
		});



	//	FlowGraph::FlowSystem::get().register_node<ResultNode>("Material");
	//	FlowGraph::FlowSystem::get().register_node<MaterialGraph>("MaterialGraph");
	FlowGraph::FlowSystem::get().register_node<MaterialFunction>("MaterialFunction");
	FlowGraph::FlowSystem::get().register_node("file", []()-> FlowGraph::graph::ptr
		{
			auto f = FileSystem::get().get_file("graph.flg");

			if (f)
				return Serializer::deserialize<FlowGraph::graph>(f->load_all());

			return nullptr;
		});


	auto result_code = 0;
	SetupDebug();
	EVENT("start");
	Application::create<RenderApplication>();
	EVENT("create");


	// There can be error while creating, so test
	if (Application::is_good())
	{
		EVENT("run");
		Application::get().run(); // TODO: Create task and run engine (scheduling tasks)
		Application::reset();
	}

	else
		result_code = S_FALSE;

	CounterManager::get().print();
	EVENT("end");
	CoUninitialize();
	SymCleanup(process);
	return result_code;
}

inline void Texture3DTiled::remove_tile(ivec3 pos, int level)
{
	remove_tile(get_tile(level, pos));
}