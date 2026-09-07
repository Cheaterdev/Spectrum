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
	};

	class NRD : public Singleton<NRD>
	{
		friend class Singleton<NRD>;
		NRD();
		~NRD();

		nrd::Instance* instance = nullptr;
		bool resolved = false;

		std::vector<HAL::TextureResource::ptr> permanent_pool;
		std::vector<HAL::TextureResource::ptr> transient_pool;
		uint2 pools_render_size = uint2(0, 0);

		// Fallback for named (non-pool) resource types this integration
		// doesn't wire (SIGMA's, REBLUR's optional confidence/disocclusion
		// inputs -- CommonSettings leaves those flags off, see HAL.NRD.cpp,
		// so NRD never actually requests them, but the resolver still needs
		// a safe default for anything unexpected). 1x1 so a wrong read/write
		// is harmless.
		HAL::TextureResource::ptr dummy_srv;
		HAL::TextureResource::ptr dummy_uav;

		// CommonSettings::frameIndex must increment by exactly 1 per real
		// frame (NRDSettings.h's own doc comment; NRD's reference
		// integration asserts this) -- owned here, not by the caller, since
		// execute() is the one place guaranteed to run exactly once/frame.
		uint32_t frame_counter = 0;

	public:
		bool available() const { return resolved; }
		bool pools_ready() const { return pools_render_size.x != 0; }

		// PERMANENT_POOL/TRANSIENT_POOL resolve to the real pool textures.
		// Public: called from the free per-kernel dispatch_*() helpers in
		// HAL.NRD.cpp, not just NRD's own members.
		HAL::TextureResource::ptr resolve_pool_resource(const nrd::ResourceDesc& r) const;
		HAL::TextureResource::ptr dummy_srv_resource() const { return dummy_srv; }
		HAL::TextureResource::ptr dummy_uav_resource() const { return dummy_uav; }

		// Bring-up only — logs what CreateInstance()/GetInstanceDesc() report.
		void smoke_test() const;

		// (Re)allocates the permanent/transient pool textures for the given
		// render resolution. Idempotent -- a no-op if already sized for
		// render_size. Call from wherever render resolution is known (e.g. a
		// PassNode's setup()/render(), matching how UpscalingDLSSRR reads
		// frame.upscale_size).
		void ensure_pools(HAL::Device& device, uint2 render_size);

		// Runs SetCommonSettings/GetComputeDispatches and issues the
		// resulting compute dispatches via the SIG system, on `list`, using
		// `inputs` for every named resource. REBLUR_DIFFUSE only dispatches
		// when inputs.diff_noisy is set (Texture2DView::resource non-null);
		// REBLUR_SPECULAR likewise on inputs.spec_noisy -- see
		// [[project-nrd-integration]]'s reflection-denoiser plan. SIGMA_
		// SHADOW's dispatches are still skipped (unwired, out of scope);
		// unmatched identifiers are logged and skipped, same as before.
		void execute(HAL::CommandList& list, const NRDFrameInputs& inputs);
	};
}
