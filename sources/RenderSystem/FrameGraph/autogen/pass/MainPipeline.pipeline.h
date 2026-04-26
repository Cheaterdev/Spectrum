
#include "Gbuffer.h"
#include "BRDF.h"

using namespace FrameGraph;

namespace Pipelines
{

class MainPipeline
{
public:

	Passes::Gbuffer gbuffer;
	Passes::BRDF bRDF;

	void add_passes(FrameGraph::Graph& graph)
	{

		graph.add_library_pass<Passes::Gbuffer>(gbuffer.setup_func, gbuffer.render_func);
		graph.add_library_pass<Passes::BRDF>(bRDF.setup_func, bRDF.render_func);
	}
};

}
