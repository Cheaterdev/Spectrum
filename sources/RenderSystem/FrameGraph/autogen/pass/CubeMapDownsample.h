
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class CubeMapDownsample 
{
public:
	struct Context
	{

		Handlers::TextureCube H(sky_cubemap);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::TextureCube H(sky_cubemap_filtered_diffuse);

	};


	static inline const wchar_t* Name = L"CubeMapDownsample";
//	static constexpr PassID ID = PassID::CubeMapDownsample;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}