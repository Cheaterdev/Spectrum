module HAL:DLSSRR;

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

		sl::float4x4 to_sl(const mat4x4& m)
		{
			sl::float4x4 out;
			for (uint32_t i = 0; i < 4; ++i)
				out.setRow(i, sl::float4(m[i][0], m[i][1], m[i][2], m[i][3]));
			return out;
		}

		sl::DLSSDOptions make_options(DLSSMode mode, ivec2 output_size, const mat4x4& world_to_view, bool hdr)
		{
			sl::DLSSDOptions options{};
			options.mode                = to_sl(mode);
			options.outputWidth         = static_cast<uint32_t>(output_size.x);
			options.outputHeight        = static_cast<uint32_t>(output_size.y);
			options.colorBuffersHDR     = hdr ? sl::Boolean::eTrue : sl::Boolean::eFalse;
			// Not tagging normal-roughness yet (see this module's doc comment),
			// but the field is mandatory on the struct either way.
			options.normalRoughnessMode = sl::DLSSDNormalRoughnessMode::ePacked;
			options.worldToCameraView   = to_sl(world_to_view);

			mat4x4 view_to_world = world_to_view;
			view_to_world.inverse();
			options.cameraViewToWorld = to_sl(view_to_world);

			return options;
		}

		ivec2 texture_size(const HAL::Resource* res)
		{
			const auto& tex = std::get<HAL::TextureDesc>(res->get_desc().desc);
			return { int(tex.Dimensions.x), int(tex.Dimensions.y) };
		}
	}

	DLSSRR::DLSSRR()
	{
		auto& sl_core = Streamline::get();
		if (!sl_core.available()) return;

		void* fn = nullptr;
		if (sl_core.get_feature_function(Feature::RayReconstruction, "slDLSSDGetOptimalSettings", fn))
			fn_get_optimal_settings = reinterpret_cast<PFun_slDLSSDGetOptimalSettings*>(fn);

		fn = nullptr;
		if (sl_core.get_feature_function(Feature::RayReconstruction, "slDLSSDSetOptions", fn))
			fn_set_options = reinterpret_cast<PFun_slDLSSDSetOptions*>(fn);

		resolved = fn_get_optimal_settings && fn_set_options;
		if (!resolved)
		{
			Log::get() << "[Streamline] DLSS-RR feature functions unavailable" << Log::endl;
			return;
		}

		// Bring-up only — see HAL:Streamline's log_feature_requirements doc
		// comment. Remove once DLSS-RR's required tag set is confirmed.
		sl_core.log_feature_requirements(Feature::RayReconstruction, "dlss_rr_requirements.temp");
	}

	void DLSSRR::set_options(DLSSMode mode, ivec2 output_size, const mat4x4& world_to_view, bool hdr, uint32_t viewport) const
	{
		if (!resolved) return;

		const sl::DLSSDOptions options = make_options(mode, output_size, world_to_view, hdr);
		const sl::Result res = fn_set_options(sl::ViewportHandle(viewport), options);
		if (res != sl::Result::eOk)
			Log::get() << (std::string("[Streamline] DLSS-RR set_options failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
	}

	void DLSSRR::denoise(HAL::CommandList& list, const FrameToken& frame, const FrameConstants& constants,
	                      const mat4x4& world_to_view, DLSSMode mode, bool hdr, uint32_t viewport,
	                      HAL::Resource* color_in, HAL::Resource* depth,
	                      HAL::Resource* motion_vectors, HAL::Resource* color_out,
	                      HAL::Resource* albedo, HAL::Resource* normal_roughness,
	                      HAL::Resource* specular_hit_distance, HAL::Resource* specular_albedo) const
	{
		if (!resolved || !frame.valid()) return;

		// Same COMMON/PRESENT hand-off requirement as DLSS::upscale() — see its
		// comment for why (legacy/enhanced barrier interop, D3D12 #1350).
		static const HAL::ResourceState kTagged{
			HAL::BarrierSync::COMPUTE_SHADING, HAL::BarrierAccess::COMMON, HAL::TextureLayout::PRESENT };

		static const HAL::ResourceState kWritten{
			HAL::BarrierSync::COMPUTE_SHADING, HAL::BarrierAccess::UNORDERED_ACCESS, HAL::TextureLayout::UNORDERED_ACCESS };

		list.begin_external_op(HAL::BarrierSync::COMPUTE_SHADING);
		list.add_resource_usage(color_in,              kTagged);
		list.add_resource_usage(depth,                 kTagged);
		list.add_resource_usage(motion_vectors,         kTagged);
		list.add_resource_usage(color_out,              kWritten);
		list.add_resource_usage(albedo,                 kTagged);
		list.add_resource_usage(normal_roughness,       kTagged);
		list.add_resource_usage(specular_hit_distance,  kTagged);
		list.add_resource_usage(specular_albedo,        kTagged);

		list.compiler.func(
			[this, frame, constants, world_to_view, mode, hdr, viewport, color_in, depth, motion_vectors,
			 color_out, albedo, normal_roughness, specular_hit_distance, specular_albedo]
			(HAL::API::CommandList& native_list)
			{
				set_options(mode, texture_size(color_out), world_to_view, hdr, viewport);

				auto& sl_core = Streamline::get();
				sl_core.set_constants(frame, constants, viewport);

				if (!resources_allocated)
					resources_allocated = sl_core.allocate_resources(Feature::RayReconstruction, &native_list, viewport);

				sl_core.set_tag(frame, BufferType::ColorIn,             color_in->get_native(),             color_in->get_native_state(),             &native_list, viewport);
				sl_core.set_tag(frame, BufferType::Depth,               depth->get_native(),                depth->get_native_state(),                &native_list, viewport);
				sl_core.set_tag(frame, BufferType::MotionVectors,       motion_vectors->get_native(),       motion_vectors->get_native_state(),       &native_list, viewport);
				sl_core.set_tag(frame, BufferType::ColorOut,            color_out->get_native(),            color_out->get_native_state(),            &native_list, viewport);
				sl_core.set_tag(frame, BufferType::Albedo,              albedo->get_native(),               albedo->get_native_state(),               &native_list, viewport);
				sl_core.set_tag(frame, BufferType::NormalRoughness,     normal_roughness->get_native(),     normal_roughness->get_native_state(),     &native_list, viewport);
				sl_core.set_tag(frame, BufferType::SpecularHitDistance, specular_hit_distance->get_native(), specular_hit_distance->get_native_state(), &native_list, viewport);
				sl_core.set_tag(frame, BufferType::SpecularAlbedo,      specular_albedo->get_native(),      specular_albedo->get_native_state(),      &native_list, viewport);

				bool ok = sl_core.evaluate(Feature::RayReconstruction, frame, &native_list, viewport);
				if (!ok) resources_allocated = false;
			});

		list.invalidate_state();
	}
}
