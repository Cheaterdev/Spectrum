
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class BlueNoise 
{
public:
	struct Context
	{

		Handlers::Texture H(BlueNoise);

	};


	static inline const wchar_t* Name = L"BlueNoise";
//	static constexpr PassID ID = PassID::BlueNoise;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}