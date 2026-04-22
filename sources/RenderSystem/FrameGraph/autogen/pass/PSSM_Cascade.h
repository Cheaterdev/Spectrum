
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class PSSM_Cascade 
{
public:
	struct Context
	{

		Handlers::Texture H(PSSM_Depths);

		Handlers::StructuredBuffer<Table::Camera> H(PSSM_Cameras);

	};


	static inline const wchar_t* Name = L"PSSM_Cascade";
//	static constexpr PassID ID = PassID::PSSM_Cascade;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}