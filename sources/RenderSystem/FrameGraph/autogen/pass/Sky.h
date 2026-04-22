
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class Sky 
{
public:
	struct Context
	{

		Handlers::Texture H(GBuffer_Depth);


		Handlers::Texture H(ResultTexture);

	};


	static inline const wchar_t* Name = L"Sky";
//	static constexpr PassID ID = PassID::Sky;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}