
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class PreScene 
{
public:
	struct Context
	{

		Handlers::StructuredBuffer<uint> H(scene);

	};


	static inline const wchar_t* Name = L"PreScene";
//	static constexpr PassID ID = PassID::PreScene;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}