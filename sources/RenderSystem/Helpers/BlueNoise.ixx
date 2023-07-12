export module Graphics:BlueNoise;

import :BinaryAsset;
import :Asset;
import FrameGraph;


export class BlueNoise:public FrameGraph::GraphGenerator
{
	HAL::Resource::ptr buffer;
	HAL::StructuredBufferView<int32_t>						sobol_buffer_view;
	HAL::StructuredBufferView<int32_t>						ranking_buffer_view;
	HAL::StructuredBufferView<int32_t>						scrambling_buffer_view;
	
public:

	virtual void generate(FrameGraph::Graph& graph) override;


	BlueNoise();

};
