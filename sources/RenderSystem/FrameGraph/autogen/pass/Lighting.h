
using namespace FrameGraph;

namespace Passes
{

class Lighting 
{
public:
	struct Context
	{

		Handlers::Texture H(global_depth);

		Handlers::StructuredBuffer<Table::Camera> H(global_camera);


		Handlers::Texture3D H(VoxelLighted);


		Handlers::Texture3D H(VoxelAlbedo);


		Handlers::Texture3D H(VoxelNormal);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::Texture H(VoxelAlbedoStatic);


		Handlers::Texture H(VoxelNormalStatic);


		Handlers::Texture H(VoxelAlbedoDynamic);


		Handlers::Texture H(VoxelNormalDynamic);

	};


	static inline const wchar_t* Name = L"Lighting";

//	static constexpr PassID ID = PassID::Lighting;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}