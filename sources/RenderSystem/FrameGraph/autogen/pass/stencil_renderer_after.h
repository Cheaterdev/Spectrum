
using namespace FrameGraph;

namespace Passes
{

class stencil_renderer_after 
{
public:
	struct Context
	{

		Handlers::Texture H(ResultTexture);


		Handlers::Texture H(Stencil_color_tex);

	};


	static inline const wchar_t* Name = L"stencil_renderer_after";

//	static constexpr PassID ID = PassID::stencil_renderer_after;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}