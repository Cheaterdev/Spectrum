
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class ResultCreation 
{
public:
	struct Context
	{

		Handlers::Texture H(ResultTexture);

	};


	static inline const wchar_t* Name = L"ResultCreation";
//	static constexpr PassID ID = PassID::ResultCreation;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}