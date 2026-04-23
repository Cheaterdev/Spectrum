
using namespace FrameGraph;

namespace Passes
{

class CubeSky 
{
public:
	struct Context
	{

		Handlers::TextureCube H(sky_cubemap);

	};


	static inline const wchar_t* Name = L"CubeSky";

//	static constexpr PassID ID = PassID::CubeSky;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}