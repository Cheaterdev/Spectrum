module Graphics:BlueNoise;
import <RenderSystem.h>;

import HAL;

using namespace HAL;
using namespace FrameGraph;

namespace _1spp
{
#include "samplerCPP/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp.h"
}

struct
{
	std::span<std::int32_t> sobolBuffer;
	std::span<std::int32_t> rankingTileBuffer;
	std::span<std::int32_t> scramblingTileBuffer;
}
const g_blueNoiseSamplerState =
{
	const_cast<std::remove_const<decltype(_1spp::sobol_256spp_256d)>::type&>(_1spp::sobol_256spp_256d),
	const_cast<std::remove_const<decltype(_1spp::rankingTile)>::type&>(_1spp::rankingTile),
	const_cast<std::remove_const<decltype(_1spp::scramblingTile)>::type&>(_1spp::scramblingTile)
};


BlueNoise::BlueNoise()
{
	LinearAllocator allocator;

	auto sobol_handle     = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.sobolBuffer.size_bytes());
	auto ranking_handle   = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.rankingTileBuffer.size_bytes());
	auto scrambling_handle = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.scramblingTileBuffer.size_bytes());

	buffer = std::make_shared<HAL::Buffer>(HAL::ResourceDesc::Buffer(allocator.get_max_usage()), HAL::HeapType::DEFAULT);

	sobol_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			sobol_handle.get_offset(),
			sobol_handle.get_size(),
			counterType::NONE
		});

	ranking_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			ranking_handle.get_offset(),
			ranking_handle.get_size(),
			counterType::NONE
		});

	scrambling_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			scrambling_handle.get_offset(),
			scrambling_handle.get_size(),
			counterType::NONE
		});

	auto list = HAL::Device::get().get_upload_list();
	list->get_copy().update(sobol_buffer_view,     0, g_blueNoiseSamplerState.sobolBuffer);
	list->get_copy().update(ranking_buffer_view,   0, g_blueNoiseSamplerState.rankingTileBuffer);
	list->get_copy().update(scrambling_buffer_view, 0, g_blueNoiseSamplerState.scramblingTileBuffer);

	list->execute_and_wait();
}
