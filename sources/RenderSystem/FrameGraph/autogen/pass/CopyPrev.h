#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class CopyPrev 
{
public:
	struct Context
	{
		GBuffer gbuffer;
	};


	static inline const wchar_t* Name = L"CopyPrev";

//	static constexpr PassID ID = PassID::CopyPrev;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}