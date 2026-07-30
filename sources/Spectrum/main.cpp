import Graphics;
import GUI;
import HAL;


#include "Platform/Window.h"

import ppl;
import Core;
import FrameGraph;
import FrameGraphDebug;

using namespace FrameGraph;

using namespace HAL;



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

class triangle_drawer : public GUI::Elements::image, public GraphGenerator, VariableContext
{
		Pipelines::MainPipeline pipeline;
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;
	stencil_renderer::ptr stenciler;

	GUI::Elements::label::ptr info;

	size_t time = 0;

	struct EyeData : public Events::prop_handler
	{
		using ptr = std::shared_ptr<EyeData>;
		first_person_camera cam;


		EyeData(HAL::RootSignature::ptr sig)
		{}
	};

public:
	first_person_camera cam;

	using ptr = std::shared_ptr<triangle_drawer>;
	//	PostProcessGraph::ptr render_graph;


	//Variable<bool> enable_gi = { true, "GI", this };
	//Variable<bool> enable_fsr = { true, "FSR", this };
	Variable<bool> downsampled = { true, "downsampled", this };
	//Variable<bool> enable_denoiser = { true, "denoiser", this };

	//Variable<bool> debug_draw = Variable<bool>(false, "debug_draw",this);
	//	VoxelGI::ptr voxel_renderer;

	int visible_count;

	mesh_renderer::ptr meshes_renderer;

	Scene::ptr scene;
	float draw_time;
	MeshAssetInstance::ptr instance;

	PSSM pssm;
	SkyRender sky;
	ShadowDenoiser shadow_denoiser;

	BlueNoise blue_noise;
	VoxelGI::ptr voxel_gi;
		SMAA smaa;
		 ~triangle_drawer()
		 {
			 RenderSystem::get().device().get_queue(CommandListType::DIRECT)->signal_and_wait();
			 RenderSystem::get().device().get_queue(CommandListType::COMPUTE)->signal_and_wait();
	 RenderSystem::get().device().get_queue(CommandListType::COPY)->signal_and_wait();

		 }
		triangle_drawer() : VariableContext(L"triangle_drawer"), pipeline(), blue_noise(pipeline), smaa(pipeline), sky(pipeline), pssm(pipeline)
	{
		texture.mul_color = { 1, 1, 1, 0 };
		texture.add_color = { 0, 0, 0, 1 };
		// SMAA used to bake this in (pow(x, 1/2.2)) when it ran; now applied
		// here so the viewport looks the same whether SMAA, FSR, or DLSS
		// produced ResultTextureNew.
		texture.gamma = true;


		auto t = CounterManager::get().start_count<triangle_drawer>();
		thinkable = true;
		clickable = true;

		scene.reset(new Scene());
		scene->name = L"Scene";

		scene_renderer = std::make_shared<main_renderer>();
		scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());


		gpu_scene_renderer = std::make_shared<main_renderer>();

		gpu_scene_renderer->register_renderer(std::make_shared<mesh_renderer>());


		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_static = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::STATIC));
		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_dynamic = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::DYNAMIC));
		//cam.position = vec3(0, 5, -30);

		cam.position = vec3(0, 0, 0);

		stenciler.reset(new stencil_renderer(pipeline));
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



		GUI::Elements::circle_selector::ptr circle(new GUI::Elements::circle_selector);
		circle->docking = GUI::dock::FILL;
		circle->x_type = GUI::pos_x_type::RIGHT;
		circle->y_type = GUI::pos_y_type::TOP;


		base::add_child(circle);
		//	lighting = std::make_shared<LightingNode>();

		circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
				float3 dir = { 0.001 + v.x, sqrt(1.001 - v.length_squared()), -v.y };
				pssm.set_position(dir);
			});

		circle->set_value({ 1, 0 });


		MeshAsset::ptr asset_ptr = EngineAssets::material_tester.get_asset();


		auto make_material = [](float3 color, float roughness, float metallic)
			{
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


				auto m = (new materials::universal_material(graph));
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

				instance->local_transform[3] = {
					i * distance - count * distance / 2, 0, j * distance - count * distance / 2, 1
				};

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
			MeshAsset::ptr ruins_ptr = nullptr; // AssetManager::get().find_by_name<MeshAsset>(L"cubes.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform=mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}
		}


		voxel_gi = std::make_shared<VoxelGI>(pipeline,scene);
	}

	float scale_speed = 0;
	vec2 wheel_pos;

	bool on_wheel(mouse_wheel type, float value, vec2 pos) override
	{
		scale_speed += value * 0.1f;
		wheel_pos = pos;
		return true;
	}

	void think(float dt) override
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

		// Camera only responds to keyboard while the viewport is focused; drop any
		// held movement otherwise so it can't drift after focus is lost.
		if (!stenciler->is_focused())
		{
			cam.move_input = { 0, 0, 0 };
			cam.fast_move  = false;
		}

		cam.frame_move(dt);
	}

	Graph* last_graph = nullptr;
	tick_timer my_timer;
	ResourceAllocInfo* debug_tex_handle = nullptr;

	void generate(Graph& graph) override
	{
		PROFILE(L"triangle_drawer");
		last_graph = &graph;

		ivec2 size = ivec2::max(ivec2(get_render_bounds().size), ivec2(64, 64));
		struct pass_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* sky_cubemap;
			GBufferViewDesc gbuffer;
		};

		scene->update(*graph.builder.current_frame);


		auto& timeinfo = graph.get_context<TimeInfo>();
		auto& skyinfo = graph.get_context<SkyInfo>();
		auto& caminfo = graph.get_context<CameraInfo>();
		auto& sceneinfo = graph.get_context<SceneInfo>();
		auto& vp = graph.get_context<ViewportInfo>();


		if (downsampled)
		{
			// Use DLSS's own recommended render resolution for the current
			// mode when available; get_optimal_settings can return eOk with
			// render_size (0,0) for an unsupported mode/size, so validate
			// before using it. Falls back to /1.5 (the FSR path).
			nvidia::DLSSOptimalSettings settings{};
			if (nvidia::DLSS::get().available() &&
			    nvidia::DLSS::get().get_optimal_settings(g_upscaling_dlss_mode, size, /*hdr=*/true, settings) &&
			    settings.render_size.x > 0 && settings.render_size.y > 0)
				vp.frame_size = settings.render_size;
			else
				vp.frame_size = size / 1.5;
		}
		else
			vp.frame_size = size;

		vp.upscale_size = size;

		sceneinfo.scene = scene;
		sceneinfo.renderer = gpu_scene_renderer;
		caminfo.cam = &cam;
		timeinfo.time = (float)my_timer.tick();
		timeinfo.totalTime += timeinfo.time;
		skyinfo.sunDir = pssm.get_position();

		// Exactly once per frame — a second call anywhere else would desync
		// SL's internal frame counter.
		if (nvidia::Streamline::get().available())
			nvidia::Streamline::get().begin_frame();

		// Jitter only matters when something accumulates it temporally (DLSS);
		// FSR1/native have no such accumulation.
		vec2 jitter_px(0, 0);
		if (nvidia::DLSS::get().available())
		{
			// Halton(2,3); phase count per NVIDIA's guidance: 8*(display/render)^2.
			const float scale_x = float(vp.upscale_size.x) / float(vp.frame_size.x);
			const float scale_y = float(vp.upscale_size.y) / float(vp.frame_size.y);
			const uint32_t phase_count = std::max<uint32_t>(1,
				static_cast<uint32_t>(8.0f * scale_x * scale_y + 0.5f));
			const uint32_t phase = static_cast<uint32_t>(graph.builder.current_frame->get_frame() % phase_count) + 1;

			auto halton = [](uint32_t index, uint32_t base) -> float
				{
					float f = 1.0f, r = 0.0f;
					while (index > 0)
					{
						f /= static_cast<float>(base);
						r += f * (index % base);
						index /= base;
					}
					return r;
				};

			// [-0.5, 0.5] pixels, screen convention: +x = right, +y = down.
			jitter_px = { halton(phase, 2) - 0.5f, halton(phase, 3) - 0.5f };
		}

		// NDC is [-1,1]; Y negated since NDC +y is up, jitter_px +y is down.
		const vec2 jitter_ndc(
			jitter_px.x / float(vp.frame_size.x),
			-jitter_px.y / float(vp.frame_size.y));
		cam.update(jitter_ndc);


	
		{
			PROFILE(L"graph");
			pipeline.add_passes(graph);
		}

	   	voxel_gi->pass_data(graph.builder);

		graph.add_slot_generator([this](Graph& graph)
			{
				PROFILE(L"FrameInfo");
				auto& time = graph.get_context<TimeInfo>();
				auto& skyinfo = graph.get_context<SkyInfo>();
				auto& cam = graph.get_context<CameraInfo>();


				Slots::FrameInfo frameInfo;
				//// hack zone
				auto sky = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered);
				if (sky && sky->resource)
					frameInfo.GetSky() = *sky->get_handler<Handlers::TextureCube>();

				/////////
				frameInfo.GetSunDir().xyz = skyinfo.sunDir;
				frameInfo.GetTime() = { time.time, time.totalTime, 0, 0 };


				frameInfo.GetCamera() = cam.cam->camera_cb.current;
				frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d();
				frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d();

				// Material texture LOD bias — see FrameData.sig's mipBias comment.
				{
					auto& vp = graph.get_context<ViewportInfo>();
					frameInfo.GetMipBias() = downsampled
						? (std::log2(float(vp.frame_size.x) / float(vp.upscale_size.x)) - 1.0f)
						: 0.0f;
				}

				auto compiled = frameInfo.compile(*graph.builder.current_frame);
				graph.register_slot_setter(compiled);
			});

		graph.add_slot_generator([this](Graph& graph)
			{
				graph.register_slot_setter(scene->compiledScene);
			});
	}

	void draw(base::Context& t) override
	{
		auto saved_srv = texture.texture.texture2D;
		if (t.result_texture_srv) texture.texture.texture2D = t.result_texture_srv;
		image::draw(t);
		texture.texture.texture2D = saved_srv;
	}


	void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);
		if (r.w <= 64 || r.h <= 64) return;
		ivec2 size = r.size;
		cam.set_projection_params(Math::pi / 4, float(r.w) / r.h, 1, 1500);
	}
};

// Window content that previews an asset. Dispatches by type:
//   TextureAsset -> universal resource_preview (GPU), self-driven via a pass.
//   BinaryAsset  -> MultiLineLabel (one label per line) in a scroll container.
//   MeshAsset    -> a live render in AssetRenderer's OWN framegraph (a second
//                   main pipeline can't be injected into the shared UI graph),
//                   re-rendered every frame from think() into a target texture.
class asset_preview_content : public GUI::base, public FrameGraph::GraphGenerator
{
	resource_preview::ptr              m_preview; // textures only
	std::shared_ptr<TextureAsset>      m_asset;   // keeps the texture alive
	std::shared_ptr<HAL::ResourceView> m_view;

	std::shared_ptr<MeshAssetInstance>  m_mesh_instance;
	std::shared_ptr<HAL::Texture>       m_mesh_target;
	GUI::Elements::image::ptr           m_mesh_img;
	std::shared_ptr<SceneTextureRenderer> m_mesh_renderer; // own graph, per preview
	std::shared_ptr<MaterialAsset>        m_material;      // material preview
	bool m_mesh_dragging = false;
	vec2 m_mesh_prev;
public:
	using ptr = std::shared_ptr<asset_preview_content>;

	asset_preview_content(std::shared_ptr<Asset> asset)
	{
		docking     = GUI::dock::FILL;
		width_size  = GUI::size_type::MATCH_PARENT;
		height_size = GUI::size_type::MATCH_PARENT;
		clickable   = true; // mesh orbit; texture/binary children handle their own input first

		if (auto tex = asset ? asset->get_ptr<TextureAsset>() : nullptr)
		{
			m_asset   = tex;
			m_preview = std::make_shared<resource_preview>();
			add_child(m_preview);
			if (auto t = tex->get_texture())
			{
				m_view = std::make_shared<HAL::Texture2DView>(t->texture_2d());
				m_preview->set_source(m_view, "asset");
			}
		}
		else if (auto bin = asset ? asset->get_ptr<BinaryAsset>() : nullptr)
		{
			auto text     = std::make_shared<GUI::Elements::MultiLineLabel>();
			text->docking = GUI::dock::FILL;
			text->text    = bin->get_data();
			add_child(text);
		}
		else if (auto mesh = asset ? asset->get_ptr<MeshAsset>() : nullptr)
		{
			m_mesh_instance = mesh->create_instance();
			m_mesh_renderer = std::make_shared<SceneTextureRenderer>();

			m_mesh_img          = std::make_shared<GUI::Elements::image>();
			m_mesh_img->docking = GUI::dock::FILL;
			add_child(m_mesh_img);

			thinkable = true; // re-render each frame via the renderer's own graph
		}
		else if (auto material = asset ? asset->get_ptr<MaterialAsset>() : nullptr)
		{
			m_material      = material;
			m_mesh_renderer = std::make_shared<SceneTextureRenderer>();

			m_mesh_img          = std::make_shared<GUI::Elements::image>();
			m_mesh_img->docking = GUI::dock::FILL;
			add_child(m_mesh_img);

			thinkable = true; // material rendered on a test mesh, same as meshes
		}
	}

	bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
	{
		if (!m_mesh_renderer) return false;
		if (button == mouse_button::LEFT || button == mouse_button::RIGHT)
		{
			m_mesh_dragging = (action == mouse_action::DOWN);
			m_mesh_prev     = pos;
			set_movable(m_mesh_dragging);
			return true;
		}
		return false;
	}

	bool on_mouse_move(vec2 pos) override
	{
		if (m_mesh_renderer && m_mesh_dragging)
		{
			m_mesh_renderer->orbit((pos - m_mesh_prev) * 0.01f);
			m_mesh_prev = pos;
			return true;
		}
		return false;
	}

	bool on_wheel(mouse_wheel type, float value, vec2 pos) override
	{
		if (!m_mesh_renderer) return false;
		m_mesh_renderer->zoom(value);
		return true;
	}

	void think(float dt) override
	{
		if (!m_mesh_renderer || (!m_mesh_instance && !m_material)) return;

		// Size the render target to the widget; recreate it when the size changes
		// so the mesh follows window resizes instead of being stretched.
		ivec2 want = ivec2::max(ivec2(m_mesh_img->get_render_bounds().size), ivec2(64, 64));
		if (!m_mesh_target || ivec2(m_mesh_target->get_size().xy) != want)
		{
			m_mesh_target = std::make_shared<HAL::Texture>(RenderSystem::get().device(),
				HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { want }, 1, 6,
					HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess));
			m_mesh_img->texture.texture = m_mesh_target->texture_2d();
		}

		if (m_mesh_instance) m_mesh_renderer->draw(m_mesh_instance, m_mesh_target);
		else if (m_material) m_mesh_renderer->draw(m_material, m_mesh_target);
	}

	void generate(FrameGraph::Graph& graph) override
	{
		if (!m_view) return; // only the texture preview needs a GPU pass
		struct empty_pass_data {};
		// PassFlags::Required — the pass writes no graph-tracked resource, so it
		// would otherwise be culled; force it to always run.
		graph.add_pass<empty_pass_data>(L"AssetPreview",
			[](empty_pass_data&, FrameGraph::TaskBuilder&) { return true; },
			[this](empty_pass_data&, FrameGraph::FrameContext& ctx) { m_preview->render(&ctx); },
			FrameGraph::PassFlags::Required);
	}
};

UINT64 frame_counter = 0;


class FrameFlowGraph : public ::FlowGraph::graph
{};

class PassNode : public::FlowGraph::Node, public GUI::Elements::FlowGraph::VisualGraph
{
	Graph& graph;
	std::wstring_view pass_name;

public:
	PassNode(Graph& graph, std::wstring_view pass_name) : graph(graph), pass_name(pass_name)
	{}

	void operator()(::FlowGraph::GraphContext*) override
	{}

	GUI::base::ptr create_editor_window() override
	{
		GUI::Elements::image::ptr img(new GUI::Elements::image);
		img->texture = Skin::get().Shadow;
		img->padding = img->texture.padding;
		img->size = { 256, 256 };
		img->width_size = GUI::size_type::FIXED;
		img->height_size = GUI::size_type::FIXED;
		return img;
	}
};
namespace GUI
{


}


// Floating panel (shadow + dark background), styled like the FlowGraph canvas
// toolbar. Used to overlay the viewport with the debug-view selector.
class debug_toolbar_panel : public GUI::base
{
public:
	void draw(GUI::base::Context& c) override
	{
		GUI::Texture shadow = Skin::get().Shadow;
		shadow.margins = { 12, 12, 12, 12 };
		c.renderer->draw(c, shadow, get_render_bounds());

		GUI::Texture bg = Skin::get().FlowWindow.Inactive;
		bg.mul_color = float4(22, 28, 38, 255) / 255.0f;
		c.renderer->draw(c, bg, get_render_bounds());
	}
};

class GraphRender : public Window, public GUI::user_interface
{
	HAL::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::future<void> task_future;
	std::future<void> task_end;
	std::promise<void> promise_end;

	Graph graph;
	bool alive = false;
	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr instance_info;

	GUI::Elements::stat_graph::ptr graph_fps;
	GUI::Elements::stat_graph::ptr graph_frametime;
	GUI::Elements::stat_graph::ptr graph_vram;
	GUI::Elements::stat_graph::ptr graph_upload;
	GUI::Elements::stat_graph::ptr graph_readback;

	std::shared_ptr<triangle_drawer> drawer;
	std::shared_ptr<triangle_drawer> drawer2;
	std::shared_ptr<FrameFlowGraph> frameFlowGraph;
	GUI::base::ptr area;


	size_t graph_usage = 0;
		Pipelines::UIPipeline pipeline;
public:
	void on_destroy() override
	{

		Application::get().shutdown();
	}

	virtual void render()
	{
		PROFILE(L"render");
		if (swap_chain) swap_chain->resize(new_size);

		{
			std::lock_guard<std::mutex> g(m);

			if (GetAsyncKeyState('R'))
			{
				RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();
				//	RenderSystem::get().device().get_queue(HAL::CommandListType::COMPUTE)->signal_and_wait();
				//	RenderSystem::get().device().get_queue(HAL::CommandListType::COPY)->signal_and_wait();

				//   AssetManager::get().reload_resources();
				HAL::pixel_shader::reload_all();
				HAL::vertex_shader::reload_all();
				HAL::geometry_shader::reload_all();
				HAL::hull_shader::reload_all();
				HAL::domain_shader::reload_all();
				HAL::compute_shader::reload_all();
				HAL::library_shader::reload_all();
				HAL::mesh_shader::reload_all();
				HAL::amplification_shader::reload_all();

				HAL::Texture::reload_all();
			}

			auto f_gc = thread_pool::get().enqueue([]()
			{
				PROFILE(L"GarbageCollect");
				RenderSystem::get().device().get_heap_factory().GarbageCollect();
			});

			Profiler::get().on_frame(frame_counter++);
			

			GUI::user_interface::size = new_size;
			if (fps.tick())
			{
				size_t total = 0;
				//for (auto& h : HAL::DescriptorHeapManager::get().heaps)
				//{
				//	if (h)
				//	{
				//		total += h->used_size();
				//	}
				//}


				size_t total_gpu = 0;


				label_fps->text = std::to_string(fps.get()) + " " + std::to_string(RenderSystem::get().device().get_vram()) + " "
					+ std::to_string(total) + " " + std::to_string(total_gpu) + " " + std::to_string(graph_usage);
			}

			float frame_dt = (float)main_timer.tick();

			// fire independent work onto thread pool
			auto f_asset = thread_pool::get().enqueue([]()
			{
				PROFILE(L"AssetManager");
				AssetManager::get().tact();
			});

			auto f_rtx = thread_pool::get().enqueue([]()
			{
				PROFILE(L"RTX update");
				RTX::get().update();
			});

			auto f_times = thread_pool::get().enqueue([this, frame_dt]()
			{
				PROFILE(L"push_times");
				auto& dev = RenderSystem::get().device();
				if (graph_fps)       graph_fps->push(1.0f / frame_dt, frame_dt);
				if (graph_frametime) graph_frametime->push(frame_dt * 1000.0f, frame_dt);

				static float vram_accum = 0.0f;
				static size_t vram_cached = 0;
				vram_accum += frame_dt;
				if (vram_accum >= 1.0f) { vram_accum = 0.0f; vram_cached = dev.get_vram(); }
				if (graph_vram)      graph_vram->push((float)vram_cached, frame_dt);

				if (graph_upload)    graph_upload->push((float)dev.get_upload_heap(), frame_dt);
				if (graph_readback)  graph_readback->push((float)dev.get_readback_heap(), frame_dt);
			});

			
			// wait for all three before setup_graph
			{ PROFILE(L"async_wait"); f_asset.wait(); f_rtx.wait(); f_times.wait();  f_gc.wait(); }
		   	process_ui(frame_dt);

			swap_chain->wait_for_free();

			PROFILE(L"CPU FRAME");
			setup_graph();

			{
				PROFILE(L"add_passes");
				pipeline.add_passes(graph);

			}

			// Register the swapchain as a passed resource only after all passes
			// exist: max_passes (used to pre-size per-state append storage) is
			// captured from passes.size() at resource-creation time.
			graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, swap_chain->get_current_frame(), swap_chain->get_fence(), ResourceFlags::Required);

			graph.setup();
			graph.compile(swap_chain->m_frameIndex);


			graph.render();


			auto fence = graph.commit_command_lists();

			{
				PROFILE(L"reset");
				graph.reset();
			}

			if (frame_counter <= 5)
				Log::get() << "[Render] calling present frame " << frame_counter << Log::endl;
			swap_chain->present();
			if (frame_counter <= 5)
				Log::get() << "[Render] present returned frame " << frame_counter << Log::endl;
		}


		if (alive)
		{

			task_future = thread_pool::get().enqueue([this]()
				{
					render();
				});
		}
		else
		{

			promise_end.set_value();
		}
	}


	void setup_graph()
	{
		PROFILE(L"setup_graph");
		graph.start_new_frame();
		

		{
			PROFILE(L"create_graph");
			create_graph(graph);
		}

		static bool gen = false;

		if (!gen && GetAsyncKeyState('N'))
		{
			{
				//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
				//	t->init(&Profiler::get());
				//	dock->get_tabs()->add_page("Profiler", t);

				//	GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());



				//docker->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(frameFlowGraph));
			}


			gen = true;
			frameFlowGraph->clear();

			struct res_stage
			{};
			std::map<ResourceAllocInfo*, ::FlowGraph::parameter::ptr> resource_stages;
			std::map<Pass*, float> pass_positions;
			std::map<CommandListType, float> list_positions;


			for (auto& chain : graph.builder.alloc_resources)
			{
				if (chain.empty()) continue;
				auto& info = chain.active();
				if (info.passed)
				{
					auto input = frameFlowGraph->register_input(info.name());
					resource_stages[&info] = input;
				}
			}
			uint offset = 0;
			for (auto pass : graph.builder.enabled_passes)
			{
				if (!pass->enabled)
					continue;

				auto node = std::make_shared<PassNode>(graph, pass->name.ptr);
				node->name = convert(std::wstring(pass->name.ptr)) + " " + std::to_string(pass->id);

				CommandListType pass_type = check(pass->flags & PassFlags::Compute)
					? CommandListType::COMPUTE
					: CommandListType::DIRECT;
				if (!pass->enabled)
				{
					node->color = float4(1, 0, 0, 1);
				}

				if (check(pass->flags & PassFlags::Required))
				{
					node->color = float4(1, 1, 0, 1);
				}

				float my_pos = list_positions[pass_type];

				for (auto& sync_pass : pass->sync_state.values)
				{
					if (!sync_pass) continue;
					my_pos = std::max(my_pos, pass_positions[const_cast<Pass*>(sync_pass)]);
				}

				node->pos = { my_pos + 800, check(pass->flags & PassFlags::Compute) * 500 };


				pass_positions[pass] = node->pos.x;
				list_positions[pass_type] = node->pos.x;

				frameFlowGraph->register_node(node);
				offset = std::max(offset, uint(node->pos.x));

				for (auto& info : pass->used.resources)
				{
					if (pass->used.resource_creations.count(info) == 0)
					{
						auto input = node->register_input(info->name());
						auto prev = resource_stages[info];

						if (prev)
						{
							prev->link(input);
						}
						auto resource_flags = pass->used.resource_flags[info];


						if (check(resource_flags & FrameGraph::WRITEABLE_FLAGS))
						{
							auto output = node->register_output(info->name());
							resource_stages[info] = output;
						}
					}
				}


				for (auto& info : pass->used.resources)
				{
					if (pass->used.resource_creations.count(info))
					{
						auto output = node->register_output(info->name());
						resource_stages[info] = output;
					}
				}
			}


			for (auto& chain : graph.builder.alloc_resources)
			{
				if (chain.empty()) continue;
				auto& info = chain.active();
				if (info.passed && check(info.flags & ResourceFlags::Required))
				{
					auto input = frameFlowGraph->register_output(info.name());
					auto prev = resource_stages[&info];

					if (prev)
					{
						prev->link(input);
					}
				}
			}
			frameFlowGraph->pos_in = { -800, 0 };
			frameFlowGraph->pos_out = { offset + 800, 0 };
		}
	}

	GUI::Elements::dock_base::ptr docker;

	GraphRender()
	{

		//scale = 1.25f;
		Window::input_handler = this;
		HAL::swap_chain_desc desc;
		desc.format = HAL::Format::B8G8R8A8_UNORM;
		desc.fullscreen = nullptr;
		desc.stereo = false;
		desc.window = this;
		swap_chain = std::make_shared<HAL::SwapChain>(RenderSystem::get().device(), desc);

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
			back->texture = Skin::get().Fill;
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
			auto d = std::make_shared<GUI::Elements::dock_base>();
			docker = d;
			d->docking = GUI::dock::FILL;
#ifndef HAL_BACKEND_VULKAN
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;

				// Floating debug-view selector over the viewport: each button sets
				// graph's DebugContext::mode, which repoints what UI_Render composites.
				{
					struct DbgOpt { const char* name; FrameGraph::DebugMode mode; };
					static const DbgOpt dbg_opts[] = {
						{ "Final",         FrameGraph::DebugMode::Final },
						{ "Albedo",        FrameGraph::DebugMode::Albedo },
						{ "Motion",        FrameGraph::DebugMode::Motion },
						{ "GI Indirect",   FrameGraph::DebugMode::GI_Indirect },
						{ "GI Reflection", FrameGraph::DebugMode::GI_Reflection },
						{ "Voxel Trace",   FrameGraph::DebugMode::VoxelTrace },
						{ "RTX",           FrameGraph::DebugMode::RTX },
					};

					auto toolbar = std::make_shared<debug_toolbar_panel>();
					toolbar->docking     = GUI::dock::NONE;
					toolbar->pos         = vec2(8, 8);
					toolbar->height_size = GUI::size_type::MATCH_CHILDREN;
					toolbar->width_size  = GUI::size_type::MATCH_CHILDREN;
					toolbar->padding     = { 4, 4, 4, 4 };

					auto debug_combo = std::make_shared<GUI::Elements::combo_box>();
					debug_combo->docking = GUI::dock::TOP;
					debug_combo->size = { 140, 24 };
					for (auto& o : dbg_opts)
					{
						auto mode = o.mode;
						debug_combo->add_item(o.name)->on_select =
							[this, mode]() { graph.get_context<FrameGraph::DebugContext>().mode = mode; };
					}
					toolbar->add_child(debug_combo);

					// DLSS-quality selector, writes g_upscaling_dlss_mode.
					// "Off" isn't offered — DLSS on/off is via FSR/DLSS
					// mutual exclusion elsewhere, not DLSSMode::Off.
					if (nvidia::DLSS::get().available())
					{
						struct DlssOpt { const char* name; nvidia::DLSSMode mode; };
						static const DlssOpt dlss_opts[] = {
							{ "Max Performance",   nvidia::DLSSMode::MaxPerformance },
							{ "Balanced",          nvidia::DLSSMode::Balanced },
							{ "Max Quality",       nvidia::DLSSMode::MaxQuality },
							{ "Ultra Quality",     nvidia::DLSSMode::UltraQuality },
							{ "Ultra Performance", nvidia::DLSSMode::UltraPerformance },
							{ "DLAA",              nvidia::DLSSMode::DLAA },
						};

						auto dlss_combo = std::make_shared<GUI::Elements::combo_box>();
						dlss_combo->docking = GUI::dock::TOP;
						dlss_combo->size = { 140, 24 };
						for (auto& o : dlss_opts)
						{
							// Skip modes SL reports unsupported (e.g. UltraQuality
							// at 1920x1080 returns render_size (0,0)).
							nvidia::DLSSOptimalSettings probe{};
							if (!nvidia::DLSS::get().get_optimal_settings(o.mode, { 1920, 1080 }, /*hdr=*/true, probe))
								continue;

							auto mode = o.mode;
							dlss_combo->add_item(o.name)->on_select =
								[mode]() { g_upscaling_dlss_mode = mode; };
							if (mode == g_upscaling_dlss_mode)
								dlss_combo->get_label()->text = o.name;
						}
						toolbar->add_child(dlss_combo);
					}

					drawer->GUI::base::add_child(toolbar);
				}

				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
#endif

			{
				//	auto text = std::make_shared<GUI::Elements::MultiLineLabel>();
				////	auto f = FileSystem::get().get_file(to_path(L"main.cpp"))->load_all();


					//text->text = f;
					///d->get_tabs()->add_page("text", text);
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

							for (auto v : elem->variables)
							{
								auto property = GUI::Elements::create_property(*v);
								table->add_child(property);
							}
						};

					folders->init(&VariableContext::get());

					dock->get_tabs()->add_page("Properties", b);
				}
				dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
				//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
				//                    t->init(drawer->scene.get());
				//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);


					frameFlowGraph = std::make_shared<FrameFlowGraph>();

					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(frameFlowGraph));
				}

				{
					auto stats_panel = std::make_shared<GUI::base>();
					stats_panel->docking      = GUI::dock::FILL;
					stats_panel->width_size   = GUI::size_type::MATCH_PARENT;
					stats_panel->height_size  = GUI::size_type::MATCH_PARENT;

					graph_fps.reset(new GUI::Elements::stat_graph());
					graph_fps->title     = "FPS";
					graph_fps->size      = {0, 70};
					graph_fps->min_range = 150.0f;
					graph_fps->mode      = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_fps);

					graph_frametime.reset(new GUI::Elements::stat_graph());
					graph_frametime->title      = "Frame Time";
					graph_frametime->unit       = "ms";
					graph_frametime->size       = {0, 70};
					graph_frametime->min_range  = 20.0f;
					graph_frametime->line_color = {0.95f, 0.70f, 0.20f, 1.00f};
					graph_frametime->fill_color = {0.95f, 0.70f, 0.20f, 0.15f};
					graph_frametime->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_frametime);

					graph_vram.reset(new GUI::Elements::stat_graph());
					graph_vram->title      = "VRAM";
					graph_vram->size       = {0, 70};
					graph_vram->min_range  = 100.0f;
					graph_vram->line_color = {0.90f, 0.30f, 0.30f, 1.00f};
					graph_vram->fill_color = {0.90f, 0.30f, 0.30f, 0.15f};
					graph_vram->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_vram);

					graph_upload.reset(new GUI::Elements::stat_graph());
					graph_upload->title      = "Upload Heap";
					graph_upload->unit       = "MB";
					graph_upload->size       = {0, 70};
					graph_upload->min_range  = 50.0f;
					graph_upload->line_color = {0.40f, 0.70f, 1.00f, 1.00f};
					graph_upload->fill_color = {0.40f, 0.70f, 1.00f, 0.15f};
					graph_upload->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_upload);

					graph_readback.reset(new GUI::Elements::stat_graph());
					graph_readback->title      = "Readback Heap";
					graph_readback->unit       = "MB";
					graph_readback->size       = {0, 70};
					graph_readback->min_range  = 50.0f;
					graph_readback->line_color = {1.00f, 0.60f, 0.20f, 1.00f};
					graph_readback->fill_color = {1.00f, 0.60f, 0.20f, 0.15f};
					graph_readback->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_readback);

					dock->get_tabs()->add_page("Stats", stats_panel);
					dock->size = {100, 230};
				}
			}

			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help"); // ->get_menu();
					file->add_item("New")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							add_task([this]()
								{
									if (!drawer) return;
									drawer->scene->remove_all();
								});
						};
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							add_task([this]()
								{
									if (!drawer) return;
									try
									{
										auto f = FileSystem::get().get_file(to_path(L"scene.dat"))->load_all();
										Serializer::deserialize(f, *drawer->scene);
									}
									catch (std::exception e)
									{
										message_box("error", "cant open", [](bool)
											{
											});
									}
								});
						};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							if (!drawer) return;
							auto data = Serializer::serialize(*drawer->scene);
							FileSystem::get().save_data(to_path(L"scene.dat"), data);
						};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							on_destroy();
						};

					edit->add_item("Debug Graph")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{

							docker->get_tabs()->add_page("GraphDebug", FrameGraphDebug::create_debug_layout(graph));

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
					label_tiles->margin = { 20, 0, 0, 0 };
					add_child(bar);
				}


				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };

					// Open a texture asset in a preview window ("Preview" in the
					// asset context menu). Lives here (above the Graphics module) so
					// it can use resource_preview.
					GUI::Elements::asset_item::on_open_preview = [this](std::shared_ptr<Asset> a)
					{
						if (!a) return;
						auto wnd     = std::make_shared<GUI::Elements::window>();
						auto content = std::make_shared<asset_preview_content>(a);
						wnd->add_child(content);
						wnd->pos  = { 250, 250 };
						wnd->size = { 520, 560 };
						if (auto ui = get_user_ui()) ui->add_child(wnd);
					};

					// Live material preview embedded in the material graph's output node.
					MaterialGraph::create_preview_hook = [](std::shared_ptr<Asset> a) -> GUI::base::ptr
					{
						if (!a) return nullptr;
						auto box         = std::make_shared<GUI::base>();
						box->width_size  = GUI::size_type::FIXED;
						box->height_size = GUI::size_type::FIXED;
						box->size        = { 160, 160 };
						box->add_child(std::make_shared<asset_preview_content>(a));
						return box;
					};
					EVENT("End Asset Explorer");
				}
			}
		}
	}

	~GraphRender() override
	{

	}


	void stop()
	{

		alive = false;
		task_end.wait();

		while (Events::Runner::has_tasks())
		{
			PROFILE(L"process_tasks");
			Events::Runner::process_tasks();
		}
	}

	void start()
	{
		promise_end = {};
		task_end = promise_end.get_future();
		alive = true;
		//task_future = thread_pool::get().enqueue([this]()
		//	{
		render();
		//	});

	}
	void on_resize(vec2 size) override
	{
		new_size = vec2::max(size, vec2{ 64, 64 });

		/*bool was_alive = alive;
	if (was_alive) stop();

	//Sleep(10);
	if (was_alive)
	{

		start();
		RenderSystem::get().device().get_queue(CommandListType::DIRECT)->signal_and_wait();
		RenderSystem::get().device().get_queue(CommandListType::COMPUTE)->signal_and_wait();
		RenderSystem::get().device().get_queue(CommandListType::COPY)->signal_and_wait();
}*/

	}


	void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
	}
};

class RenderApplication : public Application
{
	friend class Singleton<Application>;


	std::shared_ptr<GraphRender> main_window;
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
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());
		GraphicsSystem::create();
		main_window = std::make_shared<GraphRender>();
		main_window->start();
	}

	~RenderApplication() override
	{
		shutdown();
		main_window->stop();
		main_window = nullptr;
		scheduler::reset();
		GraphicsSystem::reset();
	}


	void on_tick() override
	{
	//	if (GetAsyncKeyState(VK_ESCAPE))
	//		shutdown();

		Window::process_messages();
	}

	std::vector<std::string> file_open(const std::string& Name, const std::string& StartPath,
		const std::string& Extension) override
	{
		return Window::file_open(Name, StartPath, Extension);
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
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_INFO);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_INFO);
	ClassLogger<HAL::Resource>::get().set_logging_level(Log::LEVEL_INFO);
	Log::get() << Log::LEVEL_INFO << "info text" << Log::endl;
	Log::get() << Log::LEVEL_WARNING << "warning text" << Log::endl;
	Log::get() << Log::LEVEL_DEBUG << "debug text" << Log::endl;
	Log::get() << Log::LEVEL_ERROR << "error text" << Log::endl;
}

struct test
{
	std::string str = "wtf";
	vec4 data = { 1, 2, 3, 4 };

	std::vector<vec2> vec;

	test()
	{
		vec.emplace_back(1, 2);
		vec.emplace_back(3, 4);
		vec.emplace_back(5, 6);
	}

	template <class T = void>
	void foo() requires(false)
	{}

	SERIALIZE()
	{
		ar&  NVP(str)& NVP(data)& NVP(vec);
	}
} v;

#include <shellscalingapi.h>


int APIENTRY WinMain(_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
{
//	setlocale(LC_ALL, "");
	CoInitialize(NULL);
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	FlowGraph::FlowSystem::get().register_node("file", []()-> FlowGraph::graph::ptr
		{
			auto f = FileSystem::get().get_file(to_path("graph.flg"));

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

	return result_code;
}
