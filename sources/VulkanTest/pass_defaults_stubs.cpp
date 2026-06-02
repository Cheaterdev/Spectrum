// VulkanTest — stub definitions for every PassDefault body that MainPipeline
// references.  MainPipeline::add_passes is compiled into rendersystem.lib;
// VulkanTest never calls it, but the linker still requires every symbol it
// touches to be present.
//
// Spectrum's main.cpp provides the real implementations.  Here we provide
// no-op stubs so VulkanTest can link without the full Spectrum app code.

import FrameGraph;
using namespace FrameGraph;

#include "../RenderSystem/FrameGraph/autogen/pass_defaults.h"

// ---- FSR --------------------------------------------------------------------
bool PassDefault<Passes::FSR>::setup(Passes::FSR::Context&, FrameGraph::TaskBuilder&)         { return false; }
void PassDefault<Passes::FSR>::render(Passes::FSR::Context&, FrameGraph::FrameContext&)        {}

// ---- ResultCreation ---------------------------------------------------------
bool PassDefault<Passes::ResultCreation>::setup(Passes::ResultCreation::Context&, FrameGraph::TaskBuilder&)  { return false; }
void PassDefault<Passes::ResultCreation>::render(Passes::ResultCreation::Context&, FrameGraph::FrameContext&) {}

// ---- RTXPass ----------------------------------------------------------------
bool PassDefault<Passes::RTXPass>::setup(Passes::RTXPass::Context&, FrameGraph::TaskBuilder&)  { return false; }
void PassDefault<Passes::RTXPass>::render(Passes::RTXPass::Context&, FrameGraph::FrameContext&) {}

// ---- PreScene ---------------------------------------------------------------
bool PassDefault<Passes::PreScene>::setup(Passes::PreScene::Context&, FrameGraph::TaskBuilder&)  { return false; }
void PassDefault<Passes::PreScene>::render(Passes::PreScene::Context&, FrameGraph::FrameContext&) {}

// ---- CopyPrev ---------------------------------------------------------------
bool PassDefault<Passes::CopyPrev>::setup(Passes::CopyPrev::Context&, FrameGraph::TaskBuilder&)  { return false; }
void PassDefault<Passes::CopyPrev>::render(Passes::CopyPrev::Context&, FrameGraph::FrameContext&) {}

// ---- Scene ------------------------------------------------------------------
bool PassDefault<Passes::Scene>::setup(Passes::Scene::Context&, FrameGraph::TaskBuilder&)  { return false; }
void PassDefault<Passes::Scene>::render(Passes::Scene::Context&, FrameGraph::FrameContext&) {}

// CubeMapDownsample and CubeMapEnviromentProcessor are defined in Sky.cpp
// (rendersystem.lib) — no stubs needed here.
