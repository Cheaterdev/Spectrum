#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class PSSM_Combine 
{
public:
	struct Context
	{
		Handlers::StructuredBuffer<Table::Camera> H(PSSM_Cameras);

		GBuffer gbuffer;

		Handlers::Texture H(LightMask);


		Handlers::Texture H(RTXDebug);


		Handlers::Texture H(ResultTexture);

	};


	static inline const wchar_t* Name = L"PSSM_Combine";

//	static constexpr PassID ID = PassID::PSSM_Combine;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}