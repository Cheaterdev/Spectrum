
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;

#include "GBuffer.h"

using namespace FrameGraph;

namespace Passes
{

class GBufferDownsampler 
{
public:
	struct Context
	{
		GBuffer gbuffer;
	};


	static inline const wchar_t* Name = L"GBufferDownsampler";

//	static constexpr PassID ID = PassID::GBufferDownsampler;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}