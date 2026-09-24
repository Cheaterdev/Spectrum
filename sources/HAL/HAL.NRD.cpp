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

	// See HAL.NRD.ixx's own comment on active_permanent_pool/active_transient_pool
	// for why these must be thread_local (a genuine cross-queue data race
	// otherwise, not a hypothetical one).
	thread_local std::vector<HAL::TextureResource::ptr>* NRD::active_permanent_pool = nullptr;
	thread_local std::vector<HAL::TextureResource::ptr>* NRD::active_transient_pool = nullptr;

	NRD::NRD()
	{
		const nrd::LibraryDesc& lib = *nrd::GetLibraryDesc();
		Log::get() << "[NRD] library v" << (int)lib.versionMajor << "." << (int)lib.versionMinor
		           << "." << (int)lib.versionBuild << ", " << lib.supportedDenoisersNum
		           << " denoisers supported" << Log::endl;

		// Two independent Instances, not one shared 3-denoiser Instance -- see
		// HAL.NRD.ixx's own comment on why (independent SetupConditions,
		// NRD's one-frameIndex-increment-per-Instance-per-frame requirement).
		//
		// REBLUR_DIFFUSE for indirect GI (RTXIndirectNoise, IndirectRTX,
		// voxel.prism). REBLUR_SPECULAR for reflections (RTXReflectionNoise/
		// VoxelReflectionNoise) -- a separate denoiser rather than the
		// combined REBLUR_DIFFUSE_SPECULAR method, so either signal can be
		// denoised independently of the other (g_indirect_denoiser and
		// g_reflection_denoiser are independent toggles, see
		// [[project-nrd-integration]]).
		static const nrd::DenoiserDesc reblur_denoisers[] = {
			{ 0, nrd::Denoiser::REBLUR_DIFFUSE },
			{ 1, nrd::Denoiser::REBLUR_SPECULAR }
		};
		nrd::InstanceCreationDesc reblur_desc{};
		reblur_desc.denoisers = reblur_denoisers;
		reblur_desc.denoisersNum = 2;

		const nrd::Result reblur_res = nrd::CreateInstance(reblur_desc, reblur_instance);
		reblur_resolved = reblur_res == nrd::Result::SUCCESS;
		if (reblur_resolved)
			Log::get() << "[NRD] REBLUR instance created" << Log::endl;
		else
			Log::get() << "[NRD] REBLUR CreateInstance failed (" << (int)reblur_res << ")" << Log::endl;

		// SIGMA_SHADOW for VSM's non-penumbra fallback (VSM_Combine/
		// NRD_SIGMA_Execute, vsm.prism/nrd_sig_test.prism).
		static const nrd::DenoiserDesc sigma_denoisers[] = {
			{ 0, nrd::Denoiser::SIGMA_SHADOW }
		};
		nrd::InstanceCreationDesc sigma_desc{};
		sigma_desc.denoisers = sigma_denoisers;
		sigma_desc.denoisersNum = 1;

		const nrd::Result sigma_res = nrd::CreateInstance(sigma_desc, sigma_instance);
		sigma_resolved = sigma_res == nrd::Result::SUCCESS;
		if (sigma_resolved)
			Log::get() << "[NRD] SIGMA instance created" << Log::endl;
		else
			Log::get() << "[NRD] SIGMA CreateInstance failed (" << (int)sigma_res << ")" << Log::endl;
	}

	NRD::~NRD()
	{
		if (reblur_instance)
			nrd::DestroyInstance(*reblur_instance);
		if (sigma_instance)
			nrd::DestroyInstance(*sigma_instance);
	}

	void NRD::smoke_test() const
	{
		if (reblur_resolved)
		{
			const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*reblur_instance);
			Log::get() << "[NRD] REBLUR instance desc: " << idesc.pipelinesNum << " pipelines, "
			           << idesc.permanentPoolSize << " permanent pool textures, "
			           << idesc.transientPoolSize << " transient pool textures, "
			           << idesc.constantBufferMaxDataSize << " max CB bytes" << Log::endl;
		}
		if (sigma_resolved)
		{
			const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*sigma_instance);
			Log::get() << "[NRD] SIGMA instance desc: " << idesc.pipelinesNum << " pipelines, "
			           << idesc.permanentPoolSize << " permanent pool textures, "
			           << idesc.transientPoolSize << " transient pool textures, "
			           << idesc.constantBufferMaxDataSize << " max CB bytes" << Log::endl;
		}
	}

	void NRD::ensure_pools(HAL::Device& device, uint2 render_size)
	{
		if (!available()) return;
		if (pools_render_size == render_size && pools_ready()) return;

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

		uint32_t total_permanent = 0, total_transient = 0;
		if (reblur_resolved)
		{
			const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*reblur_instance);
			make_pool(idesc.permanentPool, idesc.permanentPoolSize, reblur_permanent_pool);
			make_pool(idesc.transientPool, idesc.transientPoolSize, reblur_transient_pool);
			total_permanent += (uint32_t)reblur_permanent_pool.size();
			total_transient += (uint32_t)reblur_transient_pool.size();
		}
		if (sigma_resolved)
		{
			const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*sigma_instance);
			make_pool(idesc.permanentPool, idesc.permanentPoolSize, sigma_permanent_pool);
			make_pool(idesc.transientPool, idesc.transientPoolSize, sigma_transient_pool);
			total_permanent += (uint32_t)sigma_permanent_pool.size();
			total_transient += (uint32_t)sigma_transient_pool.size();
		}

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
		reblur_needs_history_reset = true;
		sigma_needs_history_reset = true;

		Log::get() << "[NRD] pools allocated: " << total_permanent << " permanent, "
		           << total_transient << " transient, at " << render_size.x << "x" << render_size.y << Log::endl;
	}

	// PERMANENT_POOL/TRANSIENT_POOL resolve to the real pool textures, from
	// whichever pool pair execute_reblur()/execute_shadow() most recently made
	// active (see HAL.NRD.ixx's own comment on active_permanent_pool/
	// active_transient_pool). Every other (named) resource type this
	// integration doesn't wire routes to a dummy directly.
	HAL::TextureResource::ptr NRD::resolve_pool_resource(const nrd::ResourceDesc& r) const
	{
		if (r.type == nrd::ResourceType::PERMANENT_POOL)
			return (*active_permanent_pool)[r.indexInPool];
		if (r.type == nrd::ResourceType::TRANSIENT_POOL)
			return (*active_transient_pool)[r.indexInPool];

		return r.descriptorType == nrd::DescriptorType::TEXTURE ? dummy_srv : dummy_uav;
	}

	// Clear.cs.hlsl|FLOAT=1 -- first ported kernel (see nrd_sig_test.prism,
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
		Slots::Denoise::NRD::Clear_Constants slots;

		auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
		HLSL::RWTexture2D<float4> view(h);
		view.create(output, 0, 0);
		slots.GetGOut() = view;

		compute.set_pipeline<PSOS::Denoise::NRD::NRD_Clear_Test>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// Clear.cs.hlsl|FLOAT=0 -- the uint4 permutation (nrd_sig_test.prism's
	// Clear_UInt4Resources/NRD_Clear_UInt4, sig_clear_uint4.hlsl), for
	// integer-format pool resources (e.g. SIGMA's gOut_HistoryLength,
	// RWTexture2D<uint>) that dispatch_clear's float4-typed view can't
	// correctly target. Otherwise identical to dispatch_clear -- the view's
	// actual UAV format always comes from the resource's own native format
	// (RWTexture2D<T>::create(), HAL.HLSL.ixx), not from T, so the only
	// reason this needs a separate function is HLSL-side type matching
	// (gOut[pixelPos] = 0 must compile against the shader's declared uint4
	// vs float4 element type).
	static void dispatch_clear_uint4(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch)
	{
		ASSERT(dispatch.resourcesNum == 1);
		HAL::TextureResource::ptr output = nrd_hal.resolve_pool_resource(dispatch.resources[0]);

		ASSERT(dispatch.constantBufferDataSize == 0);
		Slots::Denoise::NRD::Clear_UInt4Resources slots;

		auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
		HLSL::RWTexture2D<uint4> view(h);
		view.create(output, 0, 0);
		slots.GetGOut() = view;

		compute.set_pipeline<PSOS::Denoise::NRD::NRD_Clear_UInt4>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// Item 8 (see [[project-nrd-integration]]): real REBLUR_DIFFUSE dispatch
	// wiring. resolve_srv/resolve_uav resolve one nrd::ResourceDesc entry
	// (from a DispatchDesc::resources[] array, walked position-for-position
	// against each kernel's .prism struct field order below -- both are
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
		case nrd::ResourceType::IN_PENUMBRA:            return in.penumbra_noisy.texture2D;
		// SIGMA reads its own external output back as history (BlurFirstPass0's
		// gIn_Shadow_Translucency, TemporalStabilization's gIn_Shadow_Translucency)
		// whenever stabilizationStrength != 0 -- see NRDDescs.h's own
		// OUT_SHADOW_TRANSLUCENCY comment -- so this SRV case and OUT_
		// SHADOW_TRANSLUCENCY's UAV case below both resolve to shadow_denoised.
		case nrd::ResourceType::OUT_SHADOW_TRANSLUCENCY: return in.shadow_denoised.texture2D;
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
		case nrd::ResourceType::OUT_SHADOW_TRANSLUCENCY: return in.shadow_denoised.rwTexture2D;
		default:
			ASSERT(!"unhandled named UAV resource type -- see HAL.NRD.cpp's resolve_uav comment");
			auto h = compute.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });
			HLSL::RWTexture2D<> view(h);
			view.create(nrd_hal.dummy_uav_resource(), 0, 0);
			return view;
		}
	}

	// SIGMASharedConstants is #pragma pack(push,1) with fields in the exact
	// order/type of SIGMA_Config.hlsli's SIGMA_SHARED_CONSTANTS macro (see
	// nrd_sig_test.prism's comment) -- verified by hand against HLSL's default
	// cbuffer packing rules (no field here straddles a 16-byte boundary), so
	// NRD's own raw constantBufferData blob can be copied onto it directly.
	// The size assert is the real safety net, same reasoning as
	// fill_reblur_shared_constants below.
	static void fill_sigma_shared_constants(Table::Denoise::NRD::SIGMASharedConstants& dst, const nrd::DispatchDesc& dispatch)
	{
		// Tightly-packed sizeof is 516 bytes; NRD's raw blob is padded to a
		// 16-byte multiple (528, confirmed at runtime -- matches
		// idesc.constantBufferMaxDataSize logged at instance creation).
		ASSERT(dispatch.constantBufferDataSize >= sizeof(Table::Denoise::NRD::SIGMASharedConstants)
			&& dispatch.constantBufferDataSize < sizeof(Table::Denoise::NRD::SIGMASharedConstants) + 16);
		memcpy(&dst, dispatch.constantBufferData, sizeof(Table::Denoise::NRD::SIGMASharedConstants));
	}

	// SIGMA_SHADOW dispatch wiring, same resolve_srv/resolve_uav plumbing and
	// positional dispatch.resources[] -> .prism struct field order convention
	// as REBLUR above, now with real shared constants too (see
	// fill_sigma_shared_constants above -- every SIGMA kernel's own
	// .resources.hlsli includes the same SIGMA_SHARED_CONSTANTS block
	// unconditionally, confirmed against NRD's vendored source, so every
	// dispatch function below fills it the same way).
	static void dispatch_sigma_classifytiles(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 3);
		Slots::Denoise::NRD::SIGMA_ClassifyTilesResources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Penumbra() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGOut_Tiles() = resolve_uav(nrd_hal, compute, dispatch.resources[2], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_ClassifyTiles>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_sigma_smoothtiles(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 2);
		Slots::Denoise::NRD::SIGMA_SmoothTilesResources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGOut_Tiles() = resolve_uav(nrd_hal, compute, dispatch.resources[1], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_SmoothTiles>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_sigma_copy(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::Denoise::NRD::SIGMA_CopyResources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_History() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_HistoryLength() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGOut_History() = resolve_uav(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_HistoryLength() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_Copy>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// FIRST_PASS=0 permutation of SIGMA_Blur.cs.hlsl -- gIn_Shadow_Translucency
	// (the previous frame's OUT_SHADOW_TRANSLUCENCY, read back as history) IS
	// compiled in, see nrd_sig_test.prism's SIGMA_BlurFirstPass0Resources comment.
	static void dispatch_sigma_blur_firstpass0(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::Denoise::NRD::SIGMA_BlurFirstPass0Resources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Penumbra() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Shadow_Translucency() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Penumbra() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Shadow_Translucency() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_BlurFirstPass0>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// FIRST_PASS=1 permutation -- no gIn_Shadow_Translucency (one fewer input
	// than FIRST_PASS=0), see nrd_sig_test.prism's SIGMA_BlurFirstPass1Resources
	// comment.
	static void dispatch_sigma_blur_firstpass1(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 6);
		Slots::Denoise::NRD::SIGMA_BlurFirstPass1Resources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Penumbra() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Penumbra() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Shadow_Translucency() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_BlurFirstPass1>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_sigma_temporalstabilization(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::Denoise::NRD::SIGMA_TemporalStabilizationResources slots;
		fill_sigma_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Mv() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Penumbra() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Shadow_Translucency() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_History() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_HistoryLength() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_Shadow_Translucency() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		slots.GetGOut_HistoryLength() = resolve_uav(nrd_hal, compute, dispatch.resources[8], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_SIGMA_TemporalStabilization>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLURSharedConstants is #pragma pack(push,1) with fields in the exact
	// order/type of REBLUR_Config.hlsli's REBLUR_SHARED_CONSTANTS macro (see
	// nrd_sig_test.prism's comment) -- verified by hand against HLSL's default
	// cbuffer packing rules (no field here straddles a 16-byte boundary, so
	// zero implicit padding is needed at any point in this exact sequence),
	// so NRD's own raw constantBufferData blob can be copied onto it
	// directly instead of 77 field-by-field assignments. The size assert is
	// the real safety net: if it ever fires, the packing assumption above is
	// wrong for some field ordering and needs revisiting, not the assert
	// weakened away.
	static void fill_reblur_shared_constants(Table::Denoise::NRD::REBLURSharedConstants& dst, const nrd::DispatchDesc& dispatch)
	{
		// HLSL cbuffers are sized in multiples of 16 bytes overall; NRD's
		// raw blob is padded to that (864 bytes, confirmed at runtime),
		// 8 bytes past this struct's own tightly-packed (pack(1), no
		// trailing pad) sizeof (856) -- the size fields themselves (77
		// fields, no straddling boundary, see this function's header
		// comment) still match field-for-field, only the very end differs.
		// Only copy this struct's own byte count; the trailing pad is
		// unused on the C++ side either way.
		ASSERT(dispatch.constantBufferDataSize >= sizeof(Table::Denoise::NRD::REBLURSharedConstants)
			&& dispatch.constantBufferDataSize < sizeof(Table::Denoise::NRD::REBLURSharedConstants) + 16);
		memcpy(&dst, dispatch.constantBufferData, sizeof(Table::Denoise::NRD::REBLURSharedConstants));
	}

	static void dispatch_reblur_classifytiles(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 2);
		Slots::Denoise::NRD::REBLUR_ClassifyTilesResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGOut_Tiles() = resolve_uav(nrd_hal, compute, dispatch.resources[1], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_ClassifyTiles>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// Shared by both HitDistReconstruction PSOs (MODE_5X5=0/1 -- identical
	// resource layout, see nrd_sig_test.prism's comment on the 5x5 PSO).
	template<typename PSO>
	static void dispatch_reblur_hitdistreconstruction(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::Denoise::NRD::REBLUR_HitDistReconstructionResources slots;
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
	// nrd_sig_test.prism's REBLUR_HitDistReconstructionSpecularResources
	// comment.
	template<typename PSO>
	static void dispatch_reblur_hitdistreconstruction_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 5);
		Slots::Denoise::NRD::REBLUR_HitDistReconstructionSpecularResources slots;
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
		Slots::Denoise::NRD::REBLUR_PrePassResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PrePass>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_prepass -- one extra output
	// (gOut_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.prism's REBLUR_PrePassSpecularResources comment.
	static void dispatch_reblur_prepass_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 6);
		Slots::Denoise::NRD::REBLUR_PrePassSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_SpecHitDistForTracking() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PrePass_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_temporalaccumulation(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 16);
		Slots::Denoise::NRD::REBLUR_TemporalAccumulationResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_TemporalAccumulation>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_temporalaccumulation -- see
	// nrd_sig_test.prism's REBLUR_TemporalAccumulationSpecularResources comment
	// for the field-list differences vs diffuse (14 in + 5 out here, vs
	// diffuse's 12 in + 4 out).
	static void dispatch_reblur_temporalaccumulation_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 19);
		Slots::Denoise::NRD::REBLUR_TemporalAccumulationSpecularResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_TemporalAccumulation_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_historyfix(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 8);
		Slots::Denoise::NRD::REBLUR_HistoryFixResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGIn_DiffFast() = resolve_srv(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		slots.GetGOut_DiffFast() = resolve_uav(nrd_hal, compute, dispatch.resources[7], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_HistoryFix>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_historyfix -- one extra input
	// (gIn_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.prism's REBLUR_HistoryFixSpecularResources comment.
	static void dispatch_reblur_historyfix_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::Denoise::NRD::REBLUR_HistoryFixSpecularResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_HistoryFix_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_blur(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::Denoise::NRD::REBLUR_BlurResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_ViewZ() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_Blur>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_blur -- same resource count
	// (5 in + 2 out, Diff->Spec renamed), see nrd_sig_test.prism's
	// REBLUR_BlurSpecularResources comment.
	static void dispatch_reblur_blur_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::Denoise::NRD::REBLUR_BlurSpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_ViewZ() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_Blur_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_postblur_ts0(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::Denoise::NRD::REBLUR_PostBlurTS0Resources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PostBlurTS0>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_postblur_ts0 -- same
	// resource count (5 in + 4 out, Diff->Spec renamed), see
	// nrd_sig_test.prism's REBLUR_PostBlurTS0SpecularResources comment.
	static void dispatch_reblur_postblur_ts0_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 9);
		Slots::Denoise::NRD::REBLUR_PostBlurTS0SpecularResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PostBlurTS0_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_postblur_ts1(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::Denoise::NRD::REBLUR_PostBlurTS1Resources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Diff() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Diff() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PostBlurTS1>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_postblur_ts1 -- same
	// resource count (5 in + 2 out, Diff->Spec renamed), see
	// nrd_sig_test.prism's REBLUR_PostBlurTS1SpecularResources comment.
	static void dispatch_reblur_postblur_ts1_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 7);
		Slots::Denoise::NRD::REBLUR_PostBlurTS1SpecularResources slots;
		fill_reblur_shared_constants(slots.GetSharedConstants(), dispatch);
		slots.GetGIn_Tiles() = resolve_srv(nrd_hal, compute, dispatch.resources[0], in);
		slots.GetGIn_Normal_Roughness() = resolve_srv(nrd_hal, compute, dispatch.resources[1], in);
		slots.GetGIn_Data1() = resolve_srv(nrd_hal, compute, dispatch.resources[2], in);
		slots.GetGIn_ViewZ() = resolve_srv(nrd_hal, compute, dispatch.resources[3], in);
		slots.GetGIn_Spec() = resolve_srv(nrd_hal, compute, dispatch.resources[4], in);
		slots.GetGOut_Normal_Roughness() = resolve_uav(nrd_hal, compute, dispatch.resources[5], in);
		slots.GetGOut_Spec() = resolve_uav(nrd_hal, compute, dispatch.resources[6], in);
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_PostBlurTS1_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	static void dispatch_reblur_temporalstabilization(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 11);
		Slots::Denoise::NRD::REBLUR_TemporalStabilizationResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_TemporalStabilization>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	// REBLUR_SPECULAR sibling of dispatch_reblur_temporalstabilization -- one
	// extra input (gIn_SpecHitDistForTracking, no diffuse equivalent), see
	// nrd_sig_test.prism's REBLUR_TemporalStabilizationSpecularResources comment.
	static void dispatch_reblur_temporalstabilization_specular(nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs& in)
	{
		ASSERT(dispatch.resourcesNum == 12);
		Slots::Denoise::NRD::REBLUR_TemporalStabilizationSpecularResources slots;
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
		compute.set_pipeline<PSOS::Denoise::NRD::NRD_REBLUR_TemporalStabilization_Specular>();
		compute.set(slots);
		compute.dispatch((int)dispatch.gridWidth, (int)dispatch.gridHeight, 1);
	}

	using DispatchFn = void(*)(nvidia::NRD&, HAL::ComputeContext&, const nrd::DispatchDesc&, const nvidia::NRDFrameInputs&);
	struct DispatchFnPair { DispatchFn diffuse = nullptr; DispatchFn specular = nullptr; };

	// Resolves dispatch.pipelineIndex -> the dispatch_reblur_* function to
	// call, once, instead of running the starts_with()/find() chain below
	// per-dispatch every frame. Safe to cache: idesc.pipelines is a fixed
	// table for the lifetime of the nrd::Instance (built once in
	// nrd::CreateInstance(), not regenerated per frame), so pipelineIndex
	// always names the same shaderIdentifier string for as long as this
	// Instance lives. is_specular is a separate axis -- NRD reports the SAME
	// kernel-name prefix for both signals' permutations of a given REBLUR
	// kernel (see execute()'s own comment on dispatch.identifier) -- hence
	// the pair instead of one function per pipelineIndex.
	static std::vector<DispatchFnPair> build_dispatch_table(const nrd::InstanceDesc& idesc)
	{
		std::vector<DispatchFnPair> table(idesc.pipelinesNum);

		for (uint32_t i = 0; i < idesc.pipelinesNum; ++i)
		{
			const std::string& identifier = idesc.pipelines[i].shaderIdentifier;
			DispatchFnPair entry;

			if (identifier == "Clear.cs.hlsl|FLOAT=1")
			{
				entry.diffuse = entry.specular = [](nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs&)
				{
					dispatch_clear(nrd_hal, compute, dispatch);
				};
			}
			else if (identifier == "Clear.cs.hlsl|FLOAT=0")
			{
				entry.diffuse = entry.specular = [](nvidia::NRD& nrd_hal, HAL::ComputeContext& compute, const nrd::DispatchDesc& dispatch, const nvidia::NRDFrameInputs&)
				{
					dispatch_clear_uint4(nrd_hal, compute, dispatch);
				};
			}
			else if (identifier.starts_with("REBLUR_ClassifyTiles.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_reblur_classifytiles;
			}
			else if (identifier.starts_with("REBLUR_HitDistReconstruction.cs.hlsl") && identifier.contains("MODE_5X5=1"))
			{
				entry.diffuse  = &dispatch_reblur_hitdistreconstruction<PSOS::Denoise::NRD::NRD_REBLUR_HitDistReconstruction5x5>;
				entry.specular = &dispatch_reblur_hitdistreconstruction_specular<PSOS::Denoise::NRD::NRD_REBLUR_HitDistReconstruction5x5_Specular>;
			}
			else if (identifier.starts_with("REBLUR_HitDistReconstruction.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_hitdistreconstruction<PSOS::Denoise::NRD::NRD_REBLUR_HitDistReconstruction>;
				entry.specular = &dispatch_reblur_hitdistreconstruction_specular<PSOS::Denoise::NRD::NRD_REBLUR_HitDistReconstruction_Specular>;
			}
			else if (identifier.starts_with("REBLUR_PrePass.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_prepass;
				entry.specular = &dispatch_reblur_prepass_specular;
			}
			else if (identifier.starts_with("REBLUR_TemporalAccumulation.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_temporalaccumulation;
				entry.specular = &dispatch_reblur_temporalaccumulation_specular;
			}
			else if (identifier.starts_with("REBLUR_HistoryFix.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_historyfix;
				entry.specular = &dispatch_reblur_historyfix_specular;
			}
			else if (identifier.starts_with("REBLUR_Blur.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_blur;
				entry.specular = &dispatch_reblur_blur_specular;
			}
			else if (identifier.starts_with("REBLUR_PostBlur.cs.hlsl") && identifier.contains("TEMPORAL_STABILIZATION=0"))
			{
				entry.diffuse  = &dispatch_reblur_postblur_ts0;
				entry.specular = &dispatch_reblur_postblur_ts0_specular;
			}
			else if (identifier.starts_with("REBLUR_PostBlur.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_postblur_ts1;
				entry.specular = &dispatch_reblur_postblur_ts1_specular;
			}
			else if (identifier.starts_with("REBLUR_TemporalStabilization.cs.hlsl"))
			{
				entry.diffuse  = &dispatch_reblur_temporalstabilization;
				entry.specular = &dispatch_reblur_temporalstabilization_specular;
			}
			else if (identifier.starts_with("SIGMA_ClassifyTiles.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_classifytiles;
			}
			else if (identifier.starts_with("SIGMA_SmoothTiles.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_smoothtiles;
			}
			else if (identifier.starts_with("SIGMA_Copy.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_copy;
			}
			else if (identifier.starts_with("SIGMA_Blur.cs.hlsl") && identifier.contains("FIRST_PASS=1"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_blur_firstpass1;
			}
			else if (identifier.starts_with("SIGMA_Blur.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_blur_firstpass0;
			}
			else if (identifier.starts_with("SIGMA_TemporalStabilization.cs.hlsl"))
			{
				entry.diffuse = entry.specular = &dispatch_sigma_temporalstabilization;
			}
			// else: SIGMA_SplitScreen (never requested at default
			// CommonSettings::splitScreen=0, same as REBLUR_SplitScreen/
			// REBLUR_Validation above -- neither of those is wired either),
			// and anything else not wired (out of scope, see
			// [[project-nrd-integration]]) -- entry stays {null,null},
			// execute() skips those pipelineIndex values.

			table[i] = entry;
		}

		return table;
	}

	static void fill_common_settings(nrd::CommonSettings& common, const nvidia::NRDFrameInputs& inputs, uint2 pools_render_size, uint32_t frame_index, bool history_reset)
	{
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
		common.frameIndex = frame_index;
		common.isMotionVectorInWorldSpace = false;
		// Pool textures were just (re)allocated this frame (ensure_pools()'s
		// reblur_needs_history_reset/sigma_needs_history_reset) -- undefined
		// GPU memory, not zero-initialized, so tell NRD to discard whatever
		// history it thinks it has and clear its own pool resources rather
		// than reproject/blend against garbage. SetCommonSettings() (NRD's
		// own InstanceImpl.cpp) additionally forces resourceSizePrev/
		// worldToViewMatrixPrev/cameraJitterPrev to match this frame's
		// current values whenever accumulationMode != CONTINUE, so the
		// resourceSizePrev/rectSizePrev set to the CURRENT (not real
		// previous) size above is only ever wrong on this same reset frame,
		// where NRD immediately overwrites it anyway.
		common.accumulationMode = history_reset ? nrd::AccumulationMode::CLEAR_AND_RESTART : nrd::AccumulationMode::CONTINUE;
	}

	void NRD::execute_reblur(HAL::CommandList& list, const NRDFrameInputs& inputs)
	{
		if (!reblur_resolved || !pools_ready()) return;

		// diff_noisy/spec_noisy are default-constructed (null resource, see
		// Texture2DView::resource) whenever the caller (NRD_REBLUR_Execute)
		// didn't populate them -- g_indirect_denoiser/g_reflection_denoiser
		// aren't set to NRD this frame. Only requesting the identifiers whose
		// signal is actually wanted keeps those denoisers' dispatches out of
		// GetComputeDispatches() entirely, so resolve_srv/resolve_uav never
		// need a null-resource fallback for them.
		bool want_diffuse  = (bool)inputs.diff_noisy.resource;
		bool want_specular = (bool)inputs.spec_noisy.resource;
		if (!want_diffuse && !want_specular) return;

		PROFILE_GPU(L"NRD_REBLUR");

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*reblur_instance);

		// idesc.pipelines never changes after nrd::CreateInstance() -- so this
		// table is correct to build exactly once and reuse for every
		// subsequent execute_reblur() call/frame.
		static const std::vector<DispatchFnPair> dispatch_table = build_dispatch_table(idesc);

		nrd::CommonSettings common{};
		fill_common_settings(common, inputs, pools_render_size, reblur_frame_counter++, reblur_needs_history_reset);
		reblur_needs_history_reset = false;
		nrd::SetCommonSettings(*reblur_instance, common);

		// Library defaults throughout (see nrd_sig_test.prism's REBLURSharedConstants
		// comment and raytracing.hlsl's gHitDistParams -- both must move
		// together with hitDistanceParameters if this is ever tuned).
		nrd::ReblurSettings reblur_settings{};
		if (want_diffuse)
			nrd::SetDenoiserSettings(*reblur_instance, 0, &reblur_settings);
		if (want_specular)
			nrd::SetDenoiserSettings(*reblur_instance, 1, &reblur_settings);

		nrd::Identifier identifiers[2];
		uint32_t identifiers_num = 0;
		if (want_diffuse)  identifiers[identifiers_num++] = 0;
		if (want_specular) identifiers[identifiers_num++] = 1;

		const nrd::DispatchDesc* dispatches = nullptr;
		uint32_t dispatches_num = 0;
		nrd::GetComputeDispatches(*reblur_instance, identifiers, identifiers_num, dispatches, dispatches_num);

		auto& compute = list.get_compute();
		active_permanent_pool = &reblur_permanent_pool;
		active_transient_pool = &reblur_transient_pool;

		for (uint32_t d = 0; d < dispatches_num; ++d)
		{
			const nrd::DispatchDesc& dispatch = dispatches[d];

			// identifier 0 = REBLUR_DIFFUSE, identifier 1 = REBLUR_SPECULAR
			// (see the ctor's reblur_denoisers[] array) -- NRD reports the
			// SAME kernel-name prefix for both signals' permutations of a
			// given REBLUR kernel (the NRD_SIGNAL=DIFF/SPEC marker is a
			// `|`-suffix on the identifier string, not the prefix
			// build_dispatch_table's starts_with() checks match on), so
			// dispatch.identifier is what actually distinguishes which
			// (differently-shaped, see nrd_sig_test.prism's per-kernel
			// Specular struct comments) resource list this dispatch carries.
			bool is_specular = dispatch.identifier == 1;

			const DispatchFnPair& entry = dispatch_table[dispatch.pipelineIndex];
			DispatchFn fn = is_specular ? entry.specular : entry.diffuse;
			if (!fn)
			{
				// Anything not wired (REBLUR_SplitScreen/Validation, out of
				// scope, see [[project-nrd-integration]]) -- skipped, not an
				// error.
				continue;
			}

			fn(*this, compute, dispatch, inputs);
		}
	}

	void NRD::execute_shadow(HAL::CommandList& list, const NRDFrameInputs& inputs)
	{
		if (!sigma_resolved || !pools_ready()) return;

		bool want_shadow = (bool)inputs.penumbra_noisy.resource;
		if (!want_shadow) return;

		PROFILE_GPU(L"NRD_SIGMA");

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*sigma_instance);

		static const std::vector<DispatchFnPair> dispatch_table = build_dispatch_table(idesc);

		nrd::CommonSettings common{};
		fill_common_settings(common, inputs, pools_render_size, sigma_frame_counter++, sigma_needs_history_reset);
		sigma_needs_history_reset = false;
		nrd::SetCommonSettings(*sigma_instance, common);

		nrd::SigmaSettings sigma_settings{};
		sigma_settings.lightDirection[0] = inputs.sun_direction.x;
		sigma_settings.lightDirection[1] = inputs.sun_direction.y;
		sigma_settings.lightDirection[2] = inputs.sun_direction.z;
		nrd::SetDenoiserSettings(*sigma_instance, 0, &sigma_settings);

		nrd::Identifier identifiers[1] = { 0 };
		const nrd::DispatchDesc* dispatches = nullptr;
		uint32_t dispatches_num = 0;
		nrd::GetComputeDispatches(*sigma_instance, identifiers, 1, dispatches, dispatches_num);

		auto& compute = list.get_compute();
		active_permanent_pool = &sigma_permanent_pool;
		active_transient_pool = &sigma_transient_pool;

		for (uint32_t d = 0; d < dispatches_num; ++d)
		{
			const nrd::DispatchDesc& dispatch = dispatches[d];

			// Single identifier (0 = SIGMA_SHADOW) -- no diffuse/specular
			// split, always the .diffuse slot.
			const DispatchFnPair& entry = dispatch_table[dispatch.pipelineIndex];
			DispatchFn fn = entry.diffuse;
			if (!fn)
			{
				// SIGMA_SplitScreen (never requested at default
				// CommonSettings::splitScreen=0), and anything else not
				// wired (out of scope, see [[project-nrd-integration]]) --
				// skipped, not an error.
				continue;
			}

			fn(*this, compute, dispatch, inputs);
		}
	}
}
