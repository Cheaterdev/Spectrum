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
import GUI;
import TextEngine;

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

	// GUI skin textures, baked by `spectrum_tool textures` into cache/vulkan (the device has no image
	// decoders). The header must match the one the engine's Skin asks for: force_srgb = true.
	struct UiTextures
	{
		HAL::Texture::ptr window, button, button_hover, button_pressed;
	} g_tex;
	std::unique_ptr<GUI::NinePatch> g_nine_patch;
	std::shared_ptr<HAL::TextureResource> g_ui_target;   // FP16 UI render target, converted into the swapchain

	HAL::Texture::ptr load_gui_texture(const char* name)
	{
		auto texture = HAL::Texture::get_resource(HAL::texure_header(to_path(std::string("textures/gui/") + name + ".png"), true));
		const bool ok = texture && texture != HAL::Texture::null;
		if (ok)
			Log::get() << Log::LEVEL_INFO << "UI texture " << name << ": loaded" << Log::endl;
		else
			Log::get() << Log::LEVEL_ERROR << "UI texture " << name << ": MISSING" << Log::endl;
		return texture;
	}

	void load_ui_textures()
	{
		g_tex.window = load_gui_texture("window_active");
		g_tex.button = load_gui_texture("button");
		g_tex.button_hover = load_gui_texture("button_hover");
		g_tex.button_pressed = load_gui_texture("button_pressed");
		g_nine_patch = std::make_unique<GUI::NinePatch>();
	}

	// A skin entry as GUI::Skin builds it: the texture plus its nine-patch padding / margins.
	GUI::Texture skin_item(const HAL::Texture::ptr& texture)
	{
		GUI::Texture item;
		if (texture && texture != HAL::Texture::null)
			item = texture->texture_2d();
		return item;
	}

	// Stage 1 of the minimal UI screen: solid rects through the production UI::SimpleRect PSO and
	// Slots::UI::ColorRect (the same path Test.HAL.UI exercises on desktop), laid out in pixels.
	// Stage 2 swaps in textured nine-patches, stage 3 adds a text label.
	void draw_ui_frame(int frame)
	{
		g_swap->wait_for_free();

		auto list = g_device->get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"android_ui");

		auto target = g_swap->get_current_frame();
		const auto& dims = target->get_desc().as_texture().Dimensions;
		const float W = (float)dims.x, H = (float)dims.y;

		// The UI PSOs write R16G16B16A16_FLOAT (scRGB), so the UI renders into an FP16 target and is
		// converted into the 8-bit swapchain at the end (see the CopyTexture pass below).
		HAL::Texture2DView view(g_ui_target, *list);
		auto& gfx = list->get_graphics();

		// Stage 3: text through the real Text::Engine (Skribidi shaping + glyph atlas). Layouts are built
		// (requesting their glyphs) before anything is drawn, and rasterized glyphs are uploaded in this
		// same list ahead of the draws, as the asset preview does. Sizes are scaled by the phone density.
		constexpr float density = 3.0f;
		auto& text = Text::Engine::get();
		text.begin_frame();
		const Text::Style label_style{ 16.0f * density, Text::Weight::Normal, Text::Family::Sans };
		const Text::Style title_style{ 18.0f * density, Text::Weight::Normal, Text::Family::Sans };
		Text::Layout title, labels[3], body;
		text.build("Spectrum \xE2\x80\x94 Android", title_style, title);
		text.build("Play", label_style, labels[0]);
		text.build("Options", label_style, labels[1]);
		text.build("Quit", label_style, labels[2]);
		text.build("Hello \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD1\x96\xD1\x82 \xCE\x93\xCE\xB5\xCE\xB9\xCE\xAC  frame " + std::to_string(frame),
		           label_style, body);   // Latin, Cyrillic, Greek + a changing number
		if (text.has_pending_upload())
			text.upload(list);

		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;
		const vec4 clear = srgb(0.08f, 0.09f, 0.12f);
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, clear);

		Slots::UI::DisplayOutput output;
		output.GetUi_scale() = g_ui_scale;
		gfx.set(output);

		const float margin = W * 0.05f;
		const float panel_x = margin, panel_w = W - 2 * margin, panel_y = H * 0.08f, panel_h = H * 0.42f;
		const float pad = 32.0f;
		const float btn_h = 120.0f, btn_y = panel_y + 120.0f + 2 * pad;
		const float btn_w = (panel_w - 4 * pad) / 3.0f;
		const float bar_y = btn_y + btn_h + 2 * pad, bar_h = 40.0f, bar_w = panel_w - 2 * pad;

		// Textured nine-patches through the real GUI::NinePatch batcher, which needs only this much of
		// a GUI::GUIInfo: the list, the window size, the scale and the clip rect.
		{
			GUIInfo c;
			c.command_list = list;
			c.window_size = vec2(W, H);
			c.scale = 3.0f;   // phone density; scales the patches' padding and margins
			c.ui_clipping.left_top.x = 0;
			c.ui_clipping.left_top.y = 0;
			c.ui_clipping.right_bottom.x = W;
			c.ui_clipping.right_bottom.y = H;

			auto window = skin_item(g_tex.window);
			window.padding = { 11, 40, 13, 13 };   // as Skin::DefaultWindow.Active
			window.margins = { 12, 11, 11, 10 };
			g_nine_patch->draw(c, window, ::rect{ panel_x, panel_y, panel_w, panel_h });

			// The last button cycles through its three skin states so a frozen frame is distinguishable.
			const HAL::Texture::ptr states[3] = { g_tex.button, g_tex.button_hover, g_tex.button_pressed };
			for (int i = 0; i < 3; ++i)
			{
				auto button = skin_item(i == 2 ? states[(frame / 30) % 3] : (i == 1 ? g_tex.button_hover : g_tex.button));
				button.padding = { 3, 3, 3, 3 };
				g_nine_patch->draw(c, button, ::rect{ panel_x + pad + i * (btn_w + pad), btn_y, btn_w, btn_h });
			}
			g_nine_patch->flush(c);
		}

		gfx.set_pipeline<PSOS::UI::SimpleRect>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

		// Pixel rect (origin top-left) -> clip space quad: pos[0] = (TL, TR), pos[1] = (BL, BR).
		auto rect = [&](float x, float y, float w, float h, vec4 color) {
			const float l = 2.0f * x / W - 1.0f, r = 2.0f * (x + w) / W - 1.0f;
			const float t = 1.0f - 2.0f * y / H, b = 1.0f - 2.0f * (y + h) / H;
			Slots::UI::ColorRect slot;
			auto* p = slot.GetPos();
			p[0] = float4(l, t, r, t);
			p[1] = float4(l, b, r, b);
			for (int i = 0; i < 4; ++i)
				slot.GetColor()[i] = float4(color.x, color.y, color.z, color.w);
			gfx.set(slot);
			gfx.draw(4);
		};

		// Progress bar (still a solid rect): track + fill sweeping left to right.
		const float progress = std::fmod(frame * 0.004f, 1.0f);
		rect(panel_x + pad, bar_y, bar_w, bar_h, srgb(0.08f, 0.09f, 0.11f));
		rect(panel_x + pad, bar_y, bar_w * progress, bar_h, srgb(0.95f, 0.65f, 0.15f));

		// Text on top: window title, centred button labels, and a body line under the progress bar.
		{
			const sizer clip = { 0.0f, 0.0f, W, H };
			const vec2 window_size(W, H);
			const float4 white(1.0f, 1.0f, 1.0f, 1.0f);

			text.draw(list, title, vec2(panel_x + 3.0f * pad, panel_y + (120.0f - title.size.y) * 0.5f), white, clip, window_size);
			for (int i = 0; i < 3; ++i)
			{
				const float bx = panel_x + pad + i * (btn_w + pad);
				text.draw(list, labels[i], vec2(bx + (btn_w - labels[i].size.x) * 0.5f, btn_y + (btn_h - labels[i].size.y) * 0.5f), white, clip, window_size);
			}
			text.draw(list, body, vec2(panel_x + pad, bar_y + bar_h + pad), float4(0.85f, 0.88f, 0.95f, 1.0f), clip, window_size);
		}

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

	void on_surface(void* env, void* surface, int width, int height)
	{
		if (!g_device)
		{
			Log::get() << Log::LEVEL_ERROR << "surface: no HAL::Device" << Log::endl;
			return;
		}
		try
		{
			g_window.window = surface_to_window(env, surface);
			Log::get() << Log::LEVEL_INFO << "surface: ANativeWindow=" << hex((uint64_t)g_window.window)
			           << " size=" << width << "x" << height << Log::endl;

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
			load_ui_textures();

			// Keep drawing for five minutes: long enough to look at on the phone. There is no
			// surfaceDestroyed handling yet, so the loop ends by time rather than by lifecycle.
			const auto start = std::chrono::steady_clock::now();
			int frame = 0;
			while (std::chrono::steady_clock::now() - start < std::chrono::minutes(5))
			{
				draw_ui_frame(frame++);
				if (frame == 1 || frame == 120)
					Log::get() << Log::LEVEL_INFO << "surface: UI frame " << frame << " presented" << Log::endl;
			}
			Log::get() << Log::LEVEL_INFO << "surface: " << frame << " UI frames presented" << Log::endl;
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

	// MainActivity.nativeSurface(Surface, w, h): surfaceChanged from the SurfaceView.
	__attribute__((visibility("default"))) void Java_com_spectrum_MainActivity_nativeSurface(void* env, void*, void* surface, int w, int h)
	{
		on_surface(env, surface, w, h);
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
