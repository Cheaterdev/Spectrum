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


class GraphDebugRender : public GUI::Elements::FlowGraph::canvas
{
public:

	void generate(FrameGraph& graph)
	{

	}

};

class triangle_drawer : public GUI::Elements::image, public FrameGraphGenerator, VariableContext
{
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;
	stencil_renderer::ptr stenciler;

	GUI::Elements::label::ptr info;

	size_t time = 0;
	struct EyeData : public Events::prop_handler
	{

		using ptr = std::shared_ptr<EyeData>;
		first_person_camera cam;



		EyeData(Render::RootSignature::ptr sig)
		{

		}
	};

	std::vector<EyeData::ptr> eyes;
	first_person_camera cam;
public:
	using ptr = std::shared_ptr<triangle_drawer>;
	//	PostProcessGraph::ptr render_graph;


	Variable<bool> enable_gi = { true, "enable_gi", this };
	//Variable<bool> debug_draw = Variable<bool>(false, "debug_draw",this);
	//	VoxelGI::ptr voxel_renderer;

	int visible_count;

	mesh_renderer::ptr meshes_renderer;

	//	gpu_cached_renderer::ptr gpu_meshes_renderer_static;
	//	gpu_cached_renderer::ptr gpu_meshes_renderer_dynamic;

	Scene::ptr scene;
	Render::QueryHeap::ptr query_heap;
	float draw_time;
	MeshAssetInstance::ptr instance;




	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	PSSM pssm;
	SMAA smaa;
	SkyRender sky;
	VoxelGI::ptr voxel_gi;
	std::string debug_view;
	triangle_drawer() :VariableContext(L"triangle_drawer")
	{
		texture.mul_color = { 1,1,1,0 };
		texture.add_color = { 0,0,0,1 };

		texture.srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);

	

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
		base::add_child(stenciler);

		info.reset(new GUI::Elements::label);
		info->docking = GUI::dock::TOP;
		info->x_type = GUI::pos_x_type::LEFT;
		info->magnet_text = FW1_LEFT;
		base::add_child(info);
		base::ptr props(new base);
		props->docking = GUI::dock::FILL;
		props->x_type = GUI::pos_x_type::LEFT;
		props->y_type = GUI::pos_y_type::TOP;
		props->width_size = GUI::size_type::MATCH_CHILDREN;
		props->height_size = GUI::size_type::MATCH_CHILDREN;
		base::add_child(props);

		{
			auto combo = std::make_shared<GUI::Elements::combo_box>();

			combo->size = { 200, 25 };
			combo->on_click = [this](GUI::Elements::button::ptr butt) {

				GUI::Elements::combo_box::ptr combo = butt->get_ptr<GUI::Elements::combo_box>();
				combo->remove_items();


				combo->add_item("Normal")->on_click = [this](GUI::Elements::menu_list_element::ptr) {debug_view = ""; };


				for (auto& e : last_graph->builder.alloc_resources)
				{

					if (e.second.type != ::ResourceType::Texture) continue;

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


		base::add_child(circle);
		//	lighting = std::make_shared<LightingNode>();

		circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
					float3 dir = { 0.001 + v.x,sqrt(1.001 - v.length_squared()),-v.y };
					pssm.set_position(dir);
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

		int count = -1;
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

		ivec2 size = ivec2::max(ivec2(get_render_bounds().size),ivec2(64,64));
		struct pass_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* sky_cubemap;
			GBufferViewDesc gbuffer;

		};

		scene->update(*graph.builder.current_frame);
		graph.frame_size = size;
		graph.scene = scene.get();
		graph.renderer = gpu_scene_renderer.get();
		graph.cam = &cam;
		graph.time = (float)my_timer.tick();
		graph.totalTime += graph.time;
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

				bool need_rebuild = false;// scene->init_ras(command_list);
				SceneFrameManager::get().prepare(command_list, *scene);
		
				//if (GetAsyncKeyState('O'))
				
				if (Device::get().is_rtx_supported())
				{
				//	command_list->create_transition_point();
			//		command_list->transition(scene->raytrace->buffer, ResourceState::NON_PIXEL_SHADER_RESOURCE);

					scene->raytrace_scene->update(command_list, (UINT)scene->raytrace->max_size(), scene->raytrace->buffer->get_resource_address(), need_rebuild);
					RTX::get().prepare(command_list);
				}

			}

			command_list->end();

			command_list->execute();
		}


		{
			struct GBufferData
			{
				GBufferViewDesc gbuffer;

				Handlers::Texture H(GBuffer_HiZ);
				Handlers::Texture H(GBuffer_HiZ_UAV);


			};

			graph.add_pass<GBufferData>("GBUFFER", [this, size](GBufferData& data, TaskBuilder& builder) {
				data.gbuffer.create(size, builder);
				data.gbuffer.create_mips(size, builder);
				data.gbuffer.create_quality(size, builder);

				builder.create(data.GBuffer_HiZ, { ivec3(size / 8, 1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
				builder.create(data.GBuffer_HiZ_UAV, { ivec3(size / 8, 1), DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,1 }, ResourceFlags::UnorderedAccess);

				}, [this,&graph](GBufferData& data, FrameContext& _context) {

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

					command_list->get_graphics().set_layout(Layouts::DefaultLayout);
					command_list->get_compute().set_layout(Layouts::DefaultLayout);


					//				gpu_meshes_renderer_static->update(context);
						//			gpu_meshes_renderer_dynamic->update(context);

					GBuffer gbuffer = data.gbuffer.actualize(_context);

					gbuffer.rtv_table = RenderTargetTable(context->list->get_graphics(), { gbuffer.albedo, gbuffer.normals, gbuffer.specular, gbuffer.speed }, gbuffer.depth);

					gbuffer.HalfBuffer.hiZ_depth = *(data.GBuffer_HiZ);
					gbuffer.HalfBuffer.hiZ_table = RenderTargetTable(context->list->get_graphics(), {  }, gbuffer.HalfBuffer.hiZ_depth);
					gbuffer.HalfBuffer.hiZ_depth_uav = *(data.GBuffer_HiZ_UAV);

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

					graph.set_slot(SlotID::FrameInfo, command_list->get_graphics());
					graph.set_slot(SlotID::FrameInfo, command_list->get_compute());

					gpu_scene_renderer->render(context, scene);

					//	stenciler->render(context, scene);

					command_list->get_copy().copy_texture(gbuffer.depth_prev_mips.resource, 0, gbuffer.depth_mips.resource, 0);
					command_list->get_copy().copy_texture(gbuffer.depth_mips.resource, 0, gbuffer.depth.resource, 0);

					//	
				});
		}



		struct no
		{
			Handlers::Texture H(ResultTexture);

		};
		graph.add_pass<no>("no", [this, &graph](no& data, TaskBuilder& builder) {
			builder.create(data.ResultTexture, { graph.frame_size, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 1 });
			}, [](no& data, FrameContext& _context) {});



		pssm.generate(graph);
		sky.generate(graph);

		// remove on intel
		if(enable_gi) voxel_gi->generate(graph);

		
		sky.generate_sky(graph);

		stenciler->generate_after(graph);

		smaa.generate(graph);

		struct debug_data
		{
			Handlers::Texture H(swapchain);
			
			Handlers::Texture debug_tex;
		};

		//

		std::string res_tex = "ResultTexture";
		if (!debug_view.empty())
			res_tex = debug_view;
		auto promise = std::make_shared<std::promise<Render::Handle>>();
		result_tex = promise->get_future();

		graph.add_pass<debug_data>("DEBUG", [this, res_tex](debug_data& data, TaskBuilder& builder) {

			data.debug_tex = Handlers::Texture(res_tex);
			 builder.need(data.swapchain, ResourceFlags::RenderTarget);
			builder.need(data.debug_tex, ResourceFlags::PixelRead);

			}, [this, promise](debug_data& data, FrameContext& context) {

				auto output_tex =(*data.swapchain);
				auto debug_tex =(*data.debug_tex);

				if (!debug_tex) return;

				promise->set_value(debug_tex.texture2D);
				/*
				auto& list = context.get_list();

				list->get_graphics().set_viewport(vec4{0,0,100,100});
				list->get_graphics().set_scissor({ 0,0,100,100 });

				MipMapGenerator::get().copy_texture_2d_slow(list->get_graphics(), texture.texture, debug_tex);*/
			}, PassFlags::Required);

		graph.add_slot_generator([this](FrameGraph& graph) {

				PROFILE(L"FrameInfo");
				Slots::FrameInfo frameInfo;
				//// hack zone
				auto &sky = graph.builder.alloc_resources["sky_cubemap_filtered"];
				if (sky.resource)
					frameInfo.GetSky() = sky.get_handler<Handlers::Texture>()->textureÑube;
				/////////
				frameInfo.GetSunDir() = graph.sunDir;
				frameInfo.GetTime() = { graph.time ,graph.totalTime,0,0 };


				frameInfo.MapCamera().cb = graph.cam->camera_cb.current;
				frameInfo.MapPrevCamera().cb = graph.cam->camera_cb.prev;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
				frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d()->texture2D;

				auto compiled = frameInfo.compile(*graph.builder.current_frame);
				graph.register_slot_setter(compiled);
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
		cam.set_projection_params(Math::pi / 4, float(r.w) / r.h, 1, 1500);
		//	for (auto& e : eyes)
	//			e->g_buffer.size = { r.w,r.h };
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
			PROFILE(L"AssetManager");
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


class FrameFlowGraph : public  ::FlowGraph::graph
{

};

class PassNode : public::FlowGraph::Node , public  GUI::Elements::FlowGraph::VisualGraph
{

	virtual  void operator()(::FlowGraph::GraphContext*)
	{}
	GUI::base::ptr create_editor_window() override
	{

		//if (!debug_texture)
	//		debug_texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 128, 128, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		GUI::Elements::image::ptr img(new GUI::Elements::image);
		img->texture.texture = Render::Texture::get_resource({ "textures/gui/shadow.png", false, false });
		img->texture.padding = { 9, 9, 9, 9 };
		img->padding = { 9, 9, 9, 9 };
		img->width_size = GUI::size_type::MATCH_CHILDREN;
		img->height_size = GUI::size_type::MATCH_CHILDREN;
		//   img->size = { 64, 64 };
//		img_inner.reset(new GUI::Elements::image);


		//img_inner->texture.texture = debug_texture;
	//	img_inner->docking = GUI::dock::TOP;
	//	img_inner->size = { 128, 128 };
	//	img->add_child(img_inner);
		return img;
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
	std::shared_ptr<FrameFlowGraph> frameFlowGraph;
	GUI::base::ptr area;


	size_t graph_usage = 0;

public:
	void on_destroy() override
	{
		Application::get().shutdown();
	}

	virtual	void render()
	{
		if (swap_chain)	swap_chain->resize(new_size);

	
		{
			std::lock_guard<std::mutex> g(m);

			if (GetAsyncKeyState('R'))
			{
				Render::Device::get().get_queue(Render::CommandListType::DIRECT)->signal_and_wait();
				Render::Device::get().get_queue(Render::CommandListType::COMPUTE)->signal_and_wait();
				Render::Device::get().get_queue(Render::CommandListType::COPY)->signal_and_wait();

				//   AssetManager::get().reload_resources();
				Render::pixel_shader::reload_all();
				Render::vertex_shader::reload_all();
				Render::geometry_shader::reload_all();
				Render::hull_shader::reload_all();
				Render::domain_shader::reload_all();
				Render::compute_shader::reload_all();
				Render::library_shader::reload_all();
				Render::Texture::reload_all();
			}

			Profiler::get().on_frame(frame_counter++);

			GUI::user_interface::size = new_size;
			if (fps.tick())
			{
				size_t total = 0;
				for (auto& h : DescriptorHeapManager::get().cpu_heaps)
				{
					if (h)
					{
						total += h->used_size();
					}
				}
		

				size_t total_gpu = 0;		
				for (auto& h : DescriptorHeapManager::get().gpu_heaps)
				{
					if (h)
					{
						total_gpu += h->used_size();
					}
				}

				label_fps->text = std::to_string(fps.get()) + " " + std::to_string(Device::get().get_vram()) + " " + std::to_string(total) + " " + std::to_string(total_gpu) + " " + std::to_string(graph_usage);
			}


			process_ui((float)main_timer.tick());
			{
				PROFILE(L"Wait next");
				swap_chain->start_next();
			}
			setup_graph();
			graph.render();
	

			{

				PROFILE(L"reset");

				graph.reset();
			}
			swap_chain->present(Render::Device::get().get_queue(Render::CommandListType::DIRECT)->signal());
		}

	

		if (Application::get().is_alive())
		{
			auto ptr = get_ptr();
			task_future = scheduler::get().enqueue([ptr, this]() {

				render();
				std::this_thread::yield();
				}, std::chrono::steady_clock::now());
		}
	}



	void setup_graph()
	{


		struct pass_data
		{
			Handlers::Texture H(swapchain);

		};


		struct pass_no
		{

		};

		graph.start_new_frame();
		graph.builder.pass_texture("swapchain", swap_chain->get_current_frame());


		{

			PROFILE(L"create_graph");
			create_graph(graph);


			auto ptr = get_ptr();
		//	if(false)
			graph.add_pass<pass_data>("PROFILER", [](pass_data& data, TaskBuilder& builder) {
				builder.need(data.swapchain, ResourceFlags::Required);
				}, [this, ptr](pass_data& data, FrameContext& context) {

					context.get_list()->transition_present(data.swapchain->resource.get());

					Render::GPUTimeManager::get().read_buffer(context.get_list(), [ptr, this]() {
						run_on_ui([this, ptr]() {	Profiler::get().update(); });

						});

				}, PassFlags::Required);

		}
	
		graph.setup();
		graph.compile(swap_chain->m_frameIndex);


		static bool gen = false;

		if (!gen&&GetAsyncKeyState('N'))
		{
			gen = true;
			frameFlowGraph->clear();

			struct res_stage
			{

			};
			std::map<ResourceAllocInfo*, ::FlowGraph::parameter::ptr> resource_stages;


			for (auto &res : graph.builder.alloc_resources)
			{

				if (res.second.passed)
				{
auto input = 				 	frameFlowGraph->register_input(res.second.name);
resource_stages[&res.second] = input;
				}
			}
			for (auto pass : graph.passes)
			{
				auto node = std::make_shared<PassNode>();
				node->name = pass->name + " " + std::to_string(pass->id);

				
				if (!pass->enabled)
				{
					node->color = float4(1, 0, 0, 1);
				}

				if (check(pass->flags&PassFlags::Required))
				{
					node->color = float4(1, 1, 0, 1);
				}

				frameFlowGraph->register_node(node);

				for (auto& info : pass->used.resources)
				{
					if (pass->used.resource_creations.count(info) == 0) {
						auto input = node->register_input(info->name);
						auto prev = resource_stages[info];

						if (prev)
						{


							prev->link(input);
						}
					auto output = 	node->register_output(info->name);

					resource_stages[info] = output;
					}
				}


				for (auto& info : pass->used.resources)
				{

					if (pass->used.resource_creations.count(info))
					{
						auto output = node->register_output(info->name);
						resource_stages[info] = output;
					}
				}
			}


			for (auto& res : graph.builder.alloc_resources)
			{

				if (res.second.passed && check(res.second.flags & ResourceFlags::Required))
				{
					auto input = frameFlowGraph->register_output(res.second.name);
					auto prev = resource_stages[&res.second];

					if (prev)
					{


						prev->link(input);
					}
				}
			}



		}

		/*
	
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
		graph_usage /= (1024 * 1024);*/
	}

	FrameGraphRender()
	{
		Window::input_handler = this;
		DX12::swap_chain_desc desc;
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.fullscreen = nullptr;
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
				auto dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());

				{

					auto b = std::make_shared<GUI::base>();

					
					auto folders = std::make_shared<GUI::Elements::tree<VariableContext>>();
					folders->size = { 200, 150 };
					folders->docking = GUI::dock::LEFT;

					auto table = std::make_shared<GUI::base>();
					table->docking = GUI::dock::FILL;

					b->add_child(folders);
					b->add_child(table);
					
					folders->on_select = [this, table](VariableContext* elem)
					{
				
							table->remove_all();

							for(auto v:elem->variables)
							{
								auto property = GUI::Elements::create_property(*v);
								table->add_child(property);
							}
					};
					
					folders->init(&VariableContext::get());

					dock->get_tabs()->add_page("Properties", b);

				}
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



					frameFlowGraph = std::make_shared< FrameFlowGraph>();

					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(frameFlowGraph));

				}
			}

			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help");// ->get_menu();
					file->add_item("New")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								drawer->scene->remove_all();
							});
					};
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								try
								{
									auto f = FileSystem::get().get_file("scene.dat")->load_all();
									Serializer::deserialize(f, *drawer->scene);
								}
								catch (std::exception e)
								{
									message_box("error", "cant open", [](bool) {});
								}


							});
					};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
							auto data = Serializer::serialize(*drawer->scene);
							FileSystem::get().save_data(L"scene.dat", data);
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

			
				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };
					EVENT("End Asset Explorer");
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
		new_size = vec2::max(size, vec2{64,64});
	}


	virtual void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
		
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
		THREAD_SCOPE(GUI);

	//	assert(ppp.inited);
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());

		EVENT("Device");
		Render::Device::create();

		EVENT("PSO");
		init_signatures();
		PSOHolder::create();
		RTX::create();
		
		EVENT("AssetManager");
		AssetManager::create();
		EVENT("WindowRender");

		auto ps = Render::pixel_shader::get_resource({ "test.hlsl", "PS", 0,{}, false });
		auto cs = Render::compute_shader::get_resource({ "test.hlsl", "CS", 0,{}, false });

#ifdef OCULUS_SUPPORT
		//ovr = std::make_shared<OVRRender>();
#endif

		
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
		//Render::BufferCache::reset();
		TextureAssetRenderer::reset();
		AssetManager::reset();


		Render::PipelineLibrary::reset();

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
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_ERROR);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_ERROR);
	ClassLogger<singleton_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<Render::Resource>::get().set_logging_level(Log::LEVEL_ERROR);
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
