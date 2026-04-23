
using namespace FrameGraph;

namespace Passes
{

class Mipmapping 
{
public:
	struct Context
	{

		Handlers::Texture3D H(VoxelLighted);

	};


	static inline const wchar_t* Name = L"Mipmapping";

//	static constexpr PassID ID = PassID::Mipmapping;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}