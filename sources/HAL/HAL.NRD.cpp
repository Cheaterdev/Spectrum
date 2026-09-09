module HAL:NRD;

import Core;
import nrd;
import :Utils;
import :Device;
import :Resource;
import :Resource.Texture;
import :HLSL;
import :DescriptorHeap;
import :CommandList;
import :Autogen;
import d3d12;
import wrl;

namespace nvidia
{
	// nrd::Format's names abbreviate the RGBA components NRD-side
	// (e.g. RGBA16_SFLOAT); HAL::Format spells every component out
	// DXGI-style (R16G16B16A16_FLOAT). Mechanical 1:1 mapping, no format
	// this engine doesn't already have a DXGI equivalent for.
	static HAL::Format to_hal_format(nrd::Format f)
	{
		switch (f)
		{
		case nrd::Format::R8_UNORM: return HAL::Format::R8_UNORM;
		case nrd::Format::R8_SNORM: return HAL::Format::R8_SNORM;
		case nrd::Format::R8_UINT: return HAL::Format::R8_UINT;
		case nrd::Format::R8_SINT: return HAL::Format::R8_SINT;
		case nrd::Format::RG8_UNORM: return HAL::Format::R8G8_UNORM;
		case nrd::Format::RG8_SNORM: return HAL::Format::R8G8_SNORM;
		case nrd::Format::RG8_UINT: return HAL::Format::R8G8_UINT;
		case nrd::Format::RG8_SINT: return HAL::Format::R8G8_SINT;
		case nrd::Format::RGBA8_UNORM: return HAL::Format::R8G8B8A8_UNORM;
		case nrd::Format::RGBA8_SNORM: return HAL::Format::R8G8B8A8_SNORM;
		case nrd::Format::RGBA8_UINT: return HAL::Format::R8G8B8A8_UINT;
		case nrd::Format::RGBA8_SINT: return HAL::Format::R8G8B8A8_SINT;
		case nrd::Format::RGBA8_SRGB: return HAL::Format::R8G8B8A8_UNORM_SRGB;
		case nrd::Format::R16_UNORM: return HAL::Format::R16_UNORM;
		case nrd::Format::R16_SNORM: return HAL::Format::R16_SNORM;
		case nrd::Format::R16_UINT: return HAL::Format::R16_UINT;
		case nrd::Format::R16_SINT: return HAL::Format::R16_SINT;
		case nrd::Format::R16_SFLOAT: return HAL::Format::R16_FLOAT;
		case nrd::Format::RG16_UNORM: return HAL::Format::R16G16_UNORM;
		case nrd::Format::RG16_SNORM: return HAL::Format::R16G16_SNORM;
		case nrd::Format::RG16_UINT: return HAL::Format::R16G16_UINT;
		case nrd::Format::RG16_SINT: return HAL::Format::R16G16_SINT;
		case nrd::Format::RG16_SFLOAT: return HAL::Format::R16G16_FLOAT;
		case nrd::Format::RGBA16_UNORM: return HAL::Format::R16G16B16A16_UNORM;
		case nrd::Format::RGBA16_SNORM: return HAL::Format::R16G16B16A16_SNORM;
		case nrd::Format::RGBA16_UINT: return HAL::Format::R16G16B16A16_UINT;
		case nrd::Format::RGBA16_SINT: return HAL::Format::R16G16B16A16_SINT;
		case nrd::Format::RGBA16_SFLOAT: return HAL::Format::R16G16B16A16_FLOAT;
		case nrd::Format::R32_UINT: return HAL::Format::R32_UINT;
		case nrd::Format::R32_SINT: return HAL::Format::R32_SINT;
		case nrd::Format::R32_SFLOAT: return HAL::Format::R32_FLOAT;
		case nrd::Format::RG32_UINT: return HAL::Format::R32G32_UINT;
		case nrd::Format::RG32_SINT: return HAL::Format::R32G32_SINT;
		case nrd::Format::RG32_SFLOAT: return HAL::Format::R32G32_FLOAT;
		case nrd::Format::RGB32_UINT: return HAL::Format::R32G32B32_UINT;
		case nrd::Format::RGB32_SINT: return HAL::Format::R32G32B32_SINT;
		case nrd::Format::RGB32_SFLOAT: return HAL::Format::R32G32B32_FLOAT;
		case nrd::Format::RGBA32_UINT: return HAL::Format::R32G32B32A32_UINT;
		case nrd::Format::RGBA32_SINT: return HAL::Format::R32G32B32A32_SINT;
		case nrd::Format::RGBA32_SFLOAT: return HAL::Format::R32G32B32A32_FLOAT;
		case nrd::Format::R10_G10_B10_A2_UNORM: return HAL::Format::R10G10B10A2_UNORM;
		case nrd::Format::R10_G10_B10_A2_UINT: return HAL::Format::R10G10B10A2_UINT;
		case nrd::Format::R11_G11_B10_UFLOAT: return HAL::Format::R11G11B10_FLOAT;
		case nrd::Format::R9_G9_B9_E5_UFLOAT: return HAL::Format::R9G9B9E5_SHAREDEXP;
		default: return HAL::Format::R8G8B8A8_UNORM;
		}
	}

	NRD::NRD()
	{
		const nrd::LibraryDesc& lib = *nrd::GetLibraryDesc();
		Log::get() << "[NRD] library v" << (int)lib.versionMajor << "." << (int)lib.versionMinor
		           << "." << (int)lib.versionBuild << ", " << lib.supportedDenoisersNum
		           << " denoisers supported" << Log::endl;

		// SIGMA_SHADOW for RTXShadowNoise (ShadowRTX, voxel.sig). REBLUR_DIFFUSE
		// for indirect GI (RTXIndirectNoise, IndirectRTX, voxel.sig).
		// REBLUR_SPECULAR for reflections (RTXReflectionNoise/
		// VoxelReflectionNoise) -- a separate instance rather than the
		// combined REBLUR_DIFFUSE_SPECULAR method, so either signal can be
		// denoised independently of the other (g_indirect_denoiser and
		// g_reflection_denoiser are independent toggles, see
		// [[project-nrd-integration]]).
		static const nrd::DenoiserDesc denoisers[] = {
			{ 0, nrd::Denoiser::SIGMA_SHADOW },
			{ 1, nrd::Denoiser::REBLUR_DIFFUSE },
			{ 2, nrd::Denoiser::REBLUR_SPECULAR }
		};

		nrd::InstanceCreationDesc desc{};
		desc.denoisers = denoisers;
		desc.denoisersNum = 3;

		const nrd::Result res = nrd::CreateInstance(desc, instance);
		if (res != nrd::Result::SUCCESS)
		{
			Log::get() << "[NRD] CreateInstance failed (" << (int)res << ")" << Log::endl;
			return;
		}

		resolved = true;
		Log::get() << "[NRD] instance created" << Log::endl;
	}

	NRD::~NRD()
	{
		if (instance)
			nrd::DestroyInstance(*instance);
	}

	void NRD::smoke_test() const
	{
		if (!resolved) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);
		Log::get() << "[NRD] instance desc: " << idesc.pipelinesNum << " pipelines, "
		           << idesc.permanentPoolSize << " permanent pool textures, "
		           << idesc.transientPoolSize << " transient pool textures, "
		           << idesc.constantBufferMaxDataSize << " max CB bytes" << Log::endl;
	}

	void NRD::ensure_pools(HAL::Device& device, uint2 render_size)
	{
		if (!resolved) return;
		if (pools_render_size == render_size && pools_ready()) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);

		auto make_pool = [&](const nrd::TextureDesc* descs, uint32_t count, std::vector<HAL::TextureResource::ptr>& out)
		{
			out.clear();
			out.reserve(count);
			for (uint32_t i = 0; i < count; ++i)
			{
				const nrd::TextureDesc& td = descs[i];

				// downsampleFactor is a linear divisor (e.g. 16 for
				// SIGMA_ClassifyTiles' 16x16 tile-classification output, not
				// a bit-shift exponent), and ceiling-divided -- confirmed
				// against NVIDIA's own reference integration,
				// NRDIntegration.hpp: "DivideUp(m_Desc.resourceWidth,
				// nrdTextureDesc.downsampleFactor)".
				uint2 size(
					(render_size.x + td.downsampleFactor - 1) / td.downsampleFactor,
					(render_size.y + td.downsampleFactor - 1) / td.downsampleFactor);

				out.push_back(std::make_shared<HAL::TextureResource>(device,
					HAL::ResourceDesc::Tex2D(to_hal_format(td.format), size, 1, 1,
						HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource),
					HAL::HeapType::DEFAULT));
			}
		};

		make_pool(idesc.permanentPool, idesc.permanentPoolSize, permanent_pool);
		make_pool(idesc.transientPool, idesc.transientPoolSize, transient_pool);

		if (!dummy_srv)
		{
			dummy_srv = std::make_shared<HAL::TextureResource>(device,
				HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT, uint2(1, 1), 1, 1,
					HAL::ResFlags::ShaderResource),
				HAL::HeapType::DEFAULT);
			dummy_uav = std::make_shared<HAL::TextureResource>(device,
				HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT, uint2(1, 1), 1, 1,
					HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource),
				HAL::HeapType::DEFAULT);
		}

		pools_render_size = render_size;

		Log::get() << "[NRD] pools allocated: " << permanent_pool.size() << " permanent, "
		           << transient_pool.size() << " transient, at " << render_size.x << "x" << render_size.y << Log::endl;
	}

	// PERMANENT_POOL/TRANSIENT_POOL resolve to the real pool textures. Every
	// other (named) resource type has no real engine-side producer yet --
	// per-kernel dispatch code below routes those to a dummy directly.
	HAL::TextureResource::ptr NRD::resolve_pool_resource(const nrd::ResourceDesc& r) const
	{
		if (r.type == nrd::ResourceType::PERMANENT_POOL)
			return permanent_pool[r.indexInPool];
		if (r.type == nrd::ResourceType::TRANSIENT_POOL)
			return transient_pool[r.indexInPool];

		return r.descriptorType == nrd::DescriptorType::TEXTURE ? dummy_srv : dummy_uav;
	}

	// Clear.cs.hlsl|FLOAT=1 -- first ported kernel (see nrd_sig_test.sig,
	// workdir/shaders/nrd/sig_clear.hlsl). Clear.resources.hlsli declares
	// gDebug/gViewZScale/gDenoisingRange only "for availability in
	// Common.hlsl" -- Clear.cs.hlsl's actual body (gOut[pixelPos] = 0;) never
	// reads them, so NRD's own reflection drops them from this pipeline's
	// constant blob entirely: constantBufferDataSize is 0, confirmed at
	// runtime (18/18 dispatches this run) -- and idesc.pipelines[i].
	// hasConstantData is also 0, NRD's own per-pipeline reflection flag
	// (baked in at NRD.lib build time, independent of this dispatch call),
	// ruling out "no real work requested" as the explanation: gridWidth/
	// gridHeight for these same dispatches are 120x68 (full 1920x1080 at
	// [numthreads(16,16,1)]), i.e. genuine full-resolution clears. Asserted,
	// not branched around --
	// if NRD ever starts sending bytes here (a version bump, a different
	// permutation), silently leaving gDebug/gViewZScale/gDenoisingRange
	// uninitialized would be a real correctness bug; this fails loudly
	// instead so it gets a real fix (copy the bytes) rather than staying
	// silently wrong. Clear_Constants' generated Compiled layout has gOut (a
	// bindless resource index) appended after the 3 floats regardless --
	// that's never part of NRD's own constant blob at all, since in NRD's
	// model resources are always bound separately from constants, from the
	// dispatch's own resource list.
	static void dispatch_clear(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch)
	{
		ASSERT(dispatch.resourcesNum == 1);
		HAL::TextureResource::ptr output = nrd_hal.resolve_pool_resource(dispatch.resources[0]);

		ASSERT(dispatch.constantBufferDataSize == 0);
		Slots::Clear_Constants slots;

		auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
		HLSL::RWTexture2D<float4> view(h);
		view.create(output, 0, 0);
		slots.GetGOut() = view;

		compute.set_pipeline<PSOS::NRD_Clear_Test>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// Item 8 (see [[project-nrd-integration]]): real REBLUR_DIFFUSE dispatch
	// wiring. resolve_srv/resolve_uav resolve one nrd::ResourceDesc entry
	// (from a DispatchDesc::resources[] array, walked position-for-position
	// against each kernel's .sig struct field order below -- both are
	// derived from the same source, that kernel's real resources.hlsli
	// NRD_INPUTS/NRD_OUTPUTS declaration order, so position i in one matches
	// field i in the other) into a bindable view: PERMANENT_POOL/
	// TRANSIENT_POOL get a fresh descriptor via alloc_descriptor + .create()
	// (same as dispatch_clear, since pool textures are raw
	// HAL::TextureResource, not FrameGraph resources with a cached dual
	// view); the 4 named external inputs this integration wires
	// (IN_VIEWZ/IN_NORMAL_ROUGHNESS/IN_MV/IN_DIFF_RADIANCE_HITDIST) and the
	// 1 named output (OUT_DIFF_RADIANCE_HITDIST) reuse NRDFrameInputs'
	// already-created FrameGraph views directly (no alloc_descriptor
	// needed -- HAL::Texture2DView already carries both SRV/UAV descriptors).
	// Anything else (SIGMA's own named types, REBLUR's optional confidence/
	// disocclusion inputs this integration leaves off in CommonSettings, so
	// NRD never actually requests them) falls back to a 1x1 dummy.
	static HLSL::Texture2D<> resolve_srv(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute,
		const nrd::ResourceDesc& r, const nvidia::NRDFrameInputs& in)
	{
		if (r.type == nrd::ResourceType::PERMANENT_POOL || r.type == nrd::ResourceType::TRANSIENT_POOL)
		{
			auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
			HLSL::Texture2D<> view(h);
			view.create(nrd_hal.resolve_pool_resource(r), 0, 1, 0);
			return view;
		}

		switch (r.type)
		{
		case nrd::ResourceType::IN_VIEWZ:               return in.view_z.texture2D;
		case nrd::ResourceType::IN_NORMAL_ROUGHNESS:    return in.normal_roughness.texture2D;
		case nrd::ResourceType::IN_MV:                  return in.mv.texture2D;
		case nrd::ResourceType::IN_DIFF_RADIANCE_HITDIST: return in.diff_noisy.texture2D;
		case nrd::ResourceType::OUT_DIFF_RADIANCE_HITDIST: return in.diff_denoised.texture2D;
		case nrd::ResourceType::IN_SPEC_RADIANCE_HITDIST: return in.spec_noisy.texture2D;
		case nrd::ResourceType::OUT_SPEC_RADIANCE_HITDIST: return in.spec_denoised.texture2D;
		default:
			ASSERT(!"unhandled named SRV resource type -- see HAL.NRD.cpp's resolve_srv comment");
			auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
			HLSL::Texture2D<> view(h);
			view.create(nrd_hal.dummy_srv_resource(), 0, 1, 0);
			return view;
		}
	}

	static HLSL::RWTexture2D<> resolve_uav(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute,
		const nrd::ResourceDesc& r, const nvidia::NRDFrameInputs& in)
	{
		if (r.type == nrd::ResourceType::PERMANENT_POOL || r.type == nrd::ResourceType::TRANSIENT_POOL)
		{
			auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
			HLSL::RWTexture2D<> view(h);
			view.create(nrd_hal.resolve_pool_resource(r), 0, 0);
			return view;
		}

		switch (r.type)
		{
		case nrd::ResourceType::IN_MV:                  return in.mv.rwTexture2D; // temporal-stabilization writes it back as scratch
		case nrd::ResourceType::IN_NORMAL_ROUGHNESS:    return in.normal_roughness.rwTexture2D; // post-blur's gOut_Normal_Roughness copy
		case nrd::ResourceType::OUT_DIFF_RADIANCE_HITDIST: return in.diff_denoised.rwTexture2D;
		case nrd::ResourceType::OUT_SPEC_RADIANCE_HITDIST: return in.spec_denoised.rwTexture2D;
		default:
			ASSERT(!"unhandled named UAV resource type -- see HAL.NRD.cpp's resolve_uav comment");
			auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
			HLSL::RWTexture2D<> view(h);
			view.create(nrd_hal.dummy_uav_resource(), 0, 0);
			return view;
		}
	}

	// REBLURSharedConstants is #pragma pack(push,1) with fields in the exact
	// order/type of REBLUR_Config.hlsli's REBLUR_SHARED_CONSTANTS macro (see
	// nrd_sig_test.sig's comment) -- verified by hand against HLSL's default
	// cbuffer packing rules (no field here straddles a 16-byte boundary, so
	// zero implicit padding is needed at any point in this exact sequence),
	// so NRD's own raw constantBufferData blob can be copied onto it
	// directly instead of 77 field-by-field assignments. The size assert is
	// the real safety net: if it ever fires, the packing assumption above is
	// wrong for some field ordering and needs revisiting, not the assert
	// weakened away.
	static void fill_reblur_shared_constants(Table::REBLURSharedConstants& dst, const nrd::DispatchDesc& dispatch)
	{
		// HLSL cbuffers are sized in multiples of 16 bytes overall; NRD's
		// raw blob is padded to that (864 bytes, confirmed at runtime),
		// 8 bytes past this struct's own tightly-packed (pack(1), no
		// trailing pad) sizeof (856) -- the size fields themselves (77
		// fields, no straddling boundary, see this function's header
		// comment) still match field-for-field, only the very end differs.
		// Only copy this struct's own byte count; the trailing pad is
		// unused on the C++ side either way.
		ASSERT(dispatch.constantBufferDataSize >= sizeof(Table::REBLURSharedConstants)
			&& dispatch.constantBufferDataSize < sizeof(Table::REBLURSharedConstants) + 16);
		memcpy(&dst, dispatch.constantBufferData, sizeof(Table::REBLURSharedConstants));
	}

	static void dispatch_reblur_classifytiles(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 2);
		Slots::REBLUR_ClassifyTilesResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGOut_Tiles() = resolve_uav(nrd_hal, compute, dispatch.resources[1], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_ClassifyTiles>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// Shared by both HitDistReconstruction PSOs (MODE_5X5=0/1 -- identical
	// resource layout, see nrd_sig_test.sig's comment on the 5x5 PSO).
	template<typename PSO>
	static void dispatch_reblur_hitdistreconstruction(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::REBLUR_HitDistReconstructionResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		compute.set_pipeline<PSO>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_hitdistreconstruction --
	// same resource count/layout (4 in + 1 out, Diff->Spec renamed), see
	// nrd_sig_test.sig's REBLUR_HitDistReconstructionSpecularResources
	// comment.
	template<typename PSO>
	static void dispatch_reblur_hitdistreconstruction_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::REBLUR_HitDistReconstructionSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		compute.set_pipeline<PSO>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_prepass(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::REBLUR_PrePassResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PrePass>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_prepass -- one extra output
	// (gOut_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.sig's REBLUR_PrePassSpecularResources comment.
	static void dispatch_reblur_prepass_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 6);
		Slots::REBLUR_PrePassSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_SpecHitDistForTracking() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PrePass_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_temporalaccumulation(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 16);
		Slots::REBLUR_TemporalAccumulationResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Mv() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGPrev_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGPrev_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGPrev_InternalData() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGIn_DisocclusionThresholdMix() = resolve_srv(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGIn_DiffConfidence() = resolve_srv(nrd_hal, compute, dispatch.resources[8], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[9], in);
		slots.GetGHistory_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[10], in);
		slots.GetGHistory_DiffFast() = resolve_srv(nrd_hal, compute, dispatch.resources[11], in);
		slots.GetGOut_Data1() = resolve_uav(nrd_hal, compute, dispatch.resources[12], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[13], in);
		slots.GetGOut_DiffFast() = resolve_uav(nrd_hal, compute, dispatch.resources[14], in);
		slots.GetGOut_Data2() = resolve_uav(nrd_hal, compute, dispatch.resources[15], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalAccumulation>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_temporalaccumulation -- see
	// nrd_sig_test.sig's REBLUR_TemporalAccumulationSpecularResources comment
	// for the field-list differences vs diffuse (14 in + 5 out here, vs
	// diffuse's 12 in + 4 out).
	static void dispatch_reblur_temporalaccumulation_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 19);
		Slots::REBLUR_TemporalAccumulationSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Mv() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGPrev_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGPrev_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGPrev_InternalData() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGIn_DisocclusionThresholdMix() = resolve_srv(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGIn_SpecConfidence() = resolve_srv(nrd_hal, compute, dispatch.resources[8], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[9], in);
		slots.GetGHistory_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[10], in);
		slots.GetGHistory_SpecFast() = resolve_srv(nrd_hal, compute, dispatch.resources[11], in);
		slots.GetGPrev_SpecHitDistForTracking() = resolve_srv(nrd_hal, compute, dispatch.resources[12], in);
		slots.GetGIn_SpecHitDistForTracking() = resolve_srv(nrd_hal, compute, dispatch.resources[13], in);
		slots.GetGOut_Data1() = resolve_uav(nrd_hal, compute, dispatch.resources[14], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[15], in);
		slots.GetGOut_SpecFast() = resolve_uav(nrd_hal, compute, dispatch.resources[16], in);
		slots.GetGOut_SpecHitDistForTracking() = resolve_uav(nrd_hal, compute, dispatch.resources[17], in);
		slots.GetGOut_Data2() = resolve_uav(nrd_hal, compute, dispatch.resources[18], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalAccumulation_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_historyfix(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 8);
		Slots::REBLUR_HistoryFixResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_DiffFast() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_DiffFast() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_HistoryFix>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_historyfix -- one extra input
	// (gIn_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.sig's REBLUR_HistoryFixSpecularResources comment.
	static void dispatch_reblur_historyfix_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::REBLUR_HistoryFixSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_SpecFast() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGIn_SpecHitDistForTracking() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGOut_SpecFast() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_HistoryFix_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_blur(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::REBLUR_BlurResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_ViewZ() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_Blur>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_blur -- same resource count
	// (5 in + 2 out, Diff->Spec renamed), see nrd_sig_test.sig's
	// REBLUR_BlurSpecularResources comment.
	static void dispatch_reblur_blur_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::REBLUR_BlurSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_ViewZ() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_Blur_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_postblur_ts0(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::REBLUR_PostBlurTS0Resources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_InternalData() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGOut_DiffCopy() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS0>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_postblur_ts0 -- same
	// resource count (5 in + 4 out, Diff->Spec renamed), see
	// nrd_sig_test.sig's REBLUR_PostBlurTS0SpecularResources comment.
	static void dispatch_reblur_postblur_ts0_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::REBLUR_PostBlurTS0SpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_InternalData() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGOut_SpecCopy() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS0_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_postblur_ts1(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::REBLUR_PostBlurTS1Resources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS1>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_postblur_ts1 -- same
	// resource count (5 in + 2 out, Diff->Spec renamed), see
	// nrd_sig_test.sig's REBLUR_PostBlurTS1SpecularResources comment.
	static void dispatch_reblur_postblur_ts1_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::REBLUR_PostBlurTS1SpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS1_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_temporalstabilization(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 11);
		Slots::REBLUR_TemporalStabilizationResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Data2() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGHistory_DiffLumaStabilized() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGInOut_Mv() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGOut_InternalData() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[9], in);
		slots.GetGOut_DiffLumaStabilized() = resolve_uav(nrd_hal, compute, dispatch.resources[10], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalStabilization>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_temporalstabilization -- one
	// extra input (gIn_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.sig's REBLUR_TemporalStabilizationSpecularResources comment.
	static void dispatch_reblur_temporalstabilization_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 12);
		Slots::REBLUR_TemporalStabilizationSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Data2() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_SpecHitDistForTracking() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGHistory_SpecLumaStabilized() = resolve_srv(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGInOut_Mv() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		slots.GetGOut_InternalData() = resolve_uav(nrd_hal, compute, dispatch.resources[9], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[10], in);
		slots.GetGOut_SpecLumaStabilized() = resolve_uav(nrd_hal, compute, dispatch.resources[11], in);
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalStabilization_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// TEMP: force-creates every item-7 PSO once so ComputePipelineState::
	// on_change()'s slots.empty() assert (HAL.D3D12.PipelineState.cpp) runs
	// for all 21 kernels, not just Clear (the only one execute()'s dispatch
	// loop actually issues today) -- see CLAUDE.md, remove once item 7 is
	// confirmed done.
	static void force_create_all_psos(HAL::ComputeContext& compute)
	{
		compute.set_pipeline<PSOS::NRD_Clear_UInt4>();
		compute.set_pipeline<PSOS::NRD_SIGMA_ClassifyTiles>();
		compute.set_pipeline<PSOS::NRD_SIGMA_SmoothTiles>();
		compute.set_pipeline<PSOS::NRD_SIGMA_Copy>();
		compute.set_pipeline<PSOS::NRD_SIGMA_BlurFirstPass0>();
		compute.set_pipeline<PSOS::NRD_SIGMA_BlurFirstPass1>();
		compute.set_pipeline<PSOS::NRD_SIGMA_TemporalStabilization>();
		compute.set_pipeline<PSOS::NRD_SIGMA_SplitScreen>();
		compute.set_pipeline<PSOS::NRD_REBLUR_ClassifyTiles>();
		compute.set_pipeline<PSOS::NRD_REBLUR_HitDistReconstruction>();
		compute.set_pipeline<PSOS::NRD_REBLUR_HitDistReconstruction5x5>();
		compute.set_pipeline<PSOS::NRD_REBLUR_PrePass>();
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalAccumulation>();
		compute.set_pipeline<PSOS::NRD_REBLUR_HistoryFix>();
		compute.set_pipeline<PSOS::NRD_REBLUR_Blur>();
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS0>();
		compute.set_pipeline<PSOS::NRD_REBLUR_PostBlurTS1>();
		compute.set_pipeline<PSOS::NRD_REBLUR_TemporalStabilization>();
		compute.set_pipeline<PSOS::NRD_REBLUR_SplitScreen>();
		compute.set_pipeline<PSOS::NRD_REBLUR_Validation>();
	}

	void NRD::execute(HAL::CommandList& list, const NRDFrameInputs& inputs)
	{
		if (!resolved || !pools_ready()) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);

		nrd::CommonSettings common{};
		common.resourceSize[0] = common.resourceSizePrev[0] = common.rectSize[0] = common.rectSizePrev[0] = (uint16_t)pools_render_size.x;
		common.resourceSize[1] = common.resourceSizePrev[1] = common.rectSize[1] = common.rectSizePrev[1] = (uint16_t)pools_render_size.y;
		memcpy(common.worldToViewMatrix, inputs.world_to_view, sizeof(common.worldToViewMatrix));
		memcpy(common.worldToViewMatrixPrev, inputs.world_to_view_prev, sizeof(common.worldToViewMatrixPrev));
		memcpy(common.viewToClipMatrix, inputs.view_to_clip, sizeof(common.viewToClipMatrix));
		memcpy(common.viewToClipMatrixPrev, inputs.view_to_clip_prev, sizeof(common.viewToClipMatrixPrev));
		common.motionVectorScale[0] = 1.0f;
		common.motionVectorScale[1] = 1.0f;
		common.motionVectorScale[2] = 0.0f; // 2D screen-space MV, see NRD_GBufferPackParams' comment
		common.cameraJitter[0] = inputs.jitter.x;
		common.cameraJitter[1] = inputs.jitter.y;
		common.cameraJitterPrev[0] = inputs.jitter_prev.x;
		common.cameraJitterPrev[1] = inputs.jitter_prev.y;
		common.frameIndex = frame_counter++;
		common.isMotionVectorInWorldSpace = false;
		nrd::SetCommonSettings(*instance, common);

		nrd::SigmaSettings sigma_settings{};
		nrd::SetDenoiserSettings(*instance, 0, &sigma_settings);

		// diff_noisy/spec_noisy are default-constructed (null resource, see
		// Texture2DView::resource) whenever the caller (NRD_REBLUR_Execute)
		// didn't populate them -- g_indirect_denoiser/g_reflection_denoiser
		// aren't set to NRD this frame. Only requesting the identifiers whose
		// signal is actually wanted keeps those denoisers' dispatches out of
		// GetComputeDispatches() entirely, so resolve_srv/resolve_uav never
		// need a null-resource fallback for them.
		bool want_diffuse  = (bool)inputs.diff_noisy.resource;
		bool want_specular = (bool)inputs.spec_noisy.resource;

		// Library defaults throughout (see nrd_sig_test.sig's REBLURSharedConstants
		// comment and raytracing.hlsl's gHitDistParams -- both must move
		// together with hitDistanceParameters if this is ever tuned).
		nrd::ReblurSettings reblur_settings{};
		if (want_diffuse)
			nrd::SetDenoiserSettings(*instance, 1, &reblur_settings);
		if (want_specular)
			nrd::SetDenoiserSettings(*instance, 2, &reblur_settings);

		nrd::Identifier identifiers[3];
		uint32_t identifiers_num = 0;
		identifiers[identifiers_num++] = 0;
		if (want_diffuse)  identifiers[identifiers_num++] = 1;
		if (want_specular) identifiers[identifiers_num++] = 2;

		const nrd::DispatchDesc* dispatches = nullptr;
		uint32_t dispatches_num = 0;
		nrd::GetComputeDispatches(*instance, identifiers, identifiers_num, dispatches, dispatches_num);

		auto& compute = list.get_compute();

		force_create_all_psos(compute);

		uint32_t dispatched = 0;
		for (uint32_t d = 0; d < dispatches_num; ++d)
		{
			const nrd::DispatchDesc& dispatch = dispatches[d];
			const std::string& identifier = idesc.pipelines[dispatch.pipelineIndex].shaderIdentifier;

			// identifier 1 = REBLUR_DIFFUSE, identifier 2 = REBLUR_SPECULAR
			// (see the ctor's denoisers[] array) -- NRD reports the SAME
			// kernel-name prefix for both signals' permutations of a given
			// REBLUR kernel (the NRD_SIGNAL=DIFF/SPEC marker is a `|`-suffix
			// on the identifier string, not the prefix these starts_with()
			// checks match on), so dispatch.identifier is what actually
			// distinguishes which (differently-shaped, see nrd_sig_test.sig's
			// per-kernel Specular struct comments) resource list this
			// dispatch carries.
			bool is_specular = dispatch.identifier == 2;

			if (identifier == "Clear.cs.hlsl|FLOAT=1")
				dispatch_clear(*this, compute, dispatch);
			else if (identifier.starts_with("REBLUR_ClassifyTiles.cs.hlsl"))
				dispatch_reblur_classifytiles(*this, compute, dispatch, inputs);
			else if (identifier.starts_with("REBLUR_HitDistReconstruction.cs.hlsl") && identifier.find("MODE_5X5=1") != std::string::npos)
			{
				if (is_specular)
					dispatch_reblur_hitdistreconstruction_specular<PSOS::NRD_REBLUR_HitDistReconstruction5x5_Specular>(*this, compute, dispatch, inputs);
				else
					dispatch_reblur_hitdistreconstruction<PSOS::NRD_REBLUR_HitDistReconstruction5x5>(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_HitDistReconstruction.cs.hlsl"))
			{
				if (is_specular)
					dispatch_reblur_hitdistreconstruction_specular<PSOS::NRD_REBLUR_HitDistReconstruction_Specular>(*this, compute, dispatch, inputs);
				else
					dispatch_reblur_hitdistreconstruction<PSOS::NRD_REBLUR_HitDistReconstruction>(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_PrePass.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_prepass_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_prepass(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_TemporalAccumulation.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_temporalaccumulation_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_temporalaccumulation(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_HistoryFix.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_historyfix_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_historyfix(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_Blur.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_blur_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_blur(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_PostBlur.cs.hlsl") && identifier.find("TEMPORAL_STABILIZATION=0") != std::string::npos)
			{
				if (is_specular) dispatch_reblur_postblur_ts0_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_postblur_ts0(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_PostBlur.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_postblur_ts1_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_postblur_ts1(*this, compute, dispatch, inputs);
			}
			else if (identifier.starts_with("REBLUR_TemporalStabilization.cs.hlsl"))
			{
				if (is_specular) dispatch_reblur_temporalstabilization_specular(*this, compute, dispatch, inputs);
				else              dispatch_reblur_temporalstabilization(*this, compute, dispatch, inputs);
			}
			else
			{
				// SIGMA_SHADOW's kernels, and anything else not wired (out of
				// scope, see [[project-nrd-integration]]) -- skipped, not an
				// error.
				continue;
			}
			++dispatched;
		}

		Log::get() << "[NRD] execute(): " << dispatched << "/" << dispatches_num << " dispatches issued" << Log::endl;
	}
}
