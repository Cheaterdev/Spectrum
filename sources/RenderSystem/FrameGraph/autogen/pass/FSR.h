
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class FSR 
{
public:
	struct Context
	{

		Handlers::Texture H(ResultTexture);

		Handlers::Texture ResultTextureNew = ResultTexture;

		Handlers::Texture H(FSRTemp);

	};


	static inline const wchar_t* Name = L"FSR";
//	static constexpr PassID ID = PassID::FSR;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}