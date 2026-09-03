// NVIDIA DLSS Ray Reconstruction — sibling to HAL:DLSS. Per NVIDIA's real
// integration guide (ProgrammingGuideDLSS_RR.md), not just
// slGetFeatureRequirements (which under-reports the true set): RR replaces
// the frame's SR upscale outright ("completely overrides DLSS Super
// Resolution"), AND wants reflection-specific hint tags on top of the base
// Depth/MotionVectors/ColorIn/ColorOut/Albedo set -- NormalRoughness,
// SpecularHitDistance, and SpecularAlbedo (F0) here.
export module HAL:DLSSRR;

import Core;
import streamline;
import :Streamline;
import :DLSS;
import :Resource;
import :CommandList;

export namespace nvidia
{
	class DLSSRR : public Singleton<DLSSRR>
	{
		friend class Singleton<DLSSRR>;
		DLSSRR();

		PFun_slDLSSDGetOptimalSettings* fn_get_optimal_settings = nullptr;
		PFun_slDLSSDSetOptions*         fn_set_options           = nullptr;

		bool resolved = false;
		mutable bool resources_allocated = false;

	public:
		// DLSSRR::get() must not be called before Device::init's bind_device().
		bool available() const { return resolved; }

		// Must be called once per frame before evaluate, even if unchanged.
		void set_options(DLSSMode mode, ivec2 output_size, const mat4x4& world_to_view, bool hdr, uint32_t viewport) const;

		// Defers its own body until `list`'s native command list is actually
		// open, mirroring DLSS::upscale(). Output size is read from
		// `color_out`'s own resource description, like DLSS::upscale().
		// `normal_roughness` is the packed buffer NormalRoughnessRepack
		// produces (see UpscalingDLSSRR.sig); `specular_hit_distance` reuses
		// VoxelReflectionNoise's alpha channel (RGB=hit color, A=hit distance).
		void denoise(HAL::CommandList& list, const FrameToken& frame, const FrameConstants& constants,
		             const mat4x4& world_to_view, DLSSMode mode, bool hdr, uint32_t viewport,
		             HAL::Resource* color_in, HAL::Resource* depth,
		             HAL::Resource* motion_vectors, HAL::Resource* color_out,
		             HAL::Resource* albedo, HAL::Resource* normal_roughness,
		             HAL::Resource* specular_hit_distance, HAL::Resource* specular_albedo) const;
	};
}
