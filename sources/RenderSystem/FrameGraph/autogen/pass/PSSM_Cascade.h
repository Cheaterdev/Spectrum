#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class PSSM_Cascade : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(PSSM_Depths);

		Handlers::StructuredBuffer<Table::Camera> H(PSSM_Cameras);

		static inline const wchar_t* const resource_names[] = {		L"PSSM_Depths",		L"PSSM_Cameras",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"PSSM_Cascade";

	static constexpr uint32_t MaxCount = 6;
	static inline const wchar_t* Names[MaxCount] = {
		L"PSSM_Cascade_0",
		L"PSSM_Cascade_1",
		L"PSSM_Cascade_2",
		L"PSSM_Cascade_3",
		L"PSSM_Cascade_4",
		L"PSSM_Cascade_5",
	};

//	static constexpr PassID ID = PassID::PSSM_Cascade;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	std::array<setup_func_type, MaxCount> setup_funcs;
	std::array<render_func_type, MaxCount> render_funcs;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
