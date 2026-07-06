module Graphics;
import RenderSystem;

import :FrameGraphContext;
import :RTX;
import FrameGraph;
import HAL;

import Core;

#include "bend_sss_cpu.h"
using namespace FrameGraph;

// ── WorkGraph emulation — FlowGraph nodes ──────────────────────────────────

using TileBufView = HAL::StructuredBufferView<Table::TileRecord>;
constexpr uint64_t WG_TILE_SECTION = 8u + 256u * 256u * sizeof(Table::TileRecord); // 524296

struct WGContext : FlowGraph::GraphContext
{
	FrameContext& frame_ctx;
	ComputeContext& compute;
	const Bend::DispatchData& entry;
	int yz_base;
	int yz_count;
	Handlers::ByteAdressBuffer& wg_buffer;

	WGContext(FrameContext& fc, ComputeContext& c, const Bend::DispatchData& e,
	          int yzb, int yzc, Handlers::ByteAdressBuffer& buf)
	    : frame_ctx(fc), compute(c), entry(e), yz_base(yzb), yz_count(yzc), wg_buffer(buf)
	{}
};

struct ClassifyFlowNode : FlowGraph::GraphNode<WGContext>
{
	ClassifyFlowNode() { register_output("tiles"); }

	void operator()(WGContext* ctx) override
	{
		auto tile_buf = ctx->wg_buffer->resource->create_view<TileBufView>(
		    *ctx->frame_ctx.frame,
		    HAL::StructuredBufferViewDesc{ 0, WG_TILE_SECTION, counterType::SELF });

		ctx->compute.clear_counter(tile_buf);

		Slots::WorkGR_ClassifyPixels_NodeEmulation slot;
		slot.GetGraphInput().GetDispatch_grid() = vec3(ctx->entry.WaveCount[0], ctx->entry.WaveCount[1], ctx->entry.WaveCount[2]);
		slot.GetGraphInput().GetWaveOffset()    = int2(ctx->entry.WaveOffset_Shader[0], ctx->entry.WaveOffset_Shader[1]);
		slot.GetYZBase()                        = ctx->yz_base;
		slot.GetShadows_Node()                  = tile_buf.appendStructuredBuffer;

		ctx->compute.set_pipeline<PSOS::WorkGR_ClassifyPixels_Node>();
		ctx->compute.set(slot);
		ctx->compute.dispatch(ctx->entry.WaveCount[0], ctx->yz_count, 1);

		get_output(0)->put(tile_buf);
	}
};

struct ShadowsFlowNode : FlowGraph::GraphNode<WGContext>
{
	ShadowsFlowNode() { register_input("tiles"); }

	void operator()(WGContext* ctx) override
	{
		auto tile_buf   = get_input(0)->get<TileBufView>();
		auto& disp_args = ctx->frame_ctx.get_indirect_dispatch_args();

		ctx->frame_ctx.get_list()->get_copy().copy_buffer(
		    disp_args.resource.get(), 0,
		    tile_buf.get_counter_buffer().get(), tile_buf.get_counter_offset(), 4);

		Slots::WorkGR_Shadows_NodeEmulation slot;
		slot.GetTiles() = tile_buf.consumeStructuredBuffer;

		ctx->compute.set_pipeline<PSOS::WorkGR_Shadows_Node>();
		ctx->compute.set(slot);
		ctx->compute.exec_indirect(disp_args, 1);

		ctx->frame_ctx.get_list()->get_copy().read_counter(tile_buf, [](uint remaining) {
			ASSERT(remaining == 0);
		});
	}
};

// ── ResultCreation -----------------------------------------------------------

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


// ---- RTXShadow --------------------------------------------------------------

bool PassDefault<Passes::RTXShadow>::setup(
    Passes::RTXShadow::Context& data, FrameGraph::TaskBuilder& builder)
{
	auto& frame = builder.graph->get_context<ViewportInfo>();
	auto  size  = frame.frame_size;

	builder.need(data.gbuffer.GBuffer_Albedo,    ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_Normals,   ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_Depth,     ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_Specular,  ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_Speed,     ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_DepthPrev, ResourceFlags::PixelRead | ResourceFlags::ComputeRead);
	builder.need(data.gbuffer.GBuffer_DepthMips, ResourceFlags::None);

	builder.create(data.ShadowMask,
	    { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1 },
	    ResourceFlags::UnorderedAccess);

	if (RenderSystem::get().device().get_properties().work_graph)
	{
		auto work_pso = RenderSystem::get().device().get_engine_pso_holder().GetPSO<PSOS::WorkGR>();
		builder.create(data.WorkGraphBuffer, { work_pso->buffer_size }, ResourceFlags::UnorderedAccess);
	}
	else
	{
		// counter_pad(8) + tile_data(256*256 * sizeof(TileRecord))
		constexpr uint64_t TILE_SECTION = 8u + 256u * 256u * sizeof(Table::TileRecord); // 524296
		builder.create(data.WorkGraphBuffer, { TILE_SECTION }, ResourceFlags::UnorderedAccess);
	}

	return RenderSystem::get().device().get_properties().rtx;
}

void PassDefault<Passes::RTXShadow>::render(
    Passes::RTXShadow::Context& data, FrameGraph::FrameContext& context)
{
	auto& scene_ctx  = context.graph->get_context<SceneInfo>();
	auto& camera_ctx = context.graph->get_context<CameraInfo>();
	auto& sky_ctx    = context.graph->get_context<SkyInfo>();

	auto& compute = context.get_list()->get_compute();

	if (data.ShadowMask.is_new())
		context.get_list()->clear_uav(data.ShadowMask->rwTexture2D, vec4(0, 0, 0, 0));

	//context.graph->set_slot(SlotID::VoxelInfo, compute);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

	{
		Slots::Raytracing rtx;
		rtx.GetScene() = scene_ctx.scene->raytrace_scene->get_handle();
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
	    { data.ShadowMask->get_size().x, data.ShadowMask->get_size().y },
	    { 0, 0 },
	    { data.ShadowMask->get_size().x, data.ShadowMask->get_size().y },
	    false, 64);

	Slots::DispatchParameters dispatchParameters;
	dispatchParameters.GetDepthTexture()    = gbuffer.depth.texture2D;
	dispatchParameters.GetOutputTexture()   = data.ShadowMask->rwTexture2D;
	dispatchParameters.GetLightCoordinate() = float4(
	    res.LightCoordinate_Shader[0], res.LightCoordinate_Shader[1],
	    res.LightCoordinate_Shader[2], res.LightCoordinate_Shader[3]);
	dispatchParameters.FarDepthValue       = 0;
	dispatchParameters.NearDepthValue      = 1;
	dispatchParameters.InvDepthTextureSize = float2(
	    1.0f / data.ShadowMask->get_size().x,
	    1.0f / data.ShadowMask->get_size().y);
	compute.set(dispatchParameters);

	if (RenderSystem::get().device().get_properties().work_graph)
	{
		auto& backingBuffer = data.WorkGraphBuffer->resource;
	
		compute.set_program<PSOS::WorkGR>(backingBuffer->get_resource_address(), data.WorkGraphBuffer.is_new());

		auto ep = create_entry(compute);
		for (auto i = 0; i < res.DispatchCount; i++)
		{
			auto& e = res.Dispatch[i];
			Table::GraphInput input;
			input.GetDispatch_grid() = vec3(e.WaveCount[0], e.WaveCount[1], e.WaveCount[2]);
			input.GetWaveOffset()    = int2(e.WaveOffset_Shader[0], e.WaveOffset_Shader[1]);
			ep.add(0, input);
		}
		if (res.DispatchCount)
			compute.dispatch_graph(ep.compile());
	}
	else
	{
		// Build the emulation FlowGraph once per render call.
		// graph::start() resets all parameter values on each call, so the same
		// graph instance is safe to start() once per YZ chunk.
		FlowGraph::graph wg_graph;

		auto classify = std::make_shared<ClassifyFlowNode>();
		auto shadows  = std::make_shared<ShadowsFlowNode>();

		wg_graph.register_node(classify);
		wg_graph.register_node(shadows);
		classify->get_output(0)->link(shadows->get_input(0));

		// WaveCount[0] is always 64 (fixed wave size). Split YZ into chunks of 16.
		constexpr int MAX_YZ_CHUNK = 16;

		for (int i = 0; i < res.DispatchCount; i++)
		{
			auto& e        = res.Dispatch[i];
			int   total_yz = e.WaveCount[1] * e.WaveCount[2];

			for (int yz_base = 0; yz_base < total_yz; yz_base += MAX_YZ_CHUNK)
			{
				int yz_count = std::min(MAX_YZ_CHUNK, total_yz - yz_base);
				WGContext wg_ctx(context, compute, e, yz_base, yz_count, data.WorkGraphBuffer);
				wg_graph.start(&wg_ctx);
			}
		}
	}
}

// ── RTXColorPass — full-scene ray-traced color view ────────────────────────
// Casts primary camera rays via the ColorRTX raygen (ColorPass hit/miss) and
// writes the traced color to ColorOutput. On-demand: only enabled when a
// consumer (the debug view) needs ColorOutput.

bool PassDefault<Passes::RTXColorPass>::setup(
    Passes::RTXColorPass::Context& data, FrameGraph::TaskBuilder& builder)
{
	if (!RenderSystem::get().device().get_properties().rtx)
		return false;

	auto& frame = builder.graph->get_context<ViewportInfo>();
	auto  size  = frame.frame_size;

	// Depend on PreScene (writes `scene`) so the RTX BVH is ready before tracing.
	builder.need(data.scene, ResourceFlags::ComputeRead);

	builder.create(data.ColorOutput,
	    { ivec3(size, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
	    ResourceFlags::UnorderedAccess);
	return true;
}

void PassDefault<Passes::RTXColorPass>::render(
    Passes::RTXColorPass::Context& data, FrameGraph::FrameContext& context)
{
	auto& sceneinfo = context.graph->get_context<SceneInfo>();
	auto& compute   = context.get_list()->get_compute();

	compute.set_signature(RTX::get().rtx.m_root_sig);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	context.graph->set_slot(SlotID::SceneData, compute);

	{
		Slots::ColorRTXOutput output;
		output.GetOutput() = data.ColorOutput->rwTexture2D;
		compute.set(output);
	}

	RTX::get().render<ColorRTX>(compute, sceneinfo.scene->raytrace_scene, data.ColorOutput->get_size());
}
