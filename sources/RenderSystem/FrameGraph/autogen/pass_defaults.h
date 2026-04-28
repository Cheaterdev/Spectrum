#pragma once

// AUTO-GENERATED — do not edit by hand.
// Regenerate by running SIGParser.
// Source: passes marked [Static] in *.sig files.
//
// PassDefault<T> provides the setup/render implementations for passes whose
// logic is fully self-contained (no external wiring needed).  Bodies are
// defined out-of-line in main.cpp.

extern "C++"
{

template<typename TPass>
struct PassDefault
{
	static constexpr bool enabled = false;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};


template<>
struct PassDefault<Passes::FSR>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::FSR::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::FSR::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::ResultCreation>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::ResultCreation::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::ResultCreation::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::Profiler>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Required;

	static bool setup(Passes::Profiler::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::Profiler::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::CopyPrev>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::CopyPrev::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::CopyPrev::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::CubeMapDownsample>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;

	static bool setup(Passes::CubeMapDownsample::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::CubeMapDownsample::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::CubeMapEnviromentProcessor>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::CubeMapEnviromentProcessor::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::CubeMapEnviromentProcessor::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::PreScene>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::PreScene::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::PreScene::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::Scene>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::Scene::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::Scene::Context& data, FrameGraph::FrameContext& context);
};


template<>
struct PassDefault<Passes::UI_Render>
{
	static constexpr bool enabled = true;
	static constexpr FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;

	static bool setup(Passes::UI_Render::Context& data, FrameGraph::TaskBuilder& builder);
	static void render(Passes::UI_Render::Context& data, FrameGraph::FrameContext& context);
};


}
