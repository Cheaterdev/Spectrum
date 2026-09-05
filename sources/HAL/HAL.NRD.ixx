// NVIDIA NRD (Real-time Denoisers) — REBLUR/RELAX/SIGMA. Statically linked
// (nrd.lib), unlike nvidia::Streamline: NRD makes zero GAPI calls itself
// (GetComputeDispatches() hands back a list of {shader, constants, resource
// bindings} for the engine to execute), so there's no device/DLL-hooking
// concern — CreateInstance() itself is pure CPU-side, no device parameter.
//
// Phase 1 (see [[project-nrd-integration]] plan): vendoring + a smoke test.
// One SIGMA_SHADOW instance, logging what GetInstanceDesc() reports.
//
// Phase 2: the raw-bytecode PSO/root-signature primitive -- one shared
// HAL::RootSignature (root CBV + 2 static samplers + SRV/UAV descriptor
// tables, built straight from InstanceDesc) and one raw ID3D12PipelineState
// per InstanceDesc::pipelines[i], recompiled from NRD's real vendored shader
// source (workdir/shaders/nrd/) via this engine's own DXC pipeline --
// bypassing HAL::ComputePipelineState (hard-coupled to SIG/DXC-compiled,
// slot-reflected Shader<T>), but NOT NRD's own embedded DXIL blob.
//
// Phase 3: pool textures (InstanceDesc::permanentPool/transientPool) --
// raw HAL::TextureResource per entry, sized against the actual render
// resolution once it's known (not at device-creation time, hence a separate
// ensure_pools() rather than folding this into create_pipelines()). Still no
// GetComputeDispatches() execution loop yet -- the per-frame descriptor-table
// population and dispatch calls are follow-up work.
export module HAL:NRD;

import Core;
import nrd;
import :Utils;
import :Device;
import :RootSignature;
import :Resource.Texture;

export namespace nvidia
{
	class NRD : public Singleton<NRD>
	{
		friend class Singleton<NRD>;
		NRD();
		~NRD();

		nrd::Instance* instance = nullptr;
		bool resolved = false;

		HAL::RootSignature::ptr root_signature;
		std::vector<D3D::PipelineState> pipelines;

		std::vector<HAL::TextureResource::ptr> permanent_pool;
		std::vector<HAL::TextureResource::ptr> transient_pool;
		uint2 pools_render_size = uint2(0, 0);

	public:
		bool available() const { return resolved; }
		bool pipelines_ready() const { return !pipelines.empty(); }
		bool pools_ready() const { return pools_render_size.x != 0; }

		// Bring-up only — logs what CreateInstance()/GetInstanceDesc() report.
		void smoke_test() const;

		// Builds the shared root signature and one PSO per InstanceDesc
		// pipeline from NRD's embedded DXIL. Needs a real device, so this is
		// called separately from the constructor -- see bind_device() call
		// sites for nvidia::Streamline/DLSS for the precedent.
		void create_pipelines(HAL::Device& device);

		// (Re)allocates the permanent/transient pool textures for the given
		// render resolution. Idempotent -- a no-op if already sized for
		// render_size. Call from wherever render resolution is known (e.g. a
		// PassNode's setup()/render(), matching how UpscalingDLSSRR reads
		// frame.upscale_size) -- NOT from create_pipelines(), which runs at
		// device creation before any resolution exists.
		void ensure_pools(HAL::Device& device, uint2 render_size);
	};
}
