export module Graphics:GPUTilesBuffer;

import Core;
import HAL;

export {

	class GPUTilesBuffer
	{
		bool tiles_updated = false;

		grid<ivec3, int> tile_positions;
		ivec3 shape;
	public:

		std::vector<ivec3> used_tiles;
		using ptr = std::shared_ptr<GPUTilesBuffer>;
		HAL::StructuredBufferView<ivec3> buffer;
		HAL::StructuredBufferView<DispatchArguments> dispatch_buffer;
		uint32_t size();
		void set_size(ivec3 size, ivec3 shape);

		void insert(ivec3 pos);
		void erase(ivec3 pos);
		void clear();

		void update(HAL::CommandList::ptr list);
	};

}