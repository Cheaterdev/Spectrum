#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class RTXPass 
{
public:
	struct Context
	{
		GBuffer gbuffer;

		Handlers::Texture H(RTXDebug);


		Handlers::ByteAdressBuffer H(WorkGraphBuffer);

	};


	static inline const wchar_t* Name = L"RTXPass";

//	static constexpr PassID ID = PassID::RTXPass;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}