export module Graphics:BlueNoise;


import :BinaryAsset;
import :Asset;
import FrameGraph;


export class BlueNoise
{
	HAL::Buffer::ptr buffer;
	HAL::StructuredBufferView<int32_t> sobol_buffer_view;
	HAL::StructuredBufferView<int32_t> ranking_buffer_view;
	HAL::StructuredBufferView<int32_t> scrambling_buffer_view;

	// setup() is generated (BlueNoise.sig's own [RunAlways]) -- render only.
	Passes::BlueNoise::render_func_type m_bluenoise_render;

public:
	// Default constructor: uploads the blue-noise LUT tables to GPU memory and
	// initialises the render function member.
	// Defined in BlueNoise.cpp so the large sampler headers stay out of the
	// module interface.
	BlueNoise();

	// Template constructor: wires the render func onto the matching pass
	// in any pipeline that exposes a 'blueNoise' member of type Passes::BlueNoise.
	// Delegates to BlueNoise() so the GPU buffers and function members are ready.
	template<typename TPipeline>
	explicit BlueNoise(TPipeline& pipeline) : BlueNoise()
	{
		pipeline.blueNoise.render_func = m_bluenoise_render;
	}
};
