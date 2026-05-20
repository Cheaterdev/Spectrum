#pragma once
#include "../PassNodeBase.h"

using namespace FrameGraph;
namespace Passes
{

class Voxelize : public PassNodeBase
{
public:
	struct Context
	{


		Handlers::Texture H(VoxelAlbedo);


		Handlers::Texture H(VoxelNormal);


		Handlers::Texture H(VoxelAlbedoStatic);


		Handlers::Texture H(VoxelNormalStatic);


		Handlers::Texture H(VoxelAlbedoDynamic);


		Handlers::Texture H(VoxelNormalDynamic);

		static inline const wchar_t* const resource_names[] = {		L"VoxelAlbedo",		L"VoxelNormal",		L"VoxelAlbedoStatic",		L"VoxelNormalStatic",		L"VoxelAlbedoDynamic",		L"VoxelNormalDynamic",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"Voxelize";

//	static constexpr PassID ID = PassID::Voxelize;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	const FrameGraph::PassFlags flags = FrameGraph::PassFlags::General;
};

}
