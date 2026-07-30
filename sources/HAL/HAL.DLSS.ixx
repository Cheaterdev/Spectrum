// NVIDIA DLSS Super Resolution — options, sizing query, and the evaluate call.
// Split out of nvidia::Streamline so DLSS-RR can get its own sibling later.
// Entirely backend-agnostic via HAL::Resource::get_native()/get_native_state().
export module HAL:DLSS;

import Core;
import streamline;
import :Streamline;
import :Resource;
import :CommandList;

export namespace nvidia
{
	enum class DLSSMode
	{
		Off,
		MaxPerformance,
		Balanced,
		MaxQuality,
		UltraQuality,
		UltraPerformance,
		DLAA,   // native resolution, DLSS anti-aliasing only (no upscale)
	};

	struct DLSSOptimalSettings
	{
		ivec2 render_size;       // recommended render resolution for this mode/output
		ivec2 render_size_min;   // dynamic-resolution range SL considers valid
		ivec2 render_size_max;
	};

	class DLSS : public Singleton<DLSS>
	{
		friend class Singleton<DLSS>;
		DLSS();

		PFun_slDLSSGetOptimalSettings* fn_get_optimal_settings = nullptr;
		PFun_slDLSSSetOptions*         fn_set_options           = nullptr;

		bool resolved = false;

		// Set true the first time Streamline::allocate_resources() succeeds —
		// SL's per-feature resources only need allocating once.
		mutable bool resources_allocated = false;

	public:
		// DLSS::get() must not be called before Device::init's bind_device() —
		// resolution happens on the first get() and is never retried.
		bool available() const { return resolved; }

		// Recommended render resolution for `mode`. False (and *out untouched)
		// if unavailable. `hdr` must match the color buffers the caller will
		// actually tag (see upscale()'s `hdr` parameter).
		bool get_optimal_settings(DLSSMode mode, ivec2 output_size, bool hdr, DLSSOptimalSettings& out) const;

		// Must be called once per frame before evaluate, even if unchanged.
		void set_options(DLSSMode mode, ivec2 output_size, bool hdr, uint32_t viewport) const;

		// Defers its own body until `list`'s native command list is actually
		// open, so the caller just calls this once from render() — hence void,
		// not bool. Output size is read from color_out's own resource
		// description; `hdr`/`viewport` are caller-supplied, not assumed here.
		void upscale(HAL::CommandList& list, const FrameToken& frame, const FrameConstants& constants,
		             DLSSMode mode, bool hdr, uint32_t viewport,
		             HAL::Resource* color_in, HAL::Resource* depth,
		             HAL::Resource* motion_vectors, HAL::Resource* color_out) const;
	};
}
