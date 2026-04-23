
using namespace FrameGraph;

namespace Passes
{

class SMAA 
{
public:
	struct Context
	{

		Handlers::Texture H(ResultTexture);

		Handlers::Texture ResultTextureNew = ResultTexture;

		Handlers::Texture H(SMAA_edges);


		Handlers::Texture H(SMAA_blend);

	};


	static inline const wchar_t* Name = L"SMAA";

//	static constexpr PassID ID = PassID::SMAA;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}