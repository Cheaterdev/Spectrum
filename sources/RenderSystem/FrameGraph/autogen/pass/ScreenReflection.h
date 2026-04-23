#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class ScreenReflection 
{
public:
	struct Context
	{
		GBuffer gbuffer;

		Handlers::Texture H(VoxelReflectionNoise);


		Handlers::Texture H(noise_dir_pdf);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::Texture H(BlueNoise);


		Handlers::Texture3D H(VoxelLighted);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_hi);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_low);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_low_data);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_hi_data);

	};


	static inline const wchar_t* Name = L"ScreenReflection";

//	static constexpr PassID ID = PassID::ScreenReflection;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}