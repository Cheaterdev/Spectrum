#pragma once
#include "../PassNodeBase.h"
#include "GBuffer.h"
using namespace FrameGraph;
namespace Passes
{

class RTXPass : public PassNodeBase
{
public:
	struct Context
	{

		GBuffer gbuffer;

		Handlers::Texture H(RTXDebug);


		Handlers::ByteAdressBuffer H(WorkGraphBuffer);

		static inline const wchar_t* const resource_names[] = {		L"GBuffer_Albedo",		L"GBuffer_Normals",		L"GBuffer_Depth",		L"GBuffer_Specular",		L"GBuffer_Speed",		L"GBuffer_DepthMips",		L"GBuffer_Quality",		L"GBuffer_TempColor",		L"GBuffer_NormalsPrev",		L"GBuffer_SpecularPrev",		L"GBuffer_DepthPrev",		L"GBuffer_HiZ",		L"GBuffer_HiZ_UAV",		L"RTXDebug",		L"WorkGraphBuffer",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"RTXPass";

//	static constexpr PassID ID = PassID::RTXPass;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	const FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;
};

}
