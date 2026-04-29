#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class ShadowDenoiser_Filter : public PassNodeBase
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

		static inline const wchar_t* const resource_names[] = {		L"ShadowDenoiser_TileMetaBuffer",		L"RTXDebug",		L"GBuffer_Depth",		L"GBuffer_Normals",		L"ShadowDenoiser_Scratch",		L"ShadowDenoiser_Scratch2",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"ShadowDenoiser_Filter";

//	static constexpr PassID ID = PassID::ShadowDenoiser_Filter;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
