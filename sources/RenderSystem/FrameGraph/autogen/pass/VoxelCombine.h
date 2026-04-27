#pragma once
#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class VoxelCombine 
{
public:
	struct Context
	{
		GBuffer gbuffer;

		Handlers::Texture H(ResultTexture);


		Handlers::Texture H(VoxelFramesCount);


		Handlers::Texture H(VoxelIndirectNoise);


		Handlers::Texture H(VoxelIndirectFiltered);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_hi);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_low);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_low_data);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_hi_data);

	};


	static inline const wchar_t* Name = L"VoxelCombine";

//	static constexpr PassID ID = PassID::VoxelCombine;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}