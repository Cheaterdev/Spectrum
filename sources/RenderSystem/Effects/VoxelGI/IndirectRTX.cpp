module Graphics:IndirectRTX;

import RenderSystem;
import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

#ifdef HAL_BACKEND_D3D12

bool PassDefault<Passes::IndirectRTX>::setup(
	Passes::IndirectRTX::Context& data, TaskBuilder& builder)
{
	// Feeds RTXCombine, active only when the user has picked DLSS-RR via
	// g_upscaler_type. Gated the same as ReflectionRTX/ShadowRTX/RTXCombine
	// so all four stay in lockstep.
	if (g_upscaler_type != UpscalerType::DLSSRR ||
	    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
		return false;

	auto& frame = builder.graph->get_context<ViewportInfo>();
	auto  sz    = frame.frame_size;

	builder.create(data.RTXIndirectNoise,
		{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
	builder.need(data.BlueNoise, ResourceFlags::ComputeRead);
	GBufferViewDesc::need(builder, data.gbuffer);
	return true;
}

void PassDefault<Passes::IndirectRTX>::render(
	Passes::IndirectRTX::Context& data, FrameContext& context)
{
	auto& command_list = context.get_list();

	GBuffer gbuffer   = GBufferViewDesc::actualize(data.gbuffer);
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

bool PassDefault<Passes::IndirectRTX>::setup(Passes::IndirectRTX::Context&, TaskBuilder&) { return false; }
void PassDefault<Passes::IndirectRTX>::render(Passes::IndirectRTX::Context&, FrameContext&) {}

#endif
