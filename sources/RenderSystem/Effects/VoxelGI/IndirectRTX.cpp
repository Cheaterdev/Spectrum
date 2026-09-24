module Graphics:IndirectRTX;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// setup() is fully generated (voxel.prism's own [SetupCondition]).

void PassDefault<Passes::GI::IndirectRTXHalf>::render(
	Passes::GI::IndirectRTXHalf::Context& data, FrameContext& context)
{
	auto noisy_output = *data.RTXIndirectNoiseHalf;
	auto& sceneinfo   = context.graph->get_context<SceneInfo>();
	auto& compute     = context.get_list()->get_compute();

	compute.set_signature(RTX::get().rtx.m_root_sig);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	{
		Slots::GI::IndirectRTXHalfGBuffer half_gbuffer;
		half_gbuffer.GetDepth()   = data.GBuffer_HalfDepth->texture2D;
		half_gbuffer.GetNormals() = data.GBuffer_HalfNormals->texture2D;
		compute.set(half_gbuffer);
	}

	{
		PROFILE_GPU(L"indirect_rtx_half");
		{
			Slots::GI::Voxel::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			// DDGI probe-volume feedback term, sampled by TraceIndirectDiffuse
			// (raytracing.hlsl) -- see [[project-ddgi]] planning notes. One
			// DDGIInfo per cascade; TraceIndirectDiffuse picks the finest one
			// that actually contains each hit point.
			{
				float3 cam_pos = context.graph->get_context<CameraInfo>().cam->position;
				output.GetDdgi_cascade0() = ddgi_make_info(cam_pos, 0);
				output.GetDdgi_cascade1() = ddgi_make_info(cam_pos, 1);
				output.GetDdgi_cascade2() = ddgi_make_info(cam_pos, 2);
				output.GetDdgi_cascade3() = ddgi_make_info(cam_pos, 3);
				output.GetDdgi_cascade4() = ddgi_make_info(cam_pos, 4);
			}
			output.GetDdgi_irradiance()  = data.DDGI_ProbeIrradiance->texture2DArray;
			output.GetDdgi_visibility()  = data.DDGI_ProbeVisibility->texture2DArray;
			output.GetDdgi_residency()   = data.DDGI_ProbeResidency->structuredBuffer;
			output.GetDdgi_residency_pending() = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
			compute.set(output);
		}
		RTX::get().render<GI::IndirectRTXHalf>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

// setup() is fully generated (voxel.prism's own [SetupCondition]) -- feeds
// NRD_REBLUR_Execute (REBLUR_DIFFUSE, see [[project-nrd-integration]]) and,
// under DLSS-RR, RTXCombine -- gated purely on RTX/hardware support now,
// independent of upscaler (NRD is the only indirect-GI denoiser).

void PassDefault<Passes::GI::IndirectRTX>::render(
	Passes::GI::IndirectRTX::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();

	GBuffer gbuffer   = GBufferViewDesc::actualize(data);
	auto noisy_output = *data.RTXIndirectNoise;

	auto& sceneinfo = context.graph->get_context<SceneInfo>();

	command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

	auto& compute = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	// Slots::GI::Voxel::VoxelScreen is this codebase's only GBuffer-to-raytracing
	// binding path -- see ReflectionRTX.cpp's comment on the same. Its
	// voxel-texture/cubemap fields are left unset; MyRaygenShaderIndirectRTXOnly
	// never reads them.
	{
		Slots::GI::Voxel::VoxelScreen voxelScreen;
		gbuffer.SetTable(voxelScreen.GetGbuffer());
		compute.set(voxelScreen);
	}

	{
		Slots::GI::IndirectRTXUpscale upscale;
		upscale.GetNoiseHalf()  = data.RTXIndirectNoiseHalf->texture2D;
		upscale.GetTileFlags()  = data.TileClassifyTiles->texture2D;
		compute.set(upscale);
	}

	{
		PROFILE_GPU(L"indirect_rtx_only");
		{
			Slots::GI::Voxel::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			// DDGI probe-volume feedback term, sampled by TraceIndirectDiffuse
			// (raytracing.hlsl) -- see [[project-ddgi]] planning notes. One
			// DDGIInfo per cascade; TraceIndirectDiffuse picks the finest one
			// that actually contains each hit point.
			{
				float3 cam_pos = context.graph->get_context<CameraInfo>().cam->position;
				output.GetDdgi_cascade0() = ddgi_make_info(cam_pos, 0);
				output.GetDdgi_cascade1() = ddgi_make_info(cam_pos, 1);
				output.GetDdgi_cascade2() = ddgi_make_info(cam_pos, 2);
				output.GetDdgi_cascade3() = ddgi_make_info(cam_pos, 3);
				output.GetDdgi_cascade4() = ddgi_make_info(cam_pos, 4);
			}
			output.GetDdgi_irradiance()  = data.DDGI_ProbeIrradiance->texture2DArray;
			output.GetDdgi_visibility()  = data.DDGI_ProbeVisibility->texture2DArray;
			output.GetDdgi_residency()   = data.DDGI_ProbeResidency->structuredBuffer;
			output.GetDdgi_residency_pending() = data.DDGI_ProbeResidencyPending->rwStructuredBuffer;
			compute.set(output);
		}
		RTX::get().render<GI::IndirectRTX>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

