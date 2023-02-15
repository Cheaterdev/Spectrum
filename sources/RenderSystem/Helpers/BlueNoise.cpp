module Graphics:BlueNoise;
import HAL;

using namespace HAL;
using namespace FrameGraph;
namespace _1spp
{
#include "samplerCPP/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp.h"
}

struct
{
	std::span<const std::int32_t> sobolBuffer;
	std::span<const std::int32_t> rankingTileBuffer;
	std::span<const std::int32_t> scramblingTileBuffer;
}
const g_blueNoiseSamplerState = { _1spp::sobol_256spp_256d,  _1spp::rankingTile,  _1spp::scramblingTile };



BlueNoise::BlueNoise()
{
	LinearAllocator allocator;

	auto sobol_handle = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.sobolBuffer.size_bytes());
	auto ranking_handle = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.rankingTileBuffer.size_bytes());
	auto scrambling_handle = allocator.Allocate<std::int32_t>(g_blueNoiseSamplerState.scramblingTileBuffer.size_bytes());


	buffer = std::make_shared<HAL::Resource>(HAL::ResourceDesc::Buffer(allocator.get_max_usage()), HAL::HeapType::DEFAULT);





	sobol_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			sobol_handle.get_offset(),
			sobol_handle.get_size(),
			false
		});


	ranking_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			ranking_handle.get_offset(),
			ranking_handle.get_size(),
			false
		});


	scrambling_buffer_view = buffer->create_view<HAL::StructuredBufferView<int32_t>>(
		HAL::Device::get().get_static_gpu_data(),
		HAL::StructuredBufferViewDesc{
			scrambling_handle.get_offset(),
			scrambling_handle.get_size(),
			false
		});



	auto list = (HAL::Device::get().get_upload_list());
	list->get_copy().update(sobol_buffer_view, 0, g_blueNoiseSamplerState.sobolBuffer);
	list->get_copy().update(ranking_buffer_view, 0, g_blueNoiseSamplerState.rankingTileBuffer);
	list->get_copy().update(scrambling_buffer_view, 0, g_blueNoiseSamplerState.scramblingTileBuffer);

	list->execute_and_wait();

}

void BlueNoise::generate(FrameGraph::Graph& graph)
{

	struct BlueNoiseData
	{
		Handlers::Texture H(BlueNoise);
	};

	graph.add_pass<BlueNoiseData>("BlueNoise", [this, &graph](auto& data, auto& builder) {
		builder.create(data.BlueNoise, { ivec3(128,128,0),  HAL::Format::R8G8_UNORM, 1 ,1 }, ResourceFlags::UnorderedAccess);

		}, [this, &graph](auto& data, auto& _context) {
			auto& list = *_context.get_list();

			auto& compute = list.get_compute();

			compute.set_pipeline<PSOS::BlueNoise>();
			static size_t index = 0;

			{
				Slots::BlueNoise blue_data;
				blue_data.GetFrame_index() = index++;
				blue_data.GetSobol_buffer() = HLSL::Buffer<uint>(sobol_buffer_view.structuredBuffer);
				blue_data.GetRanking_tile_buffer() = HLSL::Buffer<uint>(ranking_buffer_view.structuredBuffer);
				blue_data.GetScrambling_tile_buffer() = HLSL::Buffer<uint>(scrambling_buffer_view.structuredBuffer);

				blue_data.GetBlue_noise_texture() = data.BlueNoise->rwTexture2D;
				blue_data.set(compute);
			}

			compute.dispach(data.BlueNoise->get_size());

		}, PassFlags::Compute);
}