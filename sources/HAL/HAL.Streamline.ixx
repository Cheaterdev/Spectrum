// NVIDIA Streamline — engine-side owner of the SDK. Nothing else in HAL
// should touch sl* directly.
//
// Manual-hooking integration: sl.interposer.lib is not linked (it lives in a
// Streamline subdirectory Windows won't search for static imports), so entry
// points are resolved by GetProcAddress instead.
export module HAL:Streamline;

import Core;
import :Utils;
import streamline;
import :API.Streamline;
import :API.CommandList;

export namespace nvidia
{
	enum class Feature
	{
		DLSS,               // Super Resolution      (sl::kFeatureDLSS)
		RayReconstruction,  // DLSS-D / denoiser     (sl::kFeatureDLSS_RR)
		Common,             // sl::kFeatureCommon — not meant to be evaluated
		                    // directly; kept only for to_sl()'s mapping.
	};

	// Buffer slots a temporal SL feature reads/writes.
	enum class BufferType
	{
		Depth,           // sl::kBufferTypeDepth
		MotionVectors,   // sl::kBufferTypeMotionVectors
		ColorIn,         // sl::kBufferTypeScalingInputColor  (render-resolution)
		ColorOut,        // sl::kBufferTypeScalingOutputColor (output-resolution)
	};

	// Opaque per-frame identity for SL's internal frame pipeline. Obtain exactly
	// once per engine frame via Streamline::begin_frame() (see current_frame()).
	class FrameToken
	{
		friend class Streamline;
		void* token = nullptr;   // sl::FrameToken*, kept opaque

	public:
		bool valid() const { return token != nullptr; }
	};

	// Per-frame camera/motion data a temporal SL feature needs. Matrices are
	// UNJITTERED — SL applies jitter_offset itself.
	struct FrameConstants
	{
		mat4x4 view_to_clip;       // current-frame unjittered projection
		mat4x4 clip_to_view;
		mat4x4 clip_to_prev_clip;  // current frame's clip space -> previous frame's
		vec2   jitter_offset;      // pixels; {0,0} until camera jitter is re-enabled
		vec2   mvec_scale = vec2(1, 1);
		vec3   camera_pos, camera_up, camera_right, camera_fwd;
		bool   reset = false;      // true on a cut/resize: invalidates SL's history
	};

	// Must be a singleton rather than a Device member: slInit has to run before
	// adapters are enumerated (Device::probe queries feature support per
	// adapter), so it necessarily predates any Device.
	class Streamline : public API::Streamline, public Singleton<Streamline>
	{
		friend class Singleton<Streamline>;
		Streamline();

		void* library = nullptr;   // HMODULE, opaque to keep <Windows.h> out of here

		// Backend-agnostic entry points only; the API-specific ones are resolved
		// by API::Streamline::resolve_api().
		PFun_slInit*                fn_init                 = nullptr;
		PFun_slShutdown*            fn_shutdown             = nullptr;
		PFun_slIsFeatureSupported*  fn_is_feature_supported = nullptr;
		PFun_slGetFeatureFunction*  fn_get_feature_function = nullptr;
		PFun_slGetNewFrameToken*    fn_get_new_frame_token  = nullptr;
		PFun_slSetConstants*        fn_set_constants        = nullptr;
		PFun_slSetTagForFrame*      fn_set_tag_for_frame    = nullptr;
		PFun_slEvaluateFeature*     fn_evaluate_feature     = nullptr;
		PFun_slAllocateResources*   fn_allocate_resources   = nullptr;
		PFun_slUpgradeInterface*    fn_upgrade_interface    = nullptr;

		bool initialized = false;
		FrameToken cached_frame;

	public:
		~Streamline() override;

		// False on no NVIDIA GPU, a stale output dir, or an old driver — not an error.
		bool available() const { return initialized; }

		// `luid` is the raw DXGI_ADAPTER_DESC::AdapterLuid bytes.
		bool supports(Feature feature, const void* luid, uint luid_size) const;

		// Call once, right after device creation.
		void bind_device(D3D::Device native_device);

		// Must be called after bind_device() succeeded.
		bool get_feature_function(Feature feature, const char* name, void*& fn) const;

		// Must be called exactly once per engine frame, before any other
		// per-frame SL call.
		FrameToken begin_frame();

		// This frame's begin_frame() token — read this instead of calling
		// begin_frame() again (a second call per frame desyncs SL).
		const FrameToken& current_frame() const { return cached_frame; }

		// `viewport` identifies which SL viewport this call applies to — pass 0
		// if the caller only ever renders a single viewport. Streamline itself
		// has no opinion on viewport count or numbering; that's the caller's.
		void set_constants(const FrameToken& frame, const FrameConstants& constants, uint32_t viewport) const;

		// Call once per BufferType the feature needs. `native_command_list` may be null.
		void set_tag(const FrameToken& frame, BufferType type, void* native_resource,
		             uint32_t d3d12_state, HAL::API::CommandList* native_command_list, uint32_t viewport) const;

		// Generic slEvaluateFeature. `native_command_list` may be null.
		// eDisableCLStateTracking means root signature/PSO/descriptor heaps on
		// that list are undefined once this returns.
		bool evaluate(Feature feature, const FrameToken& frame, HAL::API::CommandList* native_command_list, uint32_t viewport) const;

		// Must be called once before the first evaluate() for a feature — manual
		// hooking has nothing to piggyback automatic allocation on.
		// `native_command_list` may be null.
		bool allocate_resources(Feature feature, HAL::API::CommandList* native_command_list, uint32_t viewport) const;

		// Manual-hooking's substitute for the interposer's automatic
		// IDXGISwapChain::Present hook — call once, right after creating the
		// swapchain, passing the address of the raw interface pointer; on
		// success it's replaced in place with an SL proxy. See
		// [[project-streamline-dlss-integration]] for why this (not
		// kFeatureCommon) is the real fix for evaluate() failing with
		// eErrorMissingOrInvalidAPI.
		bool upgrade_interface(void** base_interface) const;
	};
}
