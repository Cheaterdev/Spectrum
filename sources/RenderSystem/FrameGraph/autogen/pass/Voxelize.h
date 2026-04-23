
using namespace FrameGraph;

namespace Passes
{

class Voxelize 
{
public:
	struct Context
	{

		Handlers::Texture H(VoxelAlbedo);


		Handlers::Texture H(VoxelNormal);


		Handlers::Texture H(VoxelAlbedoStatic);


		Handlers::Texture H(VoxelNormalStatic);


		Handlers::Texture H(VoxelAlbedoDynamic);


		Handlers::Texture H(VoxelNormalDynamic);

	};


	static inline const wchar_t* Name = L"Voxelize";

//	static constexpr PassID ID = PassID::Voxelize;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}