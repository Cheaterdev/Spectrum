// NVIDIA NRD (Real-time Denoisers) — REBLUR/RELAX/SIGMA. Statically linked
// (nrd.lib), unlike nvidia::Streamline: NRD makes zero GAPI calls itself
// (GetComputeDispatches() hands back a list of {shader, constants, resource
// bindings} for the engine to execute), so there's no device/DLL-hooking
// concern — CreateInstance() itself is pure CPU-side, no device parameter.
//
// Phase 1 (see [[project-nrd-integration]] plan): vendoring + a smoke test.
// One SIGMA_SHADOW instance, logging what GetInstanceDesc() reports.
//
// Phase 2 (superseded): a raw-bytecode PSO/root-signature primitive existed
// here briefly (one hand-built HAL::RootSignature + raw ID3D12PipelineStates,
// bound at dispatch time via raw SetComputeRootDescriptorTable calls). Pulled
// per explicit direction: this engine's entire binding model is the SIG
// system's bindless one (one CBV-of-indices per layout slot, resolved via
// ResourceDescriptorHeap), and NRD's dispatches go through that instead --
// see Phase 4.
//
// Phase 3: pool textures (InstanceDesc::permanentPool/transientPool) --
// raw HAL::TextureResource per entry, sized against the actual render
// resolution once it's known (not at device-creation time, hence a separate
// ensure_pools() rather than folding this into create_pipelines()).
//
// Phase 4: the GetComputeDispatches() execution loop (execute()), SIG-driven.
// Each NRD kernel gets its own .sig struct + ComputePSO (ordinary SIG
// declarations -- ../custom-overlay/nrd's usage notes, sources/SIGParser/sigs/
// nrd_sig_test.sig for the first one, Clear_Constants/NRD_Clear_Test) plus a
// small shim .hlsl (workdir/shaders/nrd/sig_*.hlsl) that predefines NRD.hlsli's
// 12 binding macros to route through that struct's generated bindless
// accessors instead of raw register()s -- NRD.hlsli's own documented "custom
// engine" hook. At dispatch time, GetComputeDispatches()'s constant bytes and
// resource list are copied field-by-field into the matching Slots::X struct,
// and compute.set_pipeline<PSOS::X>()/set()/dispatch() do the rest (bindless
// index resolution, CBV upload, root signature/table binding) exactly like
// every other compute pass in the engine. Only kernels with a ported .sig
// struct actually dispatch; the rest are skipped (logged) until ported.
export module HAL:NRD;

import Core;
import nrd;
import :Utils;
import :Device;
import :Resource.Texture;
import :ResourceViews;
import :CommandList;

export namespace nvidia
{
	// Real per-frame REBLUR_DIFFUSE inputs (see [[project-nrd-integration]]),
	// supplied by NRD_REBLUR_Execute's render() (Effects/VoxelGI/NRD_REBLUR_
	// Execute.cpp) from NRD_GBufferPack's packed output + IndirectRTX's own
	// (now REBLUR-packed, see raytracing.hlsl) noisy signal. Plain views, not
	// FrameGraph handles -- HAL.NRD.cpp has no FrameGraph dependency, only
	// HAL::Texture2DView (already-created SRV/UAV pair) as everywhere else in
	// this codebase's PassDefault<> render() bodies.
	struct NRDFrameInputs
	{
		HAL::Texture2DView view_z;
		HAL::Texture2DView normal_roughness;
		HAL::Texture2DView mv;
		HAL::Texture2DView diff_noisy;
		HAL::Texture2DView diff_denoised;
		HAL::Texture2DView spec_noisy;
		HAL::Texture2DView spec_denoised;

		// SIGMA_SHADOW inputs/output -- penumbra_noisy is NRD's IN_PENUMBRA
		// (SIGMA_FrontEnd_PackPenumbra-encoded distance-to-occluder, R16f+),
		// shadow_denoised is OUT_SHADOW_TRANSLUCENCY (R8+, also reused by NRD
		// itself as SIGMA's own history -- see HAL.NRD.cpp's resolve_srv
		// comment). Like diff_noisy/spec_noisy, default-constructed (null
		// resource) until a real producer packs penumbra_noisy -- execute()
		// gates SIGMA_SHADOW on it the same way it gates REBLUR on diff_noisy/
		// spec_noisy.
		HAL::Texture2DView penumbra_noisy;
		HAL::Texture2DView shadow_denoised;

		// Column-major, vector-is-a-column, non-jittered (NRD's own
		// convention, NRDSettings.h) -- raw 16-float dumps of this frame's
		// and the previous frame's view/projection matrices. Populated from
		// camera::camera_cb.current/prev (Table::Camera) by the caller,
		// which already carries both in the layout this engine's own HLSL
		// mul(M,v) convention expects -- unverified against NRD's exact
        // convention beyond that; see the plan's "Known risks" (matrix
		// convention needs a visual check, not just a compile check).
		float world_to_view[16];
		float world_to_view_prev[16];
		float view_to_clip[16];
		float view_to_clip_prev[16];
		float2 jitter;
		float2 jitter_prev;

		// World-space direction TO the light source (SIGMA_SHADOW only --
		// NRDSettings.h's SigmaSettings::lightDirection convention). Rotated
		// into view space internally by NRD (Sigma.cpp's
		// AddSharedConstants_Sigma) and used by SIGMA_Blur.cs.hlsl to shape
		// the blur kernel's anisotropy along the light direction; left zero
		// silently degrades to an isotropic kernel (SIGMA_Blur.cs.hlsl's own
		// length(t) > 0.001 guard), not a hard failure, so an unset caller
		// wouldn't be caught by any error/validation check.
		float3 sun_direction;
	};

	class NRD : public Singleton<NRD>
	{
		friend class Singleton<NRD>;
		NRD();
		~NRD();

		// Two independent nrd::Instance objects, not one shared instance with
		// 3 denoisers (as Phase 1-4 originally had it) -- REBLUR_DIFFUSE/
		// REBLUR_SPECULAR (NRD_REBLUR_Execute, RTX-gated) and SIGMA_SHADOW
		// (NRD_SIGMA_Execute, gated on VSM's !use_vsm_penumbra instead) are
		// separate PassNodes with independent SetupConditions -- either can
		// run without the other in a given frame. NRD requires
		// CommonSettings::frameIndex to increment by exactly 1 per real frame
		// PER INSTANCE (NRDSettings.h's own doc comment); sharing one
		// Instance/one frame_counter between two independently-gated callers
		// would need fragile cross-pass "did the other one already run this
		// frame" coordination. Two Instances make it trivially true instead:
		// each owns its own pools/frame_counter, and calling execute_reblur()/
		// execute_shadow() from different (possibly concurrently-recorded
		// async) command lists is safe since neither touches the other's state.
		nrd::Instance* reblur_instance = nullptr;
		nrd::Instance* sigma_instance = nullptr;
		bool reblur_resolved = false;
		bool sigma_resolved = false;

		std::vector<HAL::TextureResource::ptr> reblur_permanent_pool, reblur_transient_pool;
		std::vector<HAL::TextureResource::ptr> sigma_permanent_pool, sigma_transient_pool;

		// Which pool pair resolve_pool_resource() indexes into -- set once at
		// the top of execute_reblur()/execute_shadow(), before that call's
		// dispatch loop runs. MUST be thread_local, not a plain member: the
		// two callers run on different FrameGraph queues (NRD_REBLUR_Execute
		// is [Async], NRD_SIGMA_Execute is [Async2], test.sig) whose command
		// lists can be recorded concurrently on different worker threads, and
		// a plain shared pointer here is a genuine data race -- confirmed
		// live as an intermittent out-of-bounds pool access (one thread's
		// execute_*() reading the OTHER thread's just-swapped-in pool size
		// mid-loop), not a hypothetical one. thread_local keeps each
		// recording thread's "which pool is active" fully independent, so
		// resolve_srv/resolve_uav/every dispatch_reblur_*/dispatch_sigma_*
		// helper in HAL.NRD.cpp can stay unchanged (still just
		// nrd_hal.resolve_pool_resource(r)) instead of threading a pool-set
		// parameter through all 18 of them.
		static thread_local std::vector<HAL::TextureResource::ptr>* active_permanent_pool;
		static thread_local std::vector<HAL::TextureResource::ptr>* active_transient_pool;

		uint2 pools_render_size = uint2(0, 0);

		// Set true by ensure_pools() whenever it actually reallocates the pool
		// textures (resize, or first-ever call) -- fresh D3D12 resources are
		// undefined content, not zero-initialized, so the next execute_reblur()/
		// execute_shadow() call must tell NRD to discard history for that one
		// frame (CommonSettings::accumulationMode = CLEAR_AND_RESTART) rather
		// than blending/reprojecting against garbage. Two independent flags,
		// each consumed-and-cleared only by its own execute_*() -- ensure_pools()
		// itself is called from BOTH NRD_REBLUR_Execute's and NRD_SIGMA_Execute's
		// pre_setup in the same frame (same render_size), so a single shared
		// flag would risk one consumer clearing it before the other reads it;
		// per-denoiser flags make consumption order irrelevant.
		bool reblur_needs_history_reset = false;
		bool sigma_needs_history_reset = false;

		// Fallback for named (non-pool) resource types this integration
		// doesn't wire (REBLUR's optional confidence/disocclusion inputs --
		// CommonSettings leaves those flags off, see HAL.NRD.cpp, so NRD
		// never actually requests them, but the resolver still needs a safe
		// default for anything unexpected). 1x1 so a wrong read/write is
		// harmless. Shared between both instances -- not per-instance state.
		HAL::TextureResource::ptr dummy_srv;
		HAL::TextureResource::ptr dummy_uav;

		// CommonSettings::frameIndex must increment by exactly 1 per real
		// frame per Instance (NRDSettings.h's own doc comment; NRD's
		// reference integration asserts this) -- owned here, not by the
		// caller, since execute_reblur()/execute_shadow() are each the one
		// place guaranteed to run at most once/frame for their own Instance.
		uint32_t reblur_frame_counter = 0;
		uint32_t sigma_frame_counter = 0;

	public:
		bool available() const { return reblur_resolved || sigma_resolved; }
		bool reblur_available() const { return reblur_resolved; }
		bool sigma_available() const { return sigma_resolved; }
		bool pools_ready() const { return pools_render_size.x != 0; }

		// PERMANENT_POOL/TRANSIENT_POOL resolve to whichever pool pair
		// execute_reblur()/execute_shadow() most recently made active (see
		// active_permanent_pool/active_transient_pool above). Public: called
		// from the free per-kernel dispatch_*() helpers in HAL.NRD.cpp, not
		// just NRD's own members.
		HAL::TextureResource::ptr resolve_pool_resource(const nrd::ResourceDesc& r) const;
		HAL::TextureResource::ptr dummy_srv_resource() const { return dummy_srv; }
		HAL::TextureResource::ptr dummy_uav_resource() const { return dummy_uav; }

		// Bring-up only — logs what CreateInstance()/GetInstanceDesc() report.
		void smoke_test() const;

		// (Re)allocates both instances' permanent/transient pool textures for
		// the given render resolution (skipping whichever instance failed to
		// create). Idempotent -- a no-op if already sized for render_size.
		// Call from wherever render resolution is known (e.g. a PassNode's
		// setup()/render(), matching how UpscalingDLSSRR reads
		// frame.upscale_size) -- safe to call from either NRD_REBLUR_Execute's
		// or NRD_SIGMA_Execute's own pre_setup(), whichever runs first each
		// frame does the (idempotent) work.
		void ensure_pools(HAL::Device& device, uint2 render_size);

		// Runs SetCommonSettings/GetComputeDispatches and issues the
		// resulting compute dispatches via the SIG system, on `list`, using
		// `inputs` for every named resource. REBLUR_DIFFUSE only dispatches
		// when inputs.diff_noisy is set (Texture2DView::resource non-null);
		// REBLUR_SPECULAR likewise on inputs.spec_noisy.
		void execute_reblur(HAL::CommandList& list, const NRDFrameInputs& inputs);

		// SIGMA_SHADOW only dispatches when inputs.penumbra_noisy is set --
		// see [[project-nrd-integration]]'s reflection-denoiser plan.
		// Unmatched identifiers (e.g. SplitScreen, never requested at default
		// CommonSettings::splitScreen=0) are logged and skipped.
		void execute_shadow(HAL::CommandList& list, const NRDFrameInputs& inputs);
	};
}
