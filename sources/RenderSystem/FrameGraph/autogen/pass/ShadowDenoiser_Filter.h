
import FrameGraph;
import Core;
import HAL;

import <HAL.h>;



using namespace FrameGraph;

namespace Passes
{

class ShadowDenoiser_Filter 
{
public:
	struct Context
	{

		Handlers::StructuredBuffer<uint> H(ShadowDenoiser_TileMetaBuffer);


		Handlers::Texture H(RTXDebug);


		Handlers::Texture H(GBuffer_Depth);


		Handlers::Texture H(GBuffer_Normals);


		Handlers::Texture H(ShadowDenoiser_Scratch);


		Handlers::Texture H(ShadowDenoiser_Scratch2);

	};


	static inline const wchar_t* Name = L"ShadowDenoiser_Filter";

//	static constexpr PassID ID = PassID::ShadowDenoiser_Filter;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}