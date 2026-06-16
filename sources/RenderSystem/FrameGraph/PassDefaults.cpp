module Graphics;
import RenderSystem;

import :FrameGraphContext;
import FrameGraph;
import HAL;


#include "bend_sss_cpu.h"
using namespace FrameGraph;

// ---- ResultCreation ---------------------------------------------------------

bool PassDefault<Passes::ResultCreation>::setup(
	Passes::ResultCreation::Context& data, FrameGraph::TaskBuilder& builder)
{
	auto& frame = builder.graph->get_context<ViewportInfo>();
	builder.create(data.ResultTexture,
		{ uint3(frame.frame_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
		FrameGraph::ResourceFlags::RenderTarget);
	return false;
}

void PassDefault<Passes::ResultCreation>::render(
	Passes::ResultCreation::Context&, FrameGraph::FrameContext&) {}


// ---- CopyPrev ---------------------------------------------------------------

bool PassDefault<Passes::CopyPrev>::setup(
	Passes::CopyPrev::Context& data, FrameGraph::TaskBuilder& builder)
{
	builder.need(data.gbuffer.GBuffer_NormalsPrev,  FrameGraph::ResourceFlags::CopyDest);
	builder.need(data.gbuffer.GBuffer_SpecularPrev, FrameGraph::ResourceFlags::CopyDest);
	builder.need(data.gbuffer.GBuffer_Normals,      FrameGraph::ResourceFlags::CopySource);
	builder.need(data.gbuffer.GBuffer_Specular,     FrameGraph::ResourceFlags::CopySource);
	builder.need(data.gbuffer.GBuffer_DepthPrev,    FrameGraph::ResourceFlags::CopyDest);
	builder.need(data.gbuffer.GBuffer_DepthMips,    FrameGraph::ResourceFlags::CopySource);
	return true;
}

void PassDefault<Passes::CopyPrev>::render(
	Passes::CopyPrev::Context& data, FrameGraph::FrameContext& context)
{
	auto& copy = context.get_list()->get_copy();

	copy.copy_resource(data.gbuffer.GBuffer_NormalsPrev->resource,
	                   data.gbuffer.GBuffer_Normals->resource);
	copy.copy_resource(data.gbuffer.GBuffer_SpecularPrev->resource,
	                   data.gbuffer.GBuffer_Specular->resource);
	copy.copy_texture(data.gbuffer.GBuffer_DepthPrev->resource, 0,
	                  data.gbuffer.GBuffer_DepthMips->resource, 0);
}


// ---- Profiler ---------------------------------------------------------------

bool PassDefault<Passes::Profiler>::setup(
	Passes::Profiler::Context& data, FrameGraph::TaskBuilder& builder)
{
	builder.need(data.swapchain,
	             FrameGraph::ResourceFlags::Required | FrameGraph::ResourceFlags::RenderTarget);
	return false;
}

void PassDefault<Passes::Profiler>::render(
	Passes::Profiler::Context&, FrameGraph::FrameContext&) {}


// ---- RTXPass ----------------------------------------------------------------
bool PassDefault<Passes::RTXPass>::setup(
    Passes::RTXPass::Context& data, FrameGraph::TaskBuilder& builder)
{
    auto& frame    = builder.graph->get_context<ViewportInfo>();
    auto  work_pso = RenderSystem::get().device().get_engine_pso_holder().GetPSO<PSOS::WorkGR>();
    auto  size     = frame.frame_size;

    builder.need(data.gbuffer.GBuffer_Albedo,   ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_Normals,  ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_Depth,    ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_Specular, ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_Speed,    ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_DepthPrev, ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
    builder.need(data.gbuffer.GBuffer_DepthMips, ResourceFlags::None);

    builder.create(data.RTXDebug,
        { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 },
        ResourceFlags::UnorderedAccess);
    builder.create(data.WorkGraphBuffer, { work_pso->buffer_size },
        ResourceFlags::UnorderedAccess);

    return true;
}

void PassDefault<Passes::RTXPass>::render(
    Passes::RTXPass::Context& data, FrameGraph::FrameContext& context)
{
    auto& scene_ctx  = context.graph->get_context<SceneInfo>();
    auto& camera_ctx = context.graph->get_context<CameraInfo>();
    auto& sky_ctx    = context.graph->get_context<SkyInfo>();

    auto& compute = context.get_list()->get_compute();

    if (data.RTXDebug.is_new())
        context.get_list()->clear_uav(data.RTXDebug->rwTexture2D, vec4(0, 0, 0, 0));

    auto& backingBuffer = data.WorkGraphBuffer->resource;
    auto  work_pso = RenderSystem::get().device().get_engine_pso_holder().GetPSO<PSOS::WorkGR>();

    compute.set_program(work_pso.get(),
        backingBuffer->get_resource_address(),
        uint(work_pso->buffer_size),
        data.WorkGraphBuffer.is_new());

    context.graph->set_slot(SlotID::VoxelInfo,  compute);
    context.graph->set_slot(SlotID::FrameInfo,  compute);
    context.graph->set_slot(SlotID::SceneData,  compute);

    GBuffer gbuffer=GBufferViewDesc::actualize(data.gbuffer);


    {
        Slots::Raytracing rtx;
        rtx.GetScene() = scene_ctx.scene->raytrace_scene->raytracing_handle;
        compute.set(rtx);
    }
    {
        Slots::VoxelScreen voxelScreen;
        gbuffer.SetTable(voxelScreen.GetGbuffer());
        voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips;
        compute.set(voxelScreen);
    }

    auto light = float4(sky_ctx.sunDir, 0) * camera_ctx.cam->get_view_proj();

    Bend::DispatchList res = Bend::BuildDispatchList(
        { light.x, light.y, light.z, light.w },
        { data.RTXDebug->get_size().x, data.RTXDebug->get_size().y },
        { 0, 0 },
        { data.RTXDebug->get_size().x, data.RTXDebug->get_size().y },
        false, 64);

    Slots::DispatchParameters dispatchParameters;
    dispatchParameters.GetDepthTexture()    = gbuffer.depth.texture2D;
    dispatchParameters.GetOutputTexture()   = data.RTXDebug->rwTexture2D;
    dispatchParameters.GetLightCoordinate() = float4(
        res.LightCoordinate_Shader[0], res.LightCoordinate_Shader[1],
        res.LightCoordinate_Shader[2], res.LightCoordinate_Shader[3]);
    dispatchParameters.FarDepthValue       = 0;
    dispatchParameters.NearDepthValue      = 1;
    dispatchParameters.InvDepthTextureSize = float2(
        1.0f / data.RTXDebug->get_size().x,
        1.0f / data.RTXDebug->get_size().y);
    compute.set(dispatchParameters);

    auto ep = create_entry(compute);
    for (auto i = 0; i < res.DispatchCount; i++)
    {
        auto& e = res.Dispatch[i];
        {
            Slots::GraphInput input;
            input.GetDispatch_grid() = vec3(e.WaveCount[0], e.WaveCount[1], e.WaveCount[2]);
            input.GetWaveOffset()    = int2(e.WaveOffset_Shader[0], e.WaveOffset_Shader[1]);
            ep.add(0, input);
        }
    }
    if (res.DispatchCount)
        compute.dispatch_graph(ep.compile());
}