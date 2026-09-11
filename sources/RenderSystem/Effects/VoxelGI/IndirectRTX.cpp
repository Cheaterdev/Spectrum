module Graphics:IndirectRTX;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

#ifdef HAL_BACKEND_D3D12

FrameGraph::SetupResult PassDefault<Passes::IndirectRTXHalf>::setup(
	Passes::IndirectRTXHalf::Context& data, TaskBuilder& builder)
{
	if (!RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	return true;
}

void PassDefault<Passes::IndirectRTXHalf>::render(
	Passes::IndirectRTXHalf::Context& data, FrameContext& context)
{
	auto noisy_output = *data.RTXIndirectNoiseHalf;
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
		PROFILE_GPU(L"indirect_rtx_half");
		{
			Slots::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			compute.set(output);
		}
		RTX::get().render<IndirectRTXHalf>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

FrameGraph::SetupResult PassDefault<Passes::IndirectRTX>::setup(
	Passes::IndirectRTX::Context& data, TaskBuilder& builder)
{
	// Feeds NRD_REBLUR_Execute (REBLUR_DIFFUSE, see
	// [[project-nrd-integration]]) and, under DLSS-RR, RTXCombine -- gated
	// purely on RTX/hardware support now, independent of upscaler (NRD is
	// the only indirect-GI denoiser).
	if (!RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	return true;
}

void PassDefault<Passes::IndirectRTX>::render(
	Passes::IndirectRTX::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();

	GBuffer gbuffer   = GBufferViewDesc::actualize(data);
	auto noisy_output = *data.RTXIndirectNoise;

	auto& sceneinfo = context.graph->get_context<SceneInfo>();

	command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

	auto& compute = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	// Slots::VoxelScreen is this codebase's only GBuffer-to-raytracing
	// binding path -- see ReflectionRTX.cpp's comment on the same. Its
	// voxel-texture/cubemap fields are left unset; MyRaygenShaderIndirectRTXOnly
	// never reads them.
	{
		Slots::VoxelScreen voxelScreen;
		gbuffer.SetTable(voxelScreen.GetGbuffer());
		compute.set(voxelScreen);
	}

	{
		Slots::IndirectRTXUpscale upscale;
		upscale.GetNoiseHalf()  = data.RTXIndirectNoiseHalf->texture2D;
		upscale.GetTileFlags()  = data.TileClassifyTiles->texture2D;
		compute.set(upscale);
	}

	{
		PROFILE_GPU(L"indirect_rtx_only");
		{
			Slots::VoxelOutput output;
			output.GetNoise()     = noisy_output.rwTexture2D;
			output.GetBlueNoise() = data.BlueNoise->texture2D;
			compute.set(output);
		}
		RTX::get().render<IndirectRTX>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

#else

FrameGraph::SetupResult PassDefault<Passes::IndirectRTXHalf>::setup(Passes::IndirectRTXHalf::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::IndirectRTXHalf>::render(Passes::IndirectRTXHalf::Context&, FrameContext&) {}

FrameGraph::SetupResult PassDefault<Passes::IndirectRTX>::setup(Passes::IndirectRTX::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::IndirectRTX>::render(Passes::IndirectRTX::Context&, FrameContext&) {}

#endif
