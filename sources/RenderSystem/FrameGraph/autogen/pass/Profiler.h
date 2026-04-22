
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class Profiler 
{
public:
	struct Context
	{

		Handlers::Texture H(swapchain);

	};


	static inline const wchar_t* Name = L"Profiler";
//	static constexpr PassID ID = PassID::Profiler;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}