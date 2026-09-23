module Graphics:ShadowRTX;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// setup() is fully generated (voxel.prism's own [SetupCondition]) -- feeds
// RTXCombine (DLSS-RR) unconditionally, and additionally feeds
// NRD_SIGMA_Execute/NRD_ShadowCombine via VSM_ShadowNoise whenever
// ShadowSource::RTXReference is selected outside DLSS-RR (see
// [[project-nrd-integration]]). Gated the same as ReflectionRTX/IndirectRTX
// now -- always warm on RTX+DLSSRR-capable hardware, independent of which
// shadow source is actually selected.

void PassDefault<Passes::ShadowRTX>::render(
	Passes::ShadowRTX::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();

	GBuffer gbuffer   = GBufferViewDesc::actualize(data);
	auto noisy_output = *data.RTXShadowNoise;

	auto& sceneinfo = context.graph->get_context<SceneInfo>();

	command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

	auto& compute = command_list->get_compute();

	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	// Slots::VoxelScreen is this codebase's only GBuffer-to-raytracing
	// binding path -- see ReflectionRTX.cpp's comment on the same. Its
	// voxel-texture/cubemap fields are left unset; MyRaygenShaderShadowRTXOnly
	// never reads them.
	{
		Slots::VoxelScreen voxelScreen;
		gbuffer.SetTable(voxelScreen.GetGbuffer());
		compute.set(voxelScreen);
	}

	{
		PROFILE_GPU(L"shadow_rtx_only");
		{
			Slots::VoxelOutput output;
			output.GetNoise() = noisy_output.rwTexture2D;
			output.GetShadow_noise() = data.VSM_ShadowNoise->rwTexture2D;
			compute.set(output);
		}
		RTX::get().render<ShadowRTX>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
	}
}

