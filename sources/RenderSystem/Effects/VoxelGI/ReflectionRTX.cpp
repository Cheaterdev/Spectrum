module Graphics:ReflectionRTX;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

#ifdef HAL_BACKEND_D3D12

FrameGraph::SetupResult PassDefault<Passes::ReflectionRTXHalf>::setup(
	Passes::ReflectionRTXHalf::Context& data, TaskBuilder& builder)
{
	if (!RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	return true;
}

void PassDefault<Passes::ReflectionRTXHalf>::render(
	Passes::ReflectionRTXHalf::Context& data, FrameContext& context)
{
	auto noisy_output = *data.RTXReflectionNoiseHalf;
	auto dir_and_pdf  = *data.RTXReflectionDirPdfHalf;
	auto& sceneinfo   = context.graph->get_context<SceneInfo>();
	auto& compute     = context.get_list()->get_compute();

	compute.set_signature(RTX::get().rtx.m_root_sig);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	{
		Slots::IndirectRTXHalfGBuffer half_gbuffer;
		half_gbuffer.GetDepth()   = data.GBuffer_HalfDepth->texture2D;
		half_gbuffer.GetNormals() = data.GBuffer_HalfNormals->texture2D;
		compute.set(half_gbuffer);
	}

	{
		PROFILE_GPU(L"reflection_rtx_half");
		{
			Slots::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetDirAndPdf() = dir_and_pdf.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			compute.set(output);
		}
		RTX::get().render<ReflectionRTXHalf>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

FrameGraph::SetupResult PassDefault<Passes::ReflectionRTX>::setup(
	Passes::ReflectionRTX::Context& data, TaskBuilder& builder)
{
	// Feeds NRD_REBLUR_Execute (REBLUR_SPECULAR, see
	// [[project-nrd-integration]]) and, under DLSS-RR, RTXCombine -- gated
	// purely on RTX/hardware support now, independent of upscaler (NRD is
	// the only reflection denoiser).
	if (!RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	return true;
}

void PassDefault<Passes::ReflectionRTX>::render(
	Passes::ReflectionRTX::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();

	GBuffer gbuffer   = GBufferViewDesc::actualize(data);
	auto noisy_output = *data.RTXReflectionNoise;
	auto dir_and_pdf  = *data.RTXReflectionDirPdf;

	auto& sceneinfo = context.graph->get_context<SceneInfo>();

	command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

	auto& compute = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	// Slots::VoxelScreen is this codebase's only GBuffer-to-raytracing
	// binding path (RTXShadow reuses it the same way, GBuffer only) -- its
	// voxel-texture/cubemap fields are left unset (reads descriptor 0, an
	// established pattern for fields no bound shader ever samples, see the
	// struct's own comment in voxel.sig). MyRaygenShaderReflectionRTXOnly
	// never calls GetVoxels()/GetTex_cube(), and SlotID::VoxelInfo is never
	// bound at all -- this pass touches no voxel data whatsoever.
	{
		Slots::VoxelScreen voxelScreen;
		gbuffer.SetTable(voxelScreen.GetGbuffer());
		compute.set(voxelScreen);
	}

	{
		Slots::ReflectionRTXUpscale upscale;
		upscale.GetNoiseHalf()          = data.RTXReflectionNoiseHalf->texture2D;
		upscale.GetDirPdfHalf()         = data.RTXReflectionDirPdfHalf->texture2D;
		upscale.GetTileFlags()          = data.TileClassifyTiles->texture2D;
		upscale.GetRoughnessTileFlags() = data.TileRoughnessTiles->texture2D;
		compute.set(upscale);
	}

	{
		PROFILE_GPU(L"reflection_rtx_only");
		{
			Slots::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetDirAndPdf() = dir_and_pdf.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			compute.set(output);
		}
		RTX::get().render<ReflectionRTX>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

#else

FrameGraph::SetupResult PassDefault<Passes::ReflectionRTXHalf>::setup(Passes::ReflectionRTXHalf::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::ReflectionRTXHalf>::render(Passes::ReflectionRTXHalf::Context&, FrameContext&) {}

FrameGraph::SetupResult PassDefault<Passes::ReflectionRTX>::setup(Passes::ReflectionRTX::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::ReflectionRTX>::render(Passes::ReflectionRTX::Context&, FrameContext&) {}

#endif
