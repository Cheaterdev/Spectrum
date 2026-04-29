#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class Sky : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(GBuffer_Depth);


		Handlers::Texture H(ResultTexture);

		static inline const wchar_t* const resource_names[] = {		L"GBuffer_Depth",		L"ResultTexture",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"Sky";

//	static constexpr PassID ID = PassID::Sky;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
