// Android entry for the UI-first port. Loading libspectrum.so runs JNI_OnLoad, which creates
// the Application singleton and writes log lines (visible with `adb logcat -s Spectrum`).
//
// Deliberately free of <jni.h>: textual system headers can't be mixed with the shared header
// unit the engine modules are built on, and JNI_OnLoad only needs its two opaque pointers.
import Core;
import HAL;
import vulkan;
import RenderSystem;

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

	void draw_clear_frame(int frame)
	{
		g_swap->wait_for_free();

		auto list = g_device->get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"android_clear");
		HAL::Texture2DView view(g_swap->get_current_frame(), *list);

		// Slow hue drift so a frozen image is distinguishable from a live swapchain.
		const float t = frame * 0.02f;
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
		                             vec4(0.1f, 0.25f + 0.2f * std::sin(t), 0.6f + 0.2f * std::cos(t), 1.0f));
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
			desc.format = HAL::Format::R16G16B16A16_FLOAT;
			desc.window = &g_window;
			desc.fullscreen = nullptr;
			desc.stereo = false;
			g_swap = std::make_shared<HAL::SwapChain>(*g_device, desc);
			Log::get() << Log::LEVEL_INFO << "surface: SwapChain created" << Log::endl;

			for (int i = 0; i < 120; ++i)
				draw_clear_frame(i);
			Log::get() << Log::LEVEL_INFO << "surface: 120 clear frames presented" << Log::endl;
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

	// TEMPORARY (throw tracing, linked only with SPECTRUM_TRACE_THROWS=1 -> -Wl,--wrap=__cxa_throw):
	// logs the first throws' backtrace as <module offset> pairs for llvm-symbolizer.
	struct DlInfo { const char* fname; void* fbase; const char* sname; void* saddr; };
	using TraceFn = int (*)(void*, void*);
	extern "C++" int unwind_backtrace(TraceFn fn, void* arg) __asm__("_Unwind_Backtrace");
	extern "C++" unsigned long unwind_get_ip(void* ctx) __asm__("_Unwind_GetIP");
	extern "C++" int libc_dladdr(const void* addr, DlInfo* info) __asm__("dladdr");

	struct TraceState { std::string text; int frames = 0; };

	int trace_frame(void* ctx, void* arg)
	{
		auto& state = *static_cast<TraceState*>(arg);
		const auto ip = unwind_get_ip(ctx);
		DlInfo info{};
		if (libc_dladdr((void*)ip, &info) && info.fbase)
			state.text += " " + hex(ip - (uint64_t)info.fbase);
		return ++state.frames >= 14 ? 5 : 0;   // _URC_END_OF_STACK stops the walk
	}

	extern "C" void __real___cxa_throw(void*, void*, void (*)(void*));
	extern "C" __attribute__((visibility("default"))) void __wrap___cxa_throw(void* object, void* type, void (*destructor)(void*))
	{
		static int logged = 0;
		if (logged++ < 40)
		{
			TraceState state;
			unwind_backtrace(trace_frame, &state);
			Log::get() << Log::LEVEL_ERROR << "TMPTHROW" << state.text << Log::endl;
		}
		__real___cxa_throw(object, type, destructor);
	}

	// TEMPORARY (locale investigation): which std facility throws "locale not supported" here.
	void probe_locale()
	{
		auto attempt = [](const char* what, auto&& fn) {
			try { fn(); Log::get() << Log::LEVEL_INFO << "TMPPROBE " << what << ": ok" << Log::endl; }
			catch (const std::exception& e) { Log::get() << Log::LEVEL_ERROR << "TMPPROBE " << what << ": THREW " << e.what() << Log::endl; }
		};
		attempt("std::locale(\"\")", [] { std::locale l(""); });
		attempt("std::locale(\"C\")", [] { std::locale l("C"); });
		attempt("std::locale()", [] { std::locale l; });
		attempt("convert(string)", [] { auto w = convert(std::string_view("abc")); });
		attempt("path(wstring)", [] { std::filesystem::path p(std::wstring(L"abc")); });
		attempt("path(wstring).string()", [] { std::filesystem::path p(std::wstring(L"abc")); auto s = p.string(); });
		attempt("to_path(string)", [] { auto p = to_path(std::string("cache/vulkan/x.bin")); });
		attempt("to_path(string).string()", [] { auto p = to_path(std::string("cache/vulkan/x.bin")); auto s = p.string(); });
		attempt("filesystem::exists", [] { std::filesystem::exists(to_path(std::string("cache/vulkan"))); });
		attempt("ifstream", [] { std::ifstream f("cache/vulkan/none.bin", std::ios::binary); });
		attempt("ostringstream", [] { std::ostringstream s(std::ios::binary | std::ios::out); s << 1; });
	}

	void boot()
	{
		VSOutputLogger::create();   // Log -> OutputDebugStringA -> logcat (android/shims/windows.ixx)
		Log::get().set_logging_level(Log::LEVEL_ALL);

		Log::get() << Log::LEVEL_INFO << "Spectrum (Android): Core loaded" << Log::endl;
		probe_locale();

		// The working directory is the app's files dir (MainActivity.prepareWorkDir), holding cache/vulkan.
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());

		auto& app = Application::get<AndroidApplication>();
		Log::get() << Log::LEVEL_INFO << "Application singleton created, alive=" << app.is_alive() << Log::endl;
	}
}

extern "C"
{
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
