module;

#include <Windows.h>

module HAL:Streamline;

import Core;
import :Utils;
import streamline;
import :API.Streamline;
import :API.CommandList;

namespace nvidia
{
	namespace
	{
		// Streamline's runtime is deployed to a subdirectory next to the exe
		// (Sharpmake: TargetCopyFilesToSubDirectory). Windows would not find it
		// via the default search path, which is exactly why we load it by
		// absolute path instead of linking the import library.
		constexpr const wchar_t* kPluginSubDir = L"Streamline";

		std::wstring exe_relative(const wchar_t* sub_dir)
		{
			wchar_t path[MAX_PATH]{};
			const DWORD n = ::GetModuleFileNameW(nullptr, path, MAX_PATH);
			if (n == 0 || n >= MAX_PATH) return {};

			std::wstring dir(path, n);
			const auto slash = dir.find_last_of(L"\\/");
			if (slash == std::wstring::npos) return {};

			dir.resize(slash + 1);
			dir += sub_dir;
			return dir;
		}

		sl::Feature to_sl(Feature f)
		{
			switch (f)
			{
			case Feature::RayReconstruction: return sl::kFeatureDLSS_RR;
			case Feature::Common:            return sl::kFeatureCommon;
			case Feature::DLSS:
			default:                         return sl::kFeatureDLSS;
			}
		}

		sl::BufferType to_sl(BufferType t)
		{
			switch (t)
			{
			case BufferType::Depth:          return sl::kBufferTypeDepth;
			case BufferType::MotionVectors:  return sl::kBufferTypeMotionVectors;
			case BufferType::ColorOut:       return sl::kBufferTypeScalingOutputColor;
			case BufferType::ColorIn:
			default:                         return sl::kBufferTypeScalingInputColor;
			}
		}

		sl::float4x4 to_sl(const mat4x4& m)
		{
			sl::float4x4 out;
			for (uint32_t i = 0; i < 4; ++i)
				out.setRow(i, sl::float4(m[i][0], m[i][1], m[i][2], m[i][3]));
			return out;
		}

		void log_callback(sl::LogType type, const char* msg)
		{
			if (!msg) return;
			Log::get() << (std::string("[Streamline] ") + msg) << Log::endl;
		}
	}

	Streamline::Streamline()
	{
		const std::wstring plugin_dir = exe_relative(kPluginSubDir);
		if (plugin_dir.empty()) return;

		const std::wstring interposer = plugin_dir + L"\\sl.interposer.dll";

		// LOAD_WITH_ALTERED_SEARCH_PATH so sl.interposer resolves its own
		// siblings (sl.common, sl.dlss, ...) out of the same directory.
		auto* module_handle = ::LoadLibraryExW(interposer.c_str(), nullptr,
		                                       LOAD_WITH_ALTERED_SEARCH_PATH);
		if (!module_handle)
		{
			Log::get() << "[Streamline] sl.interposer.dll not found - DLSS disabled" << Log::endl;
			return;
		}
		library = module_handle;

		const auto resolve = [module_handle](auto& fn, const char* name)
		{
			// All Streamline entry points are extern "C", so no name mangling.
			fn = reinterpret_cast<std::remove_reference_t<decltype(fn)>>(
			         ::GetProcAddress(module_handle, name));
			return fn != nullptr;
		};

		if (!resolve(fn_init,                 "slInit") ||
		    !resolve(fn_shutdown,             "slShutdown") ||
		    !resolve(fn_is_feature_supported, "slIsFeatureSupported") ||
		    !resolve(fn_get_feature_function, "slGetFeatureFunction") ||
		    !resolve(fn_get_new_frame_token,  "slGetNewFrameToken") ||
		    !resolve(fn_set_constants,        "slSetConstants") ||
		    !resolve(fn_set_tag_for_frame,    "slSetTagForFrame") ||
		    !resolve(fn_evaluate_feature,     "slEvaluateFeature") ||
		    !resolve(fn_allocate_resources,   "slAllocateResources") ||
		    !resolve(fn_upgrade_interface,    "slUpgradeInterface") ||
		    !resolve_api(module_handle))   // backend half (slSetD3DDevice / slSetVulkanInfo)
		{
			Log::get() << "[Streamline] entry points missing - DLSS disabled" << Log::endl;
			return;
		}

		const wchar_t* paths[] = { plugin_dir.c_str() };
		const sl::Feature features[] = { sl::kFeatureDLSS, sl::kFeatureDLSS_RR };

		sl::Preferences pref{};
		pref.pathsToPlugins      = paths;
		pref.numPathsToPlugins   = 1;
		pref.featuresToLoad      = features;
		pref.numFeaturesToLoad   = _countof(features);
		pref.logMessageCallback  = &log_callback;
		pref.logLevel            = BuildOptions::Dev ? sl::LogLevel::eDefault : sl::LogLevel::eOff;
		pref.renderAPI           = sl::RenderAPI::eD3D12;
		pref.engine              = sl::EngineType::eCustom;
		pref.engineVersion       = "1.0";

		// NGX identifies the application by projectId when no NVIDIA-issued
		// applicationId is set. Leaving it empty makes getNGXFeatureRequirements
		// fail with 0xbad00005 (NVSDK_NGX_Result_FAIL_InvalidParameter), which
		// costs DLSS its min-spec query and drops DLSS-RR entirely.
		pref.projectId           = "b7e4f1a2-9c3d-4e58-8f10-2a6b5d9c7e34";

		// Manual hooking: we create the device/swapchain ourselves and hand the
		// device over via slSetD3DDevice rather than letting the interposer
		// proxy D3D12CreateDevice/CreateDXGIFactory2. OTA is deliberately not
		// enabled (non-reproducible builds).
		pref.flags = sl::PreferenceFlags::eUseManualHooking
		           | sl::PreferenceFlags::eDisableCLStateTracking
		           | sl::PreferenceFlags::eUseFrameBasedResourceTagging;

		// Logging to file is off; messages come back through logMessageCallback
		// into the engine log instead.
		pref.pathToLogsAndData = nullptr;

		const sl::Result res = fn_init(pref, sl::kSDKVersion);
		if (res != sl::Result::eOk)
		{
			Log::get() << (std::string("[Streamline] slInit failed (")
			              + std::to_string((int)res) + ") - DLSS disabled") << Log::endl;
			return;
		}

		initialized = true;
		Log::get() << "[Streamline] initialized" << Log::endl;
	}

	Streamline::~Streamline()
	{
		if (initialized && fn_shutdown)
			fn_shutdown();

		if (library)
			::FreeLibrary(static_cast<HMODULE>(library));

		library     = nullptr;
		initialized = false;
	}

	bool Streamline::supports(Feature feature, const void* luid, uint luid_size) const
	{
		if (!initialized || !luid || luid_size == 0) return false;

		sl::AdapterInfo info{};
		info.deviceLUID            = static_cast<uint8_t*>(const_cast<void*>(luid));
		info.deviceLUIDSizeInBytes = luid_size;

		return fn_is_feature_supported(to_sl(feature), info) == sl::Result::eOk;
	}

	void Streamline::bind_device(D3D::Device native_device)
	{
		if (!initialized || !native_device) return;

		// Backend half does the actual SL call and logs the specific failure.
		// A device we could not hand over means nothing downstream can work, so
		// disable the whole integration rather than leave it half-initialised.
		if (!bind_device_native(native_device))
			initialized = false;
	}

	bool Streamline::get_feature_function(Feature feature, const char* name, void*& fn) const
	{
		if (!initialized || !name) return false;

		return fn_get_feature_function(to_sl(feature), name, fn) == sl::Result::eOk;
	}

	FrameToken Streamline::begin_frame()
	{
		FrameToken result;
		if (!initialized) { cached_frame = result; return result; }

		// PFun_slGetNewFrameToken is a bare function-type alias, so the default
		// argument on the original slGetNewFrameToken declaration does not carry
		// over through the pointer — frameIndex must be passed explicitly.
		sl::FrameToken* token = nullptr;
		if (fn_get_new_frame_token(token, nullptr) == sl::Result::eOk)
			result.token = token;

		cached_frame = result;
		return result;
	}

	void Streamline::set_constants(const FrameToken& frame, const FrameConstants& constants, uint32_t viewport) const
	{
		if (!initialized || !frame.valid()) return;

		sl::Constants sl_constants{};
		sl_constants.cameraViewToClip = to_sl(constants.view_to_clip);
		sl_constants.clipToCameraView = to_sl(constants.clip_to_view);
		sl_constants.clipToPrevClip   = to_sl(constants.clip_to_prev_clip);

		{
			mat4x4 prev_clip_to_clip = constants.clip_to_prev_clip;
			prev_clip_to_clip.inverse();
			sl_constants.prevClipToClip = to_sl(prev_clip_to_clip);
		}

		sl_constants.jitterOffset = { constants.jitter_offset.x, constants.jitter_offset.y };
		sl_constants.mvecScale    = { constants.mvec_scale.x,    constants.mvec_scale.y };
		sl_constants.cameraPos    = { constants.camera_pos.x,   constants.camera_pos.y,   constants.camera_pos.z };
		sl_constants.cameraUp     = { constants.camera_up.x,    constants.camera_up.y,    constants.camera_up.z };
		sl_constants.cameraRight  = { constants.camera_right.x, constants.camera_right.y, constants.camera_right.z };
		sl_constants.cameraFwd    = { constants.camera_fwd.x,   constants.camera_fwd.y,   constants.camera_fwd.z };

		// Spectrum uses reversed-Z (see [[project-reversed-z-and-occlusion]]) —
		// this one is a known project fact, not a pending-audit guess.
		sl_constants.depthInverted = sl::Boolean::eTrue;

		// PENDING AUDIT (see FrameConstants' doc comment): these five assume
		// GBuffer_Speed is 2D, screen-space, unjittered, undilated pixel motion
		// with no camera motion baked in. Wrong in any of these ways = visible
		// ghosting/smearing, not a crash.
		sl_constants.cameraMotionIncluded  = sl::Boolean::eFalse;
		sl_constants.motionVectors3D       = sl::Boolean::eFalse;
		sl_constants.motionVectorsJittered = sl::Boolean::eFalse;
		sl_constants.motionVectorsDilated  = sl::Boolean::eFalse;
		sl_constants.orthographicProjection = sl::Boolean::eFalse;

		sl_constants.reset = constants.reset ? sl::Boolean::eTrue : sl::Boolean::eFalse;

		const sl::Result res = fn_set_constants(sl_constants, *static_cast<sl::FrameToken*>(frame.token), sl::ViewportHandle(viewport));
		if (res != sl::Result::eOk)
			Log::get() << (std::string("[Streamline] set_constants failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
	}

	void Streamline::set_tag(const FrameToken& frame, BufferType type, void* native_resource,
	                          uint32_t d3d12_state, HAL::API::CommandList* native_command_list, uint32_t viewport) const
	{
		if (!initialized || !frame.valid() || !native_resource) return;

		sl::Resource resource(sl::ResourceType::eTex2d, native_resource, d3d12_state);
		sl::ResourceTag tag(&resource, to_sl(type), sl::ResourceLifecycle::eValidUntilEvaluate);

		void* native_list = native_command_list ? native_command_list->get_native_handle() : nullptr;
		const sl::Result res = fn_set_tag_for_frame(*static_cast<sl::FrameToken*>(frame.token), sl::ViewportHandle(viewport), &tag, 1, native_list);
		if (res != sl::Result::eOk)
			Log::get() << (std::string("[Streamline] set_tag(") + std::to_string((int)type)
			              + std::string(") failed (") + std::to_string((int)res) + ")") << Log::endl;
	}

	bool Streamline::evaluate(Feature feature, const FrameToken& frame, HAL::API::CommandList* native_command_list, uint32_t viewport) const
	{
		if (!initialized || !frame.valid()) return false;

		// SL needs to be told which viewport to evaluate via `inputs`, even
		// though tagged resources come from set_tag() — omitting it fails with
		// eErrorMissingInputParameter (20).
		const sl::ViewportHandle viewport_handle(viewport);
		const sl::BaseStructure* inputs[] = { &viewport_handle };

		void* native_list = native_command_list ? native_command_list->get_native_handle() : nullptr;
		const sl::Result res = fn_evaluate_feature(to_sl(feature),
		                                            *static_cast<sl::FrameToken*>(frame.token),
		                                            inputs, 1, native_list);
		if (res != sl::Result::eOk)
		{
			Log::get() << (std::string("[Streamline] evaluate failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
			return false;
		}
		return true;
	}

	bool Streamline::upgrade_interface(void** base_interface) const
	{
		if (!initialized || !base_interface || !*base_interface) return false;

		const sl::Result res = fn_upgrade_interface(base_interface);
		if (res != sl::Result::eOk)
		{
			Log::get() << (std::string("[Streamline] upgrade_interface failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
			return false;
		}
		return true;
	}

	bool Streamline::allocate_resources(Feature feature, HAL::API::CommandList* native_command_list, uint32_t viewport) const
	{
		if (!initialized) return false;

		void* native_list = native_command_list ? native_command_list->get_native_handle() : nullptr;
		const sl::Result res = fn_allocate_resources(native_list, to_sl(feature), sl::ViewportHandle(viewport));
		if (res != sl::Result::eOk)
		{
			Log::get() << (std::string("[Streamline] allocate_resources failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
			return false;
		}
		return true;
	}
}
