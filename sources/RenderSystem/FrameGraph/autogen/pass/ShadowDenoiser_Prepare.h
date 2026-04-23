
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class ShadowDenoiser_Prepare 
{
public:
	struct Context
	{

		Handlers::Texture H(RTXDebug);


		Handlers::StructuredBuffer<uint> H(ShadowDenoiser_TileBuffer);

	};


	static inline const wchar_t* Name = L"ShadowDenoiser_Prepare";

//	static constexpr PassID ID = PassID::ShadowDenoiser_Prepare;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}