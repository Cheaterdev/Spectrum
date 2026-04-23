#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class Scene 
{
public:
	struct Context
	{
		GBuffer gbuffer;

		Handlers::StructuredBuffer<uint> H(scene);

	};


	static inline const wchar_t* Name = L"Scene";

//	static constexpr PassID ID = PassID::Scene;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}