export module Graphics:RTX;

import :MeshAsset;

import RenderSystem;
import HAL;
import Core;

export class RTX :public Singleton<RTX>, Events::prop_handler,
	public Events::Runner
{

public:
	
	using ptr = std::shared_ptr<RTX>;

	// Cross-pass debug toggle: when on, RTXShadow::render (PassDefaults.cpp)
	// skips its normal Bend/FFX hybrid-shadow-denoiser dispatch and instead
	// fires a genuine 16-ray soft-shadow reference directly into ShadowMask
	// -- a ground truth to compare VSM's own PCSS approximation against.
	// Set from VSM's own debug-view checkbox (VSM.ixx's
	// use_vsm_debug_rtx_reference, propagated here once per frame in
	// VSM.cpp's m_combine_render) rather than a toggle of its own, since the
	// two are the same user-facing switch: "show me the RTX reference".
	// Lives on this singleton (not a VSM member) because RTXShadow runs
	// earlier in the frame than VSM_Combine and has no direct reference to
	// the VSM instance -- this is the shared, always-linked spot both sides
	// already import. One-frame lag between toggling the checkbox and
	// RTXShadow picking it up (it renders before VSM_Combine writes this
	// each frame) is harmless for a debug comparison feature.
	bool debug_full_reference_shadow = false;

	MainRTX rtx{RenderSystem::get().device()};

	void prepare(HAL::CommandList::ptr& list);
   	void update();


	template<class T>
	void render(HAL::ComputeContext& compute, HAL::RaytracingAccelerationStructure::ptr scene_as, ivec2 size)
	{
		render<T>(compute, scene_as, ivec3{size,1});
	}


	template<class T>
	void render(HAL::ComputeContext& compute, HAL::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
	{

		if (!RenderSystem::get().device().is_rtx_supported()) return;
		
		{
			Slots::Raytracing rtx;
			rtx.GetScene() = scene_as->get_handle();
			compute.set(rtx);
		}

	
		rtx.dispatch<T>(size, compute);
	}


};
