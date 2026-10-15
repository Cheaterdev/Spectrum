// The demo screen both hosts run: the engine's UI tree over a lit scene (the real GBuffer, sun-light and sky
// passes through the FrameGraph), a camera driven by touch or mouse, and the final copy into the swapchain.
// Nothing here knows the window system: a host (the Android activity, the Windows `demo` command) owns the
// surface, creates the swapchain and the FP16 target, forwards input to ui(), and calls draw_frame().
export module Demo;

import Core;
import HAL;
import RenderSystem;
import Graphics;
import FrameGraph;
import GUI;

namespace
{
	float g_density = 2.0f;
	bool g_shadows = true;
	bool g_ui_visible = true;

	std::mutex g_ui_mutex;   // the touch thread reads g_ui while the render thread creates it
	std::shared_ptr<GUI::user_interface> g_ui;

	std::shared_ptr<GUI::user_interface> current_ui()
	{
		std::lock_guard<std::mutex> lock(g_ui_mutex);
		return g_ui;
	}

	// The scene behind the UI: shows the graph's ResultTexture full screen and drives the camera.
	//   Touch - two sticks: a finger that lands on the left half is a floating move stick (hold it away from
	//           where it landed to walk, the farther the faster); one on the right half looks around.
	//   Mouse - drag with the left button to look, the wheel changes the move speed.
	// Input arrives on the platform thread while the camera is read by the render thread, hence the mutex.
	class sky_view : public GUI::Elements::image
	{
		struct finger
		{
			bool is_stick;
			vec2 origin, last;
		};

		static constexpr float stick_radius_dp = 70.0f;
		static constexpr float dead_zone = 0.08f;

		std::map<int, finger> fingers;
		std::mutex m;
		vec2 look = { 0, 0 };
		vec2 stick = { 0, 0 };        // -1..1, x to the right, y down the screen
		vec2 stick_origin = { 0, 0 };
		vec2 stick_pos = { 0, 0 };
		bool stick_active = false;
		bool mouse_looking = false;
		vec2 mouse_last = { 0, 0 };
		float speed = 1.0f;

	public:
		float density = 1.0f;

		sky_view()
		{
			docking = GUI::dock::NONE;
			clickable = true;
			can_multitouch = true;
		}

		struct input { vec2 look, stick; float speed; };

		input consume()
		{
			std::lock_guard<std::mutex> lock(m);
			input in{ look, stick, speed };
			look = vec2(0, 0);
			return in;
		}

		bool on_touch_action(mouse_action action, GUI::touch& t) override
		{
			std::lock_guard<std::mutex> lock(m);
			if (action == mouse_action::DOWN)
			{
				const bool is_stick = t.pos.x < get_render_bounds().w * 0.5f;
				for (auto& [id, f] : fingers)
					if (f.is_stick == is_stick)
						return true;   // that half already has its finger
				fingers[t.id] = { is_stick, t.pos, t.pos };
				if (is_stick)
				{
					stick_active = true;
					stick_origin = stick_pos = t.pos;
				}
			}
			else if (auto it = fingers.find(t.id); it != fingers.end())
			{
				if (it->second.is_stick)
				{
					stick = vec2(0, 0);
					stick_active = false;
				}
				fingers.erase(it);
			}
			return true;
		}

		void on_touch_move(std::vector<GUI::touch*>& touches) override
		{
			std::lock_guard<std::mutex> lock(m);
			for (auto* t : touches)
			{
				auto it = fingers.find(t->id);
				if (it == fingers.end())
					continue;

				auto& f = it->second;
				if (f.is_stick)
				{
					const float radius = stick_radius_dp * density;
					vec2 v = (t->pos - f.origin) / radius;
					if (v.length() > 1.0f)
						v = v.normalize();
					stick_pos = f.origin + v * radius;
					stick = v.length() < dead_zone ? vec2(0, 0) : v;
				}
				else
				{
					look += t->pos - f.last;
					f.last = t->pos;
				}
			}
		}

		bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
		{
			if (button != mouse_button::LEFT)
				return false;
			std::lock_guard<std::mutex> lock(m);
			mouse_looking = action == mouse_action::DOWN;
			mouse_last = pos;
			return true;
		}

		bool on_mouse_move(vec2 pos) override
		{
			std::lock_guard<std::mutex> lock(m);
			if (!mouse_looking)
				return false;
			look += pos - mouse_last;
			mouse_last = pos;
			return true;
		}

		bool on_wheel(mouse_wheel type, float value, vec2 pos) override
		{
			std::lock_guard<std::mutex> lock(m);
			speed = std::clamp(speed * (1.0f + 0.1f * value), 0.1f, 20.0f);
			return true;
		}

		void draw(Context& c) override
		{
			auto saved = texture.texture.texture2D;
			if (c.result_texture_srv)
				texture.texture.texture2D = c.result_texture_srv;
			image::draw(c);
			texture.texture.texture2D = saved;

			std::lock_guard<std::mutex> lock(m);
			if (stick_active)
			{
				const float r = stick_radius_dp * density;
				c.renderer->draw_color(c, float4(1, 1, 1, 0.12f), ::rect{ stick_origin.x - r, stick_origin.y - r, 2 * r, 2 * r });
				c.renderer->draw_color(c, float4(1, 1, 1, 0.35f), ::rect{ stick_pos.x - 0.35f * r, stick_pos.y - 0.35f * r, 0.7f * r, 0.7f * r });
			}
		}
	};

	std::shared_ptr<sky_view> g_sky_view;
	std::unique_ptr<first_person_camera> g_camera;
	std::unique_ptr<SkyRender> g_sky_render;
	std::unique_ptr<VSM> g_vsm;
	std::unique_ptr<BlueNoise> g_blue_noise;
	Scene::ptr g_scene;
	main_renderer::ptr g_main_renderer;
	std::unique_ptr<Pipelines::Environment::SkyViewPipeline> g_sky_pipeline;

	// The demo screen, built from the engine's own elements. Everything sits in `g_root`, whose layout
	// scale is the density: children are positioned and sized in dp, the root itself in pixels.
	GUI::base::ptr g_root;
	GUI::Elements::window::ptr g_window_el;
	GUI::Elements::button::ptr g_buttons[3];
	GUI::Elements::progress_bar::ptr g_progress;
	GUI::Elements::label::ptr g_body;
	GUI::Elements::stat_graph::ptr g_graph_ms, g_graph_fps;
	int g_clicks[3] = {};
	ivec2 g_laid_out_for = { 0, 0 };

	// The per-frame constants every pass reads (camera, sun, lookup textures, the baked sky). The sky only exists
	// once the graph has allocated it, so a pass that needs it fills this in itself while it records.
	Slots::Frame::FrameInfo make_frame_info(FrameGraph::Graph& graph, const HLSL::TextureCube<float4>* sky_cube = nullptr)
	{
		auto& time = graph.get_context<TimeInfo>();
		auto& skyinfo = graph.get_context<SkyInfo>();
		auto& cam = graph.get_context<CameraInfo>();
		Slots::Frame::FrameInfo frameInfo;
		if (sky_cube)
			frameInfo.GetSky() = *sky_cube;
		frameInfo.GetSunDir().xyz = skyinfo.sunDir;
		frameInfo.GetTime() = { time.time, time.totalTime, 0, 0 };
		frameInfo.GetCamera() = cam.cam->camera_cb.current;
		frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;
		frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
		frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
		return frameInfo;
	}

	void ensure_ui()
	{
		if (current_ui())
			return;

		// Built outside g_ui_mutex and published at the end: loading the skin and sky assets takes a while,
		// and the platform thread's touch path takes that mutex, so holding it here stalls input (an ANR).
		THREAD_SCOPE(GUI);
		Skin::get();   // loads every skin texture now rather than inside the first frame
		auto ui = std::make_shared<GUI::user_interface>();   // shared-owned: the touch path walks to parents via shared_from_this

		// Hooks the desktop window provides; set_capture is called by draggable elements (the window title).
		ui->set_capture = [](bool) {};
		ui->get_clipboard = [] { return std::wstring(); };
		ui->set_clipboard = [](std::wstring_view) {};

		g_sky_view = std::make_shared<sky_view>();   // first child: drawn behind the window and graphs
		g_sky_view->density = g_density;
		ui->add_child(g_sky_view);

		g_camera = std::make_unique<first_person_camera>();
		g_camera->position = vec3(0, 1.7f, 0);
		g_camera->look_at(vec3(0, 1.0f, 6));
		g_sky_pipeline = std::make_unique<Pipelines::Environment::SkyViewPipeline>();
		g_sky_render = std::make_unique<SkyRender>(*g_sky_pipeline);
		g_blue_noise = std::make_unique<BlueNoise>(*g_sky_pipeline);

		// Shadows: the engine's VSM with a resident atlas (no sparse textures) and a fixed sun, PCF filtered (the PCSS
		// penumbra path runs too, behind use_vsm_penumbra).
		// SkyAmbient adds the baked sky as the indirect light ray tracing and voxel GI would supply.
		g_vsm = std::make_unique<VSM>(*g_sky_pipeline);
		g_vsm->enabled = g_shadows;
		g_vsm->use_vsm_penumbra = false;   // PCF by default: PCSS costs several times more on a phone ("VSM/PCSS penumbra=1" in demo.cfg turns it on)
		g_vsm->set_position(float3(0.3f, 0.35f, 0.4f).normalize());

		// Knobs for trying things on a device without a rebuild: "demo.cfg" in the working directory, one
		// "Context/Variable=value" per line, naming a Variable<T> as the Properties panel does (bool and float take
		// the number, an enum its index), e.g. "VSM/PCSS penumbra=0" or "Sync debug/Widen every barrier to ALL/ALL=1"
		// -- the context is everything before the first '/'.
		{
			std::ifstream cfg("demo.cfg");
			std::string line;
			while (std::getline(cfg, line))
			{
				const auto eq = line.rfind('=');
				const auto slash = line.find('/');
				if (eq == std::string::npos || slash == std::string::npos || slash > eq)
					continue;
				const std::string context_name = line.substr(0, slash);
				const std::string variable_name = line.substr(slash + 1, eq - slash - 1);
				const std::string value = line.substr(eq + 1);

				bool applied = false;
				VariableContext::get().iterate([&](VariableContext* context)
				{
					const std::wstring wide = context->get_name();
					if (std::string(wide.begin(), wide.end()) != context_name)
						return true;
					for (auto* variable : context->variables)
					{
						if (variable->get_name() != variable_name)
							continue;
						if (auto* flag = dynamic_cast<Variable<bool>*>(variable))
							*flag = std::atoi(value.c_str()) != 0, applied = true;
						else if (auto* number = dynamic_cast<Variable<float>*>(variable))
							*number = std::strtof(value.c_str(), nullptr), applied = true;
						else if (!variable->get_enum_names().empty())
							variable->set_enum_index(std::atoi(value.c_str())), applied = true;
					}
					return true;
				});
				Log::get() << "demo.cfg: " << context_name << "/" << variable_name << " = " << value << (applied ? "" : "  (NOT FOUND)") << Log::endl;
			}
		}

		// VSM switched off: the engine's own fallback lights the GBuffer with the sun alone (VSM::m_unshadowed_render,
		// which needs the VSM object itself, so it is repeated here to bind the baked sky).
		g_sky_pipeline->vSM_Unshadowed.render_func = [](Passes::Shadows::VSM::VSM_Unshadowed::Context& data, FrameGraph::FrameContext& context)
		{
			GBuffer gbuffer = GBufferViewDesc::actualize(data);

			auto& compute = context.get_list()->get_compute();
			// Not the generated slot: that one is built before the baked sky exists.
			if (data.sky_cubemap_filtered)
				compute.set(make_frame_info(*context.graph, &data.sky_cubemap_filtered->textureCube));
			else
				compute.set(make_frame_info(*context.graph));

			Slots::Shadows::VSM::VSMLighting lighting;
			gbuffer.SetTable(lighting.GetGbuffer());
			lighting.GetResult() = data.ResultTexture->rwTexture2D;
			compute.set(lighting);

			compute.set_pipeline<PSOS::Shadows::VSM::VSMUnshadowed>();
			compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
		};

		// The scene: the engine's own meshes (with their materials) in front of the camera.
		g_scene = std::make_shared<Scene>();
		g_vsm->attach_scene(g_scene);
		g_main_renderer = std::make_shared<main_renderer>();
		g_main_renderer->register_renderer(std::make_shared<mesh_renderer>());

		auto place = [](EngineAsset<MeshAsset>& asset, vec3 position, float scale) {
			auto instance = asset.get_asset()->create_instance();
			instance->local_transform.scaling(scale);
			instance->local_transform.a41 = position.x;
			instance->local_transform.a42 = position.y;
			instance->local_transform.a43 = position.z;
			g_scene->add_child(instance);
		};
		place(EngineAssets::plane, vec3(0, 0, 0), 40.0f);
		place(EngineAssets::material_tester, vec3(0, 1, 6), 1.0f);
		place(EngineAssets::axis, vec3(-3, 0, 7), 0.25f);

		g_root = std::make_shared<GUI::base>();
		g_root->docking = GUI::dock::NONE;
		// Full screen and in front of the sky view: left clickable it takes every touch first, and its default
		// move handler marks the finger as handled, so the sky view only ever saw a second finger.
		g_root->clickable = false;
		g_root->scale = g_density;
		g_root->visible = g_ui_visible;
		ui->add_child(g_root);

		g_window_el = std::make_shared<GUI::Elements::window>();
		g_window_el->set_title("Spectrum \xE2\x80\x94 Android");
		g_root->add_child(g_window_el);

		static const char* const names[3] = { "Play", "Options", "Quit" };
		for (int i = 0; i < 3; ++i)
		{
			auto b = std::make_shared<GUI::Elements::button>();
			b->docking = GUI::dock::NONE;
			b->get_label()->text = names[i];
			b->get_label()->font_size = 16.0f;
			b->get_label()->magnet_text = FW1_CENTER | FW1_VCENTER | FW1_NOWORDWRAP;
			b->on_click = [i](GUI::Elements::button::ptr button) {
				const char* name = names[i];
				button->get_label()->text = std::string(name) + "  " + std::to_string(++g_clicks[i]);
			};
			g_window_el->add_child(b);
			g_buttons[i] = b;
		}

		g_progress = std::make_shared<GUI::Elements::progress_bar>();
		g_progress->docking = GUI::dock::NONE;
		g_progress->track_color = { 0.08f, 0.09f, 0.11f, 1.0f };
		g_progress->fill_color = { 0.95f, 0.65f, 0.15f, 1.0f };
		g_window_el->add_child(g_progress);

		g_body = std::make_shared<GUI::Elements::label>();
		g_body->docking = GUI::dock::NONE;
		g_body->font_size = 16.0f;
		g_window_el->add_child(g_body);

		auto make_graph = [](const char* title, const char* unit, float min_range, float4 color) {
			auto g = std::make_shared<GUI::Elements::stat_graph>();
			g->title = title;
			g->unit = unit;
			g->min_range = min_range;
			g->line_color = color;
			g->fill_color = { color.x, color.y, color.z, 0.3f };
			g->docking = GUI::dock::NONE;
			g->width_size = GUI::size_type::FIXED;
			g_root->add_child(g);
			return g;
		};
		g_graph_ms = make_graph("Frame time", "ms", 5.0f, { 0.30f, 0.85f, 0.30f, 1.0f });
		g_graph_fps = make_graph("FPS", "", 20.0f, { 0.35f, 0.65f, 1.0f, 1.0f });

		std::lock_guard<std::mutex> lock(g_ui_mutex);
		g_ui = ui;
	}

	// Positions in dp for a surface of (W, H) pixels. Done when the size changes only, so a window the
	// user has dragged stays where it was put.
	void layout_ui(float W, float H)
	{
		const float wdp = W / g_density, hdp = H / g_density;
		const float margin = wdp * 0.05f, pad = 12.0f, title_h = 40.0f, btn_h = 44.0f, bar_h = 14.0f;
		const float panel_x = margin, panel_w = wdp - 2 * margin, panel_y = hdp * 0.08f;
		const float panel_h = std::max(hdp * 0.42f, 230.0f);
		const float inner_w = panel_w - 40.0f;   // the window's 20dp margin sits outside its pos/size

		g_ui->size = vec2(W, H);
		g_sky_view->pos = vec2(0, 0);
		g_sky_view->size = vec2(W, H);
		g_root->size = vec2(W, H);   // the root is sized in pixels, only its children are in dp
		g_window_el->pos = vec2(panel_x + 20.0f, panel_y + 20.0f);
		g_window_el->size = vec2(inner_w, panel_h - 40.0f);

		const float btn_w = (inner_w - 4 * pad) / 3.0f;
		for (int i = 0; i < 3; ++i)
		{
			g_buttons[i]->pos = vec2(pad + i * (btn_w + pad), pad);
			g_buttons[i]->size = vec2(btn_w, btn_h);
		}
		const float bar_y = pad + btn_h + pad;
		g_progress->pos = vec2(pad, bar_y);
		g_progress->size = vec2(inner_w - 2 * pad, bar_h);
		g_body->pos = vec2(pad, bar_y + bar_h + pad);
		g_body->size = vec2(inner_w - 2 * pad, 30.0f);

		const float gy = panel_y + panel_h + pad, gh = 90.0f;
		g_graph_ms->pos = vec2(panel_x, gy);
		g_graph_ms->size = vec2(panel_w, gh);
		g_graph_fps->pos = vec2(panel_x, gy + gh + pad);
		g_graph_fps->size = vec2(panel_w, gh);
	}

	// Touch arrives as one nativeTouchPoint per finger and a nativeTouchFlush(flags) that ends the message.

	std::unique_ptr<FrameGraph::Graph> g_graph;
	std::unique_ptr<Pipelines::UI::UIPipeline> g_pipeline;

	// One UI frame through the FrameGraph, as the desktop app does it: the widget tree is walked into
	// the UI context (create_graph), UIPipeline's UI_PreDraw / UI_Render passes are added, and the graph
	// draws into ui_target in place of the swapchain. That target is FP16 because the UI PSOs write
	// FP16; the SDR swapchain gets it through the CopyTexture pass at the end.
	void draw_ui_frame(HAL::Device& device, HAL::SwapChain& swap, const std::shared_ptr<HAL::TextureResource>& ui_target, int frame)
	{
		swap.wait_for_free();
		ensure_ui();

		if (!g_graph)
		{
			g_graph = std::make_unique<FrameGraph::Graph>();
			g_pipeline = std::make_unique<Pipelines::UI::UIPipeline>();
		}
		auto& graph = *g_graph;

		auto target = swap.get_current_frame();
		const auto& dims = target->get_desc().as_texture().Dimensions;
		const float W = (float)dims.x, H = (float)dims.y;

		static auto last = std::chrono::steady_clock::now();
		const auto now = std::chrono::steady_clock::now();
		const float dt = std::chrono::duration<float>(now - last).count();
		last = now;

		{
			PROFILE(L"demo_ui_update");
			if (g_laid_out_for != ivec2((int)dims.x, (int)dims.y))
			{
				layout_ui(W, H);
				g_laid_out_for = ivec2((int)dims.x, (int)dims.y);
			}

			g_progress->value = std::fmod(frame * 0.004f, 1.0f);
			g_body->text = "Hello \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD1\x96\xD1\x82 \xCE\x93\xCE\xB5\xCE\xB9\xCE\xAC \xF0\x9F\x98\x80\xF0\x9F\x9A\x80 frame " + std::to_string(frame);
			if (dt > 0.0f)
			{
				g_graph_ms->push(dt * 1000.0f, dt);
				g_graph_fps->push(1.0f / dt, dt);
			}
			g_ui->process_ui(dt);
		}

		{
			PROFILE(L"demo_graph_build");
			graph.start_new_frame();
			graph.get_context<TimeInfo>().frame = frame;

			auto& display = graph.get_context<Table::UI::DisplayState>();
			display.hdr = false;
			display.max_nits = 80.0f;
			display.sdr_white_nits = 80.0f;

			// Camera: look delta since the last frame plus the held move stick (see sky_view).
			{
				auto in = g_sky_view->consume();
				auto& cam = *g_camera;
				cam.add_look(in.look);
				cam.move_input = vec3(in.stick.x, 0.0f, -in.stick.y);   // stick up = forward
				cam.move_speed = 4.0f * in.speed;                       // units per second at full deflection
				cam.frame_move(std::min(dt, 0.1f));
				cam.set_projection_params(Math::pi / 3, W / H, 0.1f, 1500.0f);
				cam.update();
			}

			graph.get_context<TimeInfo>().time = dt;
			graph.get_context<TimeInfo>().totalTime += dt;
			graph.get_context<SkyInfo>().sunDir = g_vsm->get_position();
			graph.get_context<CameraInfo>().cam = g_camera.get();
			{
				auto& scene_info = graph.get_context<SceneInfo>();
				scene_info.renderer = g_main_renderer;
				scene_info.scene = g_scene;
				g_scene->update_transforms();
				g_scene->update(*graph.builder.current_frame);
			}
			{
				auto& viewport = graph.get_context<ViewportInfo>();
				viewport.frame_size = ivec2((int)W / 2, (int)H / 2);   // half resolution: the atmosphere is the costly part
				viewport.upscale_size = ivec2((int)W, (int)H);
			}

			g_vsm->update_frame(graph);   // before any pass's setup
			g_ui->create_graph(graph);
			g_sky_pipeline->add_passes(graph);   // before the UI passes: they read its ResultTexture
			g_sky_pipeline->run_pre_setups(graph);
			g_pipeline->add_passes(graph);

			// FrameInfo is what the sky shader reads the camera from.
			graph.add_slot_generator([](FrameGraph::Graph& graph) {
				graph.register_slot_setter(make_frame_info(graph).compile(*graph.builder.current_frame));
			});
			// The scene's tables (nodes, meshes, materials) the geometry stage reads.
			graph.add_slot_generator([](FrameGraph::Graph& graph) {
				graph.register_slot_setter(graph.get_context<SceneInfo>().scene->compiledScene);
			});
			// After every pass exists: per-resource state is sized from the pass count at creation.
			graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, ui_target, {}, FrameGraph::ResourceFlags::Required);
			g_vsm->pass_data(graph.builder);
			// Single-threaded page allocation for every clip level, before VSM_RenderPages records.
			graph.add_slot_generator([](FrameGraph::Graph& graph) { g_vsm->plan_frame(graph); });
			g_pipeline->run_pre_setups(graph);
			tonemap_update_selectors(graph);
			bloom_update_selectors(graph);
			graph.setup();
			graph.compile(frame);
		}

		HAL::FenceWaiter fence;
		{
			PROFILE(L"demo_graph_render");
			graph.render();
			fence = graph.commit_command_lists();
			fence.wait();
			graph.reset();
		}

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"demo_present");
		HAL::Texture2DView view(ui_target, *list);
		auto& gfx = list->get_graphics();

		// FP16 UI target -> SDR swapchain through the engine's CopyTexture PSO (gamma-encodes for 8-bit).
		{
			HAL::Texture2DView sc_view(target, *list);
			HAL::Format sc_format = target->get_desc().as_texture().Format;   // non-const: its conversion operator isn't

			gfx.set_pipeline<PSOS::Utility::CopyTexture>(PSOS::Utility::CopyTexture::Format(sc_format));
			gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			gfx.set_viewport(sc_view.get_viewport());
			gfx.set_scissor(sc_view.get_scissor());

			Slots::Utility::CopyTexture copy;
			copy.GetSrcTex() = view.texture2D;
			gfx.set(copy);

			RT::Frame::SingleColor rt;
			rt.GetColor() = sc_view.renderTarget;
			gfx.set_rtv(rt);
			gfx.draw(4);
		}

		list->execute_and_wait();
		swap.present();
	}
}

export namespace demo
{
	// Layout unit: 1 = 1 dp. A host passes the display's density (1 on a desktop at 96 dpi).
	void set_density(float density)
	{
		g_density = density;
	}

	// Before the first frame: VSM shadows on (default) or off, and the window/graphs drawn over the scene.
	void set_shadows(bool enabled) { g_shadows = enabled; }
	void set_ui_visible(bool visible) { g_ui_visible = visible; }

	// The FP16 target the frame graph renders the screen into; a host recreates it whenever its swapchain does.
	// Sampled by the final copy, hence ShaderResource next to RenderTarget.
	std::shared_ptr<HAL::TextureResource> make_target(HAL::Device& device, ivec2 size)
	{
		return std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT, { (uint)size.x, (uint)size.y }, 1, 1, HAL::ResFlags::RenderTarget | HAL::ResFlags::ShaderResource),
			HAL::HeapType::DEFAULT);
	}

	// One frame into `swap`'s current image. The first call builds the UI and the scene (loading the skin,
	// sky and mesh assets), which takes a moment.
	void draw_frame(HAL::Device& device, HAL::SwapChain& swap, const std::shared_ptr<HAL::TextureResource>& ui_target, int frame)
	{
		draw_ui_frame(device, swap, ui_target, frame);
	}

	// The UI tree: null until the first frame. Hosts forward input to it (touch_event, mouse_*_event_internal).
	std::shared_ptr<GUI::user_interface> ui()
	{
		return current_ui();
	}

	// Drops the per-surface state; the next frame lays the screen out again for the new size.
	void surface_changed()
	{
		g_laid_out_for = ivec2(0, 0);
	}
}
