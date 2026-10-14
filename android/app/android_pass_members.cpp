module FrameGraph;
import HAL;

using namespace FrameGraph;

// Dev::Profiler's render() sits in PassDefaults.cpp, which the Android build does not compile; the
// passes of UIPipeline need it to link. It draws nothing there either.
void PassDefault<Passes::Dev::Profiler>::render(Passes::Dev::Profiler::Context&, FrameGraph::FrameContext&) {}

// ResultCreation only exists to create ResultTexture (its setup is generated); it never renders.
void PassDefault<Passes::Frame::ResultCreation>::render(Passes::Frame::ResultCreation::Context&, FrameGraph::FrameContext&) {}
