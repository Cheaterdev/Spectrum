#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class stencil_renderer_before : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(depth_tex);


		Handlers::StructuredBuffer<UINT> H(id_buffer);


		Handlers::StructuredBuffer<UINT> H(axis_id_buffer);

		static inline const wchar_t* const resource_names[] = {		L"depth_tex",		L"id_buffer",		L"axis_id_buffer",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"stencil_renderer_before";

//	static constexpr PassID ID = PassID::stencil_renderer_before;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
