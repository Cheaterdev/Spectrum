#pragma once

// Base template — no default implementation.
// Specialize this for any PassNode whose setup/render logic is fully
// self-contained (i.e. needs nothing from outside the pass context).
// For passes that need RenderSystem types (PSOS/Slots), declare the
// specialization here and define setup/render out-of-line in the
// corresponding Graphics module .cpp file.
template<typename TPass>
struct PassDefault
{
	static constexpr bool enabled = false;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};


// ---- ResultCreation ----------------------------------------------------
// Allocates the HDR result texture sized to the current viewport.

template<>
struct PassDefault<Passes::ResultCreation>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::ResultCreation::Context& data, FrameGraph::TaskBuilder& builder)
	{
		auto& frame = builder.graph->get_context<ViewportInfo>();
		builder.create(data.ResultTexture,
			{ uint3(frame.frame_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
			FrameGraph::ResourceFlags::RenderTarget);
		return false; // resource-only pass, no render work
	}

	static void render(Passes::ResultCreation::Context&, FrameGraph::FrameContext&) {}
};


// ---- CopyPrev ----------------------------------------------------------
// Copies GBuffer normals, specular and depth into their "prev" slots at
// the end of each frame.

template<>
struct PassDefault<Passes::CopyPrev>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::CopyPrev::Context& data, FrameGraph::TaskBuilder& builder)
	{
		builder.need(data.gbuffer.GBuffer_NormalsPrev,  FrameGraph::ResourceFlags::CopyDest);
		builder.need(data.gbuffer.GBuffer_SpecularPrev, FrameGraph::ResourceFlags::CopyDest);
		builder.need(data.gbuffer.GBuffer_Normals,      FrameGraph::ResourceFlags::CopySource);
		builder.need(data.gbuffer.GBuffer_Specular,     FrameGraph::ResourceFlags::CopySource);
		builder.need(data.gbuffer.GBuffer_DepthPrev,    FrameGraph::ResourceFlags::CopyDest);
		builder.need(data.gbuffer.GBuffer_DepthMips,    FrameGraph::ResourceFlags::CopySource);
		return true;
	}

	static void render(Passes::CopyPrev::Context& data, FrameGraph::FrameContext& context)
	{
		auto& copy = context.get_list()->get_copy();

		copy.copy_resource(data.gbuffer.GBuffer_NormalsPrev->resource,
		                   data.gbuffer.GBuffer_Normals->resource);
		copy.copy_resource(data.gbuffer.GBuffer_SpecularPrev->resource,
		                   data.gbuffer.GBuffer_Specular->resource);
		copy.copy_texture(data.gbuffer.GBuffer_DepthPrev->resource, 0,
		                  data.gbuffer.GBuffer_DepthMips->resource, 0);
	}
};


// ---- CubeMapDownsample -------------------------------------------------
// Generates mipmaps for the sky cubemap.
// setup/render are defined out-of-line in Graphics:Sky (Sky.cpp).

template<>
struct PassDefault<Passes::CubeMapDownsample>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::CubeMapDownsample::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::CubeMapDownsample::Context& data, FrameGraph::FrameContext& context);
};


// ---- CubeMapEnviromentProcessor ----------------------------------------
// Filters the sky cubemap into specular and diffuse IBL targets.
// setup/render are defined out-of-line in Graphics:Sky (Sky.cpp).

template<>
struct PassDefault<Passes::CubeMapEnviromentProcessor>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::CubeMapEnviromentProcessor::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::CubeMapEnviromentProcessor::Context& data, FrameGraph::FrameContext& context);
};


// ---- Profiler ----------------------------------------------------------
// Ensures the swapchain texture is retained as a required render target.
// Render is intentionally empty — the pass exists only for the resource
// dependency it declares.

template<>
struct PassDefault<Passes::Profiler>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Required;

	static bool setup(Passes::Profiler::Context& data, FrameGraph::TaskBuilder& builder)
	{
		builder.need(data.swapchain,
		             FrameGraph::ResourceFlags::Required | FrameGraph::ResourceFlags::RenderTarget);
		return false;
	}

	static void render(Passes::Profiler::Context&, FrameGraph::FrameContext&) {}
};


// ---- FSR ---------------------------------------------------------------
// AMD FidelityFX Super Resolution upscaling pass.
// setup/render are defined out-of-line in Graphics:FSR (FSR.cpp) because
// they require PSOS/Slots and the FSR math headers (ffx_a.h, ffx_fsr1.h)
// which are not available inside FrameGraph:Passes.

template<>
struct PassDefault<Passes::FSR>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::FSR::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::FSR::Context& data, FrameGraph::FrameContext& context);
};
