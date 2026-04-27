export module Graphics:BlueNoise;

import <RenderSystem.h>;
import :BinaryAsset;
import :Asset;
import FrameGraph;


export class BlueNoise
{
	HAL::Buffer::ptr buffer;
	HAL::StructuredBufferView<int32_t> sobol_buffer_view;
	HAL::StructuredBufferView<int32_t> ranking_buffer_view;
	HAL::StructuredBufferView<int32_t> scrambling_buffer_view;

public:
	// Default constructor: uploads the blue-noise LUT tables to GPU memory.
	// Defined in BlueNoise.cpp so the large sampler headers stay out of the
	// module interface.
	BlueNoise();

	// Template constructor: wires setup/render funcs onto the matching pass
	// in any pipeline that exposes a 'blueNoise' member of type Passes::BlueNoise.
	// Delegates to BlueNoise() first so the GPU buffers are always ready.
	template<typename TPipeline>
	explicit BlueNoise(TPipeline& pipeline) : BlueNoise()
	{
		pipeline.blueNoise.flags = FrameGraph::PassFlags::Compute;

		pipeline.blueNoise.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			builder.create(data.BlueNoise,
				{ ivec3(128, 128, 0), HAL::Format::R8G8_UNORM, 1, 1 },
				FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
			return true;
		};

		pipeline.blueNoise.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& compute = context.get_list()->get_compute();
			compute.set_pipeline<PSOS::BlueNoise>();

			static uint index = 0;
			Slots::BlueNoise blue_data;
			blue_data.GetFrame_index()            = index++;
			blue_data.GetSobol_buffer()           = HLSL::Buffer<uint>(sobol_buffer_view.structuredBuffer);
			blue_data.GetRanking_tile_buffer()    = HLSL::Buffer<uint>(ranking_buffer_view.structuredBuffer);
			blue_data.GetScrambling_tile_buffer() = HLSL::Buffer<uint>(scrambling_buffer_view.structuredBuffer);
			blue_data.GetBlue_noise_texture()     = data.BlueNoise->rwTexture2D;
			compute.set(blue_data);

			compute.dispatch(data.BlueNoise->get_size());
		};
	}
};
