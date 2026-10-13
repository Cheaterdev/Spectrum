// Android entry for the UI-first port. Loading libspectrum.so runs JNI_OnLoad, which creates
// the Application singleton and writes log lines (visible with `adb logcat -s Spectrum`).
//
// Deliberately free of <jni.h>: textual system headers can't be mixed with the shared header
// unit the engine modules are built on, and JNI_OnLoad only needs its two opaque pointers.
import Core;
import HAL;
import vulkan;
import RenderSystem;
import Graphics;
import FrameGraph;
import GUI;
import TextEngine;
import Test.Framework;

namespace
{
	std::string hex(uint64_t v)
	{
		static const char* digits = "0123456789abcdef";
		std::string s;
		do { s.insert(s.begin(), digits[v & 15]); v >>= 4; } while (v);
		return "0x" + s;
	}

	// Diagnostic for sparse ("virtual") resources: the HAL needs ONE memory type that every sparse
	// resource kind accepts. This builds a throwaway VkDevice and prints, per kind, whether sparse is
	// supported at all and which memory types it allows, then the intersection and the memory types.
	void probe_sparse(VkPhysicalDevice pd)
	{
		VkPhysicalDeviceFeatures supported{};
		vkGetPhysicalDeviceFeatures(pd, &supported);
		Log::get() << Log::LEVEL_INFO << "SPARSE features: binding=" << (int)supported.sparseBinding
		           << " residencyBuffer=" << (int)supported.sparseResidencyBuffer
		           << " residencyImage2D=" << (int)supported.sparseResidencyImage2D
		           << " residencyImage3D=" << (int)supported.sparseResidencyImage3D
		           << " aliased=" << (int)supported.sparseResidencyAliased << Log::endl;

		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, nullptr);
		std::vector<VkQueueFamilyProperties> families(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, families.data());
		uint32_t family = ~0u;
		for (uint32_t i = 0; i < family_count; ++i)
		{
			Log::get() << Log::LEVEL_INFO << "SPARSE queue family " << i << " flags=" << hex(families[i].queueFlags)
			           << " count=" << families[i].queueCount << Log::endl;
			if (family == ~0u && (families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT))
				family = i;
		}
		if (family == ~0u)
		{
			Log::get() << Log::LEVEL_ERROR << "SPARSE: no queue family with sparse binding" << Log::endl;
			return;
		}

		VkPhysicalDeviceMemoryProperties mem{};
		vkGetPhysicalDeviceMemoryProperties(pd, &mem);
		for (uint32_t i = 0; i < mem.memoryTypeCount; ++i)
			Log::get() << Log::LEVEL_INFO << "SPARSE memory type " << i << " heap=" << mem.memoryTypes[i].heapIndex
			           << " flags=" << hex(mem.memoryTypes[i].propertyFlags) << Log::endl;

		float priority = 1.0f;
		VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		qci.queueFamilyIndex = family;
		qci.queueCount = 1;
		qci.pQueuePriorities = &priority;
		VkPhysicalDeviceFeatures enable{};
		enable.sparseBinding = supported.sparseBinding;
		enable.sparseResidencyBuffer = supported.sparseResidencyBuffer;
		enable.sparseResidencyImage2D = supported.sparseResidencyImage2D;
		enable.sparseResidencyImage3D = supported.sparseResidencyImage3D;
		VkDeviceCreateInfo dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		dci.queueCreateInfoCount = 1;
		dci.pQueueCreateInfos = &qci;
		dci.pEnabledFeatures = &enable;
		VkDevice device = VK_NULL_HANDLE;
		if (vkCreateDevice(pd, &dci, nullptr, &device) != VK_SUCCESS)
		{
			Log::get() << Log::LEVEL_ERROR << "SPARSE: probe vkCreateDevice failed" << Log::endl;
			return;
		}

		uint32_t intersection = ~0u;
		auto report = [&](const char* what, bool created, const VkMemoryRequirements& req) {
			Log::get() << Log::LEVEL_INFO << "SPARSE " << what << ": " << (created ? "created" : "NOT supported")
			           << " alignment=" << req.alignment << " memoryTypeBits=" << hex(req.memoryTypeBits) << Log::endl;
			if (created)
				intersection &= req.memoryTypeBits;
		};

		{
			VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
			bci.size = 64 * 1024;
			bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			VkBuffer buffer = VK_NULL_HANDLE;
			VkMemoryRequirements req{};
			const bool ok = vkCreateBuffer(device, &bci, nullptr, &buffer) == VK_SUCCESS;
			if (ok) { vkGetBufferMemoryRequirements(device, buffer, &req); vkDestroyBuffer(device, buffer, nullptr); }
			report("buffer", ok, req);
		}

		struct Kind { const char* name; VkImageType type; VkFormat format; VkImageUsageFlags usage; };
		const Kind kinds[] = {
			{ "2D R8G8B8A8_UNORM storage", VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT },
			{ "2D D32_SFLOAT depth", VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT },
			{ "2D D16_UNORM depth", VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT },
			{ "2D R32_UINT storage", VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT },
			{ "3D R16G16B16A16_SFLOAT storage", VK_IMAGE_TYPE_3D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT },
		};
		for (const Kind& k : kinds)
		{
			VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			ici.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
			ici.imageType = k.type;
			ici.format = k.format;
			ici.extent = { 256, 256, k.type == VK_IMAGE_TYPE_3D ? 64u : 1u };
			ici.mipLevels = 1;
			ici.arrayLayers = 1;
			ici.samples = VK_SAMPLE_COUNT_1_BIT;
			ici.usage = k.usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			uint32_t props = 0;
			vkGetPhysicalDeviceSparseImageFormatProperties(pd, k.format, k.type, VK_SAMPLE_COUNT_1_BIT, ici.usage,
			                                               VK_IMAGE_TILING_OPTIMAL, &props, nullptr);
			VkImage image = VK_NULL_HANDLE;
			VkMemoryRequirements req{};
			const bool ok = props && vkCreateImage(device, &ici, nullptr, &image) == VK_SUCCESS;
			if (ok) { vkGetImageMemoryRequirements(device, image, &req); vkDestroyImage(device, image, nullptr); }
			Log::get() << Log::LEVEL_INFO << "SPARSE format-properties count for " << k.name << ": " << props << Log::endl;
			report(k.name, ok, req);
		}

		Log::get() << Log::LEVEL_INFO << "SPARSE intersection of all supported kinds: " << hex(intersection)
		           << (intersection ? "" : "  (EMPTY: this is the engine's failure)") << Log::endl;
		vkDestroyDevice(device, nullptr);
	}

	HAL::Device* g_device = nullptr;   // owned by RenderSystem
	std::shared_ptr<HAL::SwapChain> g_swap;

	// The Activity's SurfaceView surface as an ANativeWindow*; get_hwnd() is what the Vulkan
	// swapchain turns into a VkAndroidSurfaceKHR.
	struct AndroidWindow : HAL::hwnd_provider
	{
		void* window = nullptr;
		void* get_hwnd() const override { return window; }   // HWND is void* on Android
	} g_window;

	// jni.h is deliberately not included; JNIEnv*/jobject cross as void*. The asm label keeps this
	// from clashing with the NDK declaration if a header module ever exposes it.
	extern "C++" void* surface_to_window(void* env, void* surface) __asm__("ANativeWindow_fromSurface");

	// Same trick for the two libc calls: no <stdlib.h>/<unistd.h> next to the header unit.
	extern "C++" const char* libc_getenv(const char* name) __asm__("getenv");
	extern "C++" int libc_chdir(const char* path) __asm__("chdir");

	// DisplayOutput.ui_scale: where SDR white lands in the scRGB swapchain. The desktop engine feeds
	// sdr_white_nits / 80; Android has no such query here, so MainActivity passes a measured value
	// (intent extra "ui_scale", exported as SPECTRUM_UI_SCALE).
	float g_ui_scale = 1.0f;

	void read_ui_scale()
	{
		if (const char* v = libc_getenv("SPECTRUM_UI_SCALE"))
			g_ui_scale = std::stof(v);
		Log::get() << Log::LEVEL_INFO << "ui_scale = " << std::to_string(g_ui_scale) << Log::endl;
	}

	// The swapchain is scRGB (linear), the UI colors below are written as sRGB.
	float srgb_to_linear(float c)
	{
		return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
	}

	vec4 srgb(float r, float g, float b, float a = 1.0f)
	{
		return vec4(srgb_to_linear(r), srgb_to_linear(g), srgb_to_linear(b), a);
	}

	std::shared_ptr<HAL::TextureResource> g_ui_target;   // FP16 UI render target, converted into the swapchain

	// Layout unit: 1 = 1 dp. MainActivity exports DisplayMetrics.density (2.6 on the test phone) as
	// SPECTRUM_DENSITY; the old NDK GUI derived its scale from DPI (max(1, DPI / 260)) instead.
	float g_density = 2.0f;

	void read_density()
	{
		if (const char* v = libc_getenv("SPECTRUM_DENSITY"))
			g_density = std::stof(v);
		Log::get() << Log::LEVEL_INFO << "density = " << std::to_string(g_density) << Log::endl;
	}

	std::mutex g_ui_mutex;   // the touch thread reads g_ui while the render thread creates it
	std::shared_ptr<GUI::user_interface> g_ui;

	std::shared_ptr<GUI::user_interface> current_ui()
	{
		std::lock_guard<std::mutex> lock(g_ui_mutex);
		return g_ui;
	}

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

	void ensure_ui()
	{
		std::lock_guard<std::mutex> lock(g_ui_mutex);
		if (g_ui)
			return;

		THREAD_SCOPE(GUI);
		Skin::get();   // loads every skin texture now rather than inside the first frame
		g_ui = std::make_shared<GUI::user_interface>();   // shared-owned: the touch path walks to parents via shared_from_this

		// Hooks the desktop window provides; set_capture is called by draggable elements (the window title).
		g_ui->set_capture = [](bool) {};
		g_ui->get_clipboard = [] { return std::wstring(); };
		g_ui->set_clipboard = [](std::wstring_view) {};

		g_root = std::make_shared<GUI::base>();
		g_root->docking = GUI::dock::NONE;
		g_root->scale = g_density;
		g_ui->add_child(g_root);

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
	std::mutex g_touch_mutex;
	GUI::touch_event g_touch_pending;

	std::unique_ptr<FrameGraph::Graph> g_graph;
	std::unique_ptr<Pipelines::UI::UIPipeline> g_pipeline;

	// One UI frame through the FrameGraph, as the desktop app does it: the widget tree is walked into
	// the UI context (create_graph), UIPipeline's UI_PreDraw / UI_Render passes are added, and the graph
	// draws into g_ui_target in place of the swapchain. That target is FP16 because the UI PSOs write
	// FP16; the SDR swapchain gets it through the CopyTexture pass at the end.
	void draw_ui_frame(int frame)
	{
		g_swap->wait_for_free();
		ensure_ui();

		if (!g_graph)
		{
			g_graph = std::make_unique<FrameGraph::Graph>();
			g_pipeline = std::make_unique<Pipelines::UI::UIPipeline>();
		}
		auto& graph = *g_graph;

		auto target = g_swap->get_current_frame();
		const auto& dims = target->get_desc().as_texture().Dimensions;
		const float W = (float)dims.x, H = (float)dims.y;

		static auto last = std::chrono::steady_clock::now();
		const auto now = std::chrono::steady_clock::now();
		const float dt = std::chrono::duration<float>(now - last).count();
		last = now;

		{
			PROFILE(L"android_ui_update");
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
			PROFILE(L"android_graph_build");
			graph.start_new_frame();
			graph.get_context<TimeInfo>().frame = frame;

			auto& display = graph.get_context<Table::UI::DisplayState>();
			display.hdr = false;
			display.max_nits = 80.0f;
			display.sdr_white_nits = 80.0f;

			g_ui->create_graph(graph);
			g_pipeline->add_passes(graph);
			// After every pass exists: per-resource state is sized from the pass count at creation.
			graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, g_ui_target, {}, FrameGraph::ResourceFlags::Required);
			g_pipeline->run_pre_setups(graph);
			graph.setup();
			graph.compile(frame);
		}

		HAL::FenceWaiter fence;
		{
			PROFILE(L"android_graph_render");
			graph.render();
			fence = graph.commit_command_lists();
			fence.wait();
			graph.reset();
		}

		auto list = g_device->get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"android_present");
		HAL::Texture2DView view(g_ui_target, *list);
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
		g_swap->present();
	}


	// Bumped by MainActivity on every surface change (resize, rotation, destroy); a render loop belongs to
	// one generation and ends as soon as it is no longer the latest.
	std::atomic<int> g_latest_generation{ 0 };

	extern "C++" void native_window_release(void* window) __asm__("ANativeWindow_release");
	extern "C++" int native_window_width(void* window) __asm__("ANativeWindow_getWidth");
	extern "C++" int native_window_height(void* window) __asm__("ANativeWindow_getHeight");

	// Everything built for one surface, torn down in dependency order when the loop ends: the GPU is
	// idled first so no frame still references the swapchain images or the UI target.
	void render_surface(void* env, void* surface, int generation)
	{
		if (!g_device)
		{
			Log::get() << Log::LEVEL_ERROR << "surface: no HAL::Device" << Log::endl;
			return;
		}

		struct Cleanup
		{
			~Cleanup()
			{
				try
				{
					g_device->wait_idle();
					g_ui_target.reset();
					g_swap.reset();
				}
				catch (const std::exception& e)
				{
					Log::get() << Log::LEVEL_ERROR << "surface: cleanup threw: " << e.what() << Log::endl;
				}
				if (g_window.window)
					native_window_release(g_window.window);
				g_window.window = nullptr;
				Log::get() << Log::LEVEL_INFO << "surface: released" << Log::endl;
			}
		} cleanup;

		try
		{
			g_window.window = surface_to_window(env, surface);
			Log::get() << Log::LEVEL_INFO << "surface: generation " << generation << " ANativeWindow=" << hex((uint64_t)g_window.window)
			           << " size=" << native_window_width(g_window.window) << "x" << native_window_height(g_window.window) << Log::endl;

			HAL::swap_chain_desc desc{};
			desc.format = HAL::Format::R8G8B8A8_UNORM;   // SDR surface; the UI is rendered to FP16 and converted
			desc.window = &g_window;
			desc.fullscreen = nullptr;
			desc.stereo = false;
			g_swap = std::make_shared<HAL::SwapChain>(*g_device, desc);
			Log::get() << Log::LEVEL_INFO << "surface: SwapChain created" << Log::endl;

			{
				const auto& dims = g_swap->get_current_frame()->get_desc().as_texture().Dimensions;
				g_ui_target = std::make_shared<HAL::TextureResource>(*g_device,
					HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT, { dims.x, dims.y }, 1, 1, HAL::ResFlags::RenderTarget | HAL::ResFlags::ShaderResource),   // sampled by the conversion pass
					HAL::HeapType::DEFAULT);
			}
			read_ui_scale();
			read_density();

			int frame = 0;
			while (g_latest_generation.load() == generation)
			{
				draw_ui_frame(frame++);
				if (frame == 1 || frame == 120)
					Log::get() << Log::LEVEL_INFO << "surface: UI frame " << frame << " presented" << Log::endl;
			}
			Log::get() << Log::LEVEL_INFO << "surface: generation " << generation << " ended after " << frame << " frames" << Log::endl;
		}
		catch (const std::exception& e)
		{
			Log::get() << Log::LEVEL_ERROR << "surface: threw: " << e.what() << Log::endl;
		}
	}

	// Brings the engine's RenderSystem up exactly as the desktop app does (adapter selection, HAL::Device,
	// init_managers -> every engine PSO from the precompiled shader cache), then logs the adapters and the
	// sparse-resource probe for diagnosis. `adb logcat -s Spectrum` shows how far it gets on this GPU.
	void probe_gpu()
	{
		try
		{
			Log::get() << Log::LEVEL_INFO << "GPU: RenderSystem::create()" << Log::endl;
			if (!RenderSystem::create())
			{
				Log::get() << Log::LEVEL_ERROR << "GPU: RenderSystem::create() found no usable adapter" << Log::endl;
				return;
			}
			g_device = &RenderSystem::get().device();
			Log::get() << Log::LEVEL_INFO << "GPU: RenderSystem ready" << Log::endl;

			// GetPSO asserts on a null entry (DEV), so a missing UI pipeline shows up as a trap here.
			auto& psos = g_device->get_engine_pso_holder();
			auto report = [](const char* name, auto pso) {
				Log::get() << Log::LEVEL_INFO << "UI PSO " << name << ": " << (pso ? "ok" : "MISSING") << Log::endl;
			};
			report("NinePatch", psos.GetPSO<PSOS::UI::NinePatch>());
			report("SimpleRect", psos.GetPSO<PSOS::UI::SimpleRect>());
			report("CanvasBack", psos.GetPSO<PSOS::UI::CanvasBack>());
			report("CanvasLines", psos.GetPSO<PSOS::UI::CanvasLines>());
			report("StatGraphLines", psos.GetPSO<PSOS::Dev::StatGraphLines>());
		}
		catch (const std::exception& e)
		{
			Log::get() << Log::LEVEL_ERROR << "GPU: RenderSystem::create() threw: " << e.what() << Log::endl;
			return;
		}

		HAL::Adapter::ptr chosen;
		int index = 0;
		HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter) {
			const auto p = HAL::Device::probe(adapter);
			Log::get() << Log::LEVEL_INFO << "GPU adapter " << index++ << ": " << p.name
			           << " | full_bindless=" << p.full_bindless << " mesh_shader=" << p.mesh_shader
			           << " ray_query=" << p.ray_query << " rtx_pipeline=" << p.rtx
			           << " tiled_3d=" << p.tiled_3d << " tiled_depth=" << p.tiled_depth
			           << " direct_gpu_upload_heap=" << p.direct_gpu_upload_heap
			           << " min_storage_buffer_offset_alignment=" << p.min_storage_buffer_offset_alignment << Log::endl;
			if (!chosen && p.full_bindless)
				chosen = adapter;
		});

		if (chosen)
			probe_sparse(chosen->get_vk_physical());
	}

	class AndroidApplication : public Application
	{
		friend class Singleton<Application>;
		AndroidApplication() = default;

		int ticks = 0;

	protected:
		void on_tick() override
		{
			Log::get() << Log::LEVEL_INFO << "Application::on_tick " << ticks << Log::endl;
			if (++ticks >= 3)
				shutdown();
		}
	};

	void boot()
	{
		VSOutputLogger::create();   // Log -> OutputDebugStringA -> logcat (android/shims/windows.ixx)
		Log::get().set_logging_level(Log::LEVEL_ALL);

		Log::get() << Log::LEVEL_INFO << "Spectrum (Android): Core loaded" << Log::endl;

		// The working directory is the app's files dir (MainActivity.prepareWorkDir), holding cache/vulkan.
		// Not when one is already there: the linked test modules register their own at load time, and two
		// native providers make FileSystem::iterate visit every file twice.
		if (!FileSystem::get().has_providers())
			FileSystem::get().register_provider(std::make_shared<native_file_provider>());

		auto& app = Application::get<AndroidApplication>();
		Log::get() << Log::LEVEL_INFO << "Application singleton created, alive=" << app.is_alive() << Log::endl;
	}
}

extern "C"
{
	// Target of the generated stubs (android/build.py, unported_stubs.s): code this port doesn't compile
	// yet is linked as a stub that lands here with its demangled name instead of failing dlopen.
	__attribute__((visibility("default"))) void spectrum_unported_called(const char* name)
	{
		Log::get() << Log::LEVEL_ERROR << "UNPORTED function called: " << name << Log::endl;
		__builtin_trap();
	}

	// JNI_VERSION_1_6; the VM and reserved pointers are unused.
	__attribute__((visibility("default"))) int JNI_OnLoad(void*, void*)
	{
		boot();
		return 0x00010006;
	}

	// MainActivity.nativeProbeGpu(): Vulkan instance + adapter probe + HAL::Device (see probe_gpu).
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeProbeGpu(void*, void*)
	{
		probe_gpu();
	}

	// MainActivity.nativeRenderSurface(Surface, generation): renders until the generation moves on.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeRenderSurface(void* env, void*, void* surface, int generation)
	{
		render_surface(env, surface, generation);
	}

	// MainActivity.nativeRunTests(): runs the CPU tests linked into this library (the `tests` target of
	// android/build.py) instead of the UI. The filter comes from SPECTRUM_TEST_FILTER (intent extra
	// "tests"; empty = all). Results go to logcat: `adb logcat -s Spectrum | grep -E "PASS|FAIL|TESTS"`.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeRunTests(void*, void*)
	{
		const char* filter = libc_getenv("SPECTRUM_TEST_FILTER");
		Application::test_mode = true;

		auto& registry = Test::TestRegistry::Instance();
		auto results = registry.RunAll(filter ? filter : "");
		registry.PrintResults(results);

		size_t passed = 0, failed = 0, skipped = 0;
		for (auto& r : results)
		{
			if (r.skipped)
				++skipped;
			else if (r.passed)
				++passed;
			else
				++failed;
		}
		Log::get() << Log::LEVEL_INFO << "TESTS: " << std::to_string(results.size()) << " total, " << std::to_string(passed)
		           << " passed, " << std::to_string(failed) << " failed, " << std::to_string(skipped) << " skipped" << Log::endl;
	}

	// MainActivity.nativeTouchPoint / nativeTouchFlush: Android's touch, as one GUI::touch_event per
	// MotionEvent. It goes straight to user_interface::on_touch_event, never through the mouse handlers.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeTouchPoint(void*, void*, int id, float x, float y)
	{
		std::lock_guard<std::mutex> lock(g_touch_mutex);
		g_touch_pending.touches.push_back({ id, x, y });
	}

	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeTouchFlush(void*, void*, int flags)
	{
		GUI::touch_event event;
		{
			std::lock_guard<std::mutex> lock(g_touch_mutex);
			event = std::move(g_touch_pending);
			g_touch_pending = {};
		}
		event.flags = flags;

		if (auto ui = current_ui())
			ui->on_touch_event(event);
	}

	// MainActivity.nativeSetGeneration(generation): called on every surface change / destroy.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeSetGeneration(void*, void*, int generation)
	{
		g_latest_generation.store(generation);
	}

	// MainActivity.nativeEnterWorkDir(): the engine's relative cache/ paths resolve against the app's
	// files dir, which Java exports as SPECTRUM_WORKDIR (Os.chdir isn't public API).
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeEnterWorkDir(void*, void*)
	{
		const char* dir = libc_getenv("SPECTRUM_WORKDIR");
		const int r = dir ? libc_chdir(dir) : -1;
		Log::get() << Log::LEVEL_INFO << "workdir: " << (dir ? dir : "(unset)") << (r == 0 ? " ok" : " FAILED") << Log::endl;
	}

	// MainActivity.nativeRun(): the Java side of the same smoke test.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeRun(void*, void*)
	{
		Application::get<AndroidApplication>().run();
		Log::get() << Log::LEVEL_INFO << "Application::run finished (via MainActivity)" << Log::endl;
	}

	// Runs the application loop to completion (three ticks, then shutdown): a smoke test that
	// can be called from native code without the Activity.
	__attribute__((visibility("default"))) void spectrum_run()
	{
		Application::get<AndroidApplication>().run();
		Log::get() << Log::LEVEL_INFO << "Application::run finished" << Log::endl;
	}
}
