#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class PSSM_Global : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(global_depth);

		Handlers::StructuredBuffer<Table::Camera> H(global_camera);

		static inline const wchar_t* const resource_names[] = {		L"global_depth",		L"global_camera",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"PSSM_Global";

//	static constexpr PassID ID = PassID::PSSM_Global;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	const FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
