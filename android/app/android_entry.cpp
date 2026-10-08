// Android entry for the UI-first port. Loading libspectrum.so runs JNI_OnLoad, which creates
// the Application singleton and writes log lines (visible with `adb logcat -s Spectrum`).
//
// Deliberately free of <jni.h>: textual system headers can't be mixed with the shared header
// unit the engine modules are built on, and JNI_OnLoad only needs its two opaque pointers.
import Core;
import HAL;

namespace
{
	// Creates the HAL's Vulkan instance, probes every adapter the way RenderSystem::select_adapter()
	// does on Windows, then builds a real HAL::Device on the first usable one. Everything is logged
	// so `adb logcat -s Spectrum` shows how far the engine's own device path gets on this GPU.
	void probe_gpu()
	{
		Log::get() << Log::LEVEL_INFO << "GPU: HAL::init()" << Log::endl;
		HAL::init();

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

		if (!chosen)
		{
			Log::get() << Log::LEVEL_ERROR << "GPU: no adapter with full bindless support" << Log::endl;
			return;
		}

		try
		{
			Log::get() << Log::LEVEL_INFO << "GPU: creating HAL::Device" << Log::endl;
			HAL::DeviceDesc desc;
			desc.adapter = chosen;
			auto device = std::make_shared<HAL::Device>(desc);
			Log::get() << Log::LEVEL_INFO << "GPU: HAL::Device created OK" << Log::endl;
		}
		catch (const std::exception& e)
		{
			Log::get() << Log::LEVEL_ERROR << "GPU: HAL::Device creation threw: " << e.what() << Log::endl;
		}
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
