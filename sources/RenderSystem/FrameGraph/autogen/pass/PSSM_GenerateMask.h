#pragma once
#include "GBuffer.h"
using namespace FrameGraph;

namespace Passes
{

class PSSM_GenerateMask 
{
public:
	struct Context
	{

		Handlers::Texture H(PSSM_Depths);

		Handlers::StructuredBuffer<Table::Camera> H(PSSM_Cameras);

		GBuffer gbuffer;

		Handlers::Texture H(LightMask);

	};


	static inline const wchar_t* Name = L"PSSM_GenerateMask";

//	static constexpr PassID ID = PassID::PSSM_GenerateMask;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}