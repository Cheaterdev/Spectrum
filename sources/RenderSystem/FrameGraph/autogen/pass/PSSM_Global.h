
using namespace FrameGraph;

namespace Passes
{

class PSSM_Global 
{
public:
	struct Context
	{

		Handlers::Texture H(global_depth);

		Handlers::StructuredBuffer<Table::Camera> H(global_camera);

	};


	static inline const wchar_t* Name = L"PSSM_Global";

//	static constexpr PassID ID = PassID::PSSM_Global;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}