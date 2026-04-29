#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class CubeMapDownsample : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::TextureCube H(sky_cubemap);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::TextureCube H(sky_cubemap_filtered_diffuse);

		static inline const wchar_t* const resource_names[] = {		L"sky_cubemap",		L"sky_cubemap_filtered",		L"sky_cubemap_filtered_diffuse",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"CubeMapDownsample";

//	static constexpr PassID ID = PassID::CubeMapDownsample;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
