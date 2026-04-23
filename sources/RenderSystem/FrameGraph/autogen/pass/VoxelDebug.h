
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;

#include "GBuffer.h"

using namespace FrameGraph;

namespace Passes
{

class VoxelDebug 
{
public:
	struct Context
	{
		GBuffer gbuffer;

		Handlers::Texture H(VoxelDebug);


		Handlers::Texture3D H(VoxelLighted);

	};


	static inline const wchar_t* Name = L"VoxelDebug";

//	static constexpr PassID ID = PassID::VoxelDebug;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}