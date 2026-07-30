module HAL:DLSS;

import Core;
import streamline;
import :Streamline;
import :Types;

namespace nvidia
{
	namespace
	{
		sl::DLSSMode to_sl(DLSSMode m)
		{
			switch (m)
			{
			case DLSSMode::MaxPerformance:   return sl::DLSSMode::eMaxPerformance;
			case DLSSMode::Balanced:         return sl::DLSSMode::eBalanced;
			case DLSSMode::MaxQuality:       return sl::DLSSMode::eMaxQuality;
			case DLSSMode::UltraQuality:     return sl::DLSSMode::eUltraQuality;
			case DLSSMode::UltraPerformance: return sl::DLSSMode::eUltraPerformance;
			case DLSSMode::DLAA:             return sl::DLSSMode::eDLAA;
			case DLSSMode::Off:
			default:                          return sl::DLSSMode::eOff;
			}
		}

		sl::DLSSOptions make_options(DLSSMode mode, ivec2 output_size, bool hdr)
		{
			sl::DLSSOptions options{};
			options.mode            = to_sl(mode);
			options.outputWidth     = static_cast<uint32_t>(output_size.x);
			options.outputHeight    = static_cast<uint32_t>(output_size.y);
			options.colorBuffersHDR = hdr ? sl::Boolean::eTrue : sl::Boolean::eFalse;

			return options;
		}

		ivec2 texture_size(const HAL::Resource* res)
		{
			const auto& tex = std::get<HAL::TextureDesc>(res->get_desc().desc);
			return { int(tex.Dimensions.x), int(tex.Dimensions.y) };
		}
	}

	DLSS::DLSS()
	{
		auto& sl_core = Streamline::get();
		if (!sl_core.available()) return;

		void* fn = nullptr;
		if (sl_core.get_feature_function(Feature::DLSS, "slDLSSGetOptimalSettings", fn))
			fn_get_optimal_settings = reinterpret_cast<PFun_slDLSSGetOptimalSettings*>(fn);

		fn = nullptr;
		if (sl_core.get_feature_function(Feature::DLSS, "slDLSSSetOptions", fn))
			fn_set_options = reinterpret_cast<PFun_slDLSSSetOptions*>(fn);

		resolved = fn_get_optimal_settings && fn_set_options;
		if (!resolved)
			Log::get() << "[Streamline] DLSS feature functions unavailable" << Log::endl;
	}

	bool DLSS::get_optimal_settings(DLSSMode mode, ivec2 output_size, bool hdr, DLSSOptimalSettings& out) const
	{
		if (!resolved) return false;

		const sl::DLSSOptions options = make_options(mode, output_size, hdr);

		sl::DLSSOptimalSettings settings{};
		if (fn_get_optimal_settings(options, settings) != sl::Result::eOk)
			return false;

		out.render_size     = { static_cast<int>(settings.optimalRenderWidth),  static_cast<int>(settings.optimalRenderHeight) };
		out.render_size_min = { static_cast<int>(settings.renderWidthMin),      static_cast<int>(settings.renderHeightMin) };
		out.render_size_max = { static_cast<int>(settings.renderWidthMax),      static_cast<int>(settings.renderHeightMax) };
		return true;
	}

	void DLSS::set_options(DLSSMode mode, ivec2 output_size, bool hdr, uint32_t viewport) const
	{
		if (!resolved) return;

		const sl::DLSSOptions options = make_options(mode, output_size, hdr);
		fn_set_options(sl::ViewportHandle(viewport), options);
	}

	void DLSS::upscale(HAL::CommandList& list, const FrameToken& frame, const FrameConstants& constants,
	                    DLSSMode mode, bool hdr, uint32_t viewport,
	                    HAL::Resource* color_in, HAL::Resource* depth,
	                    HAL::Resource* motion_vectors, HAL::Resource* color_out) const
	{
		if (!resolved || !frame.valid()) return;

		list.compiler.func(
			[this, frame, constants, mode, hdr, viewport, color_in, depth, motion_vectors, color_out]
			(HAL::API::CommandList& native_list)
			{
				set_options(mode, texture_size(color_out), hdr, viewport);

				auto& sl_core = Streamline::get();
				sl_core.set_constants(frame, constants, viewport);

				if (!resources_allocated)
					resources_allocated = sl_core.allocate_resources(Feature::DLSS, &native_list, viewport);

				sl_core.set_tag(frame, BufferType::ColorIn,      color_in->get_native(),      color_in->get_native_state(),      &native_list, viewport);
				sl_core.set_tag(frame, BufferType::Depth,         depth->get_native(),         depth->get_native_state(),         &native_list, viewport);
				sl_core.set_tag(frame, BufferType::MotionVectors, motion_vectors->get_native(), motion_vectors->get_native_state(), &native_list, viewport);
				sl_core.set_tag(frame, BufferType::ColorOut,      color_out->get_native(),     color_out->get_native_state(),     &native_list, viewport);

				bool ok = sl_core.evaluate(Feature::DLSS, frame, &native_list, viewport);

				// SL can silently recreate its internal DLSSContext; un-latch
				// so the next frame retries allocate_resources().
				if (!ok)
					resources_allocated = false;
			});

		// The deferred body above will let SL rebind its own compute state at
		// replay time; must invalidate here (record time), not inside it.
		list.invalidate_state();
	}
}
