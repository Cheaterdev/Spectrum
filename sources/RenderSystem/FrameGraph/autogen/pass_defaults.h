#pragma once
#define PASS_EXPORT extern "C++"
// Base template — no default implementation.
// Specialize this for any PassNode whose setup/render logic is fully
// self-contained (i.e. needs nothing from outside the pass context).
// All specialization bodies are defined out-of-line in main.cpp.

extern "C++"
{

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

		static bool setup(Passes::ResultCreation::Context& data, FrameGraph::TaskBuilder& builder);
		static void render(Passes::ResultCreation::Context& data, FrameGraph::FrameContext& context);
	};


	// ---- CopyPrev ----------------------------------------------------------
	// Copies GBuffer normals, specular and depth into their "prev" slots at
	// the end of each frame.

	template<>
	struct PassDefault<Passes::CopyPrev>
	{
		static constexpr bool enabled = true;
		static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

		static bool setup(Passes::CopyPrev::Context& data, FrameGraph::TaskBuilder& builder);
		static void render(Passes::CopyPrev::Context& data, FrameGraph::FrameContext& context);
	};


	// ---- CubeMapDownsample -------------------------------------------------
	// Generates mipmaps for the sky cubemap.

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

	template<>
	struct PassDefault<Passes::Profiler>
	{
		static constexpr bool enabled = true;
		static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Required;

		static bool setup(Passes::Profiler::Context& data, FrameGraph::TaskBuilder& builder);
		static void render(Passes::Profiler::Context& data, FrameGraph::FrameContext& context);
	};


	// ---- FSR ---------------------------------------------------------------
	// AMD FidelityFX Super Resolution upscaling pass.

	template<>
	struct PassDefault<Passes::FSR>
	{
		static constexpr bool enabled = true;
		static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

		static bool setup(Passes::FSR::Context& data, FrameGraph::TaskBuilder& builder);
		static void render(Passes::FSR::Context& data, FrameGraph::FrameContext& context);
	};
}