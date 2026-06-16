module Graphics;

import :FrameGraphContext;
import FrameGraph;
import HAL;

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
// Not yet implemented; always disabled so the pass is skipped at runtime.

bool PassDefault<Passes::RTXPass>::setup(
	Passes::RTXPass::Context&, FrameGraph::TaskBuilder&) { return false; }

void PassDefault<Passes::RTXPass>::render(
	Passes::RTXPass::Context&, FrameGraph::FrameContext&) {}
