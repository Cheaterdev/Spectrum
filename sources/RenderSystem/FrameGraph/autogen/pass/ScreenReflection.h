#pragma once
#include "../PassNodeBase.h"
#include "GBuffer.h"
using namespace FrameGraph;
namespace Passes
{

class ScreenReflection : public PassNodeBase
{
public:
	struct Context
	{

		GBuffer gbuffer;

		Handlers::Texture H(VoxelReflectionNoise);


		Handlers::Texture H(noise_dir_pdf);


		Handlers::TextureCube H(sky_cubemap_filtered);


		Handlers::Texture H(BlueNoise);


		Handlers::Texture3D H(VoxelLighted);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_hi);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_low);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_low_data);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_hi_data);

		static inline const wchar_t* const resource_names[] = {		L"GBuffer_Albedo",		L"GBuffer_Normals",		L"GBuffer_Depth",		L"GBuffer_Specular",		L"GBuffer_Speed",		L"GBuffer_DepthMips",		L"GBuffer_Quality",		L"GBuffer_TempColor",		L"GBuffer_NormalsPrev",		L"GBuffer_SpecularPrev",		L"GBuffer_DepthPrev",		L"GBuffer_HiZ",		L"GBuffer_HiZ_UAV",		L"VoxelReflectionNoise",		L"noise_dir_pdf",		L"sky_cubemap_filtered",		L"BlueNoise",		L"VoxelLighted",		L"VoxelScreen_hi",		L"VoxelScreen_low",		L"VoxelScreen_low_data",		L"VoxelScreen_hi_data",
		};
		static constexpr uint32_t resource_count = std::size(resource_names);
	};


	std::span<const wchar_t* const> GetUsedResourcesList() const override
	{
		return Context::resource_names;
	}

	static inline const wchar_t* Name = L"ScreenReflection";

//	static constexpr PassID ID = PassID::ScreenReflection;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;


	setup_func_type setup_func;
	render_func_type render_func;

	const FrameGraph::PassFlags flags = FrameGraph::PassFlags::Compute;
};

}
