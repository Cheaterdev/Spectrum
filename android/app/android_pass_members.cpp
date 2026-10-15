module FrameGraph;
import HAL;

using namespace FrameGraph;

// Dev::Profiler's render() sits in PassDefaults.cpp, which the Android build does not compile; the
// passes of UIPipeline need it to link. It draws nothing there either.
void PassDefault<Passes::Dev::Profiler>::render(Passes::Dev::Profiler::Context&, FrameGraph::FrameContext&) {}

// ResultCreation creates ResultTexture (its setup is generated) and clears it, as in PassDefaults.cpp.
void PassDefault<Passes::Frame::ResultCreation>::render(Passes::Frame::ResultCreation::Context& data, FrameGraph::FrameContext& context)
{
	RT::Frame::SingleColor rt;
	rt.GetColor() = data.ResultTexture->renderTarget;
	context.get_list()->get_graphics().set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor);
}
