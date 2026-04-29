#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class Profiler : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(swapchain);

		static inline const wchar_t* const resource_names[] = {		L"swapchain",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"Profiler";

//	static constexpr PassID ID = PassID::Profiler;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
