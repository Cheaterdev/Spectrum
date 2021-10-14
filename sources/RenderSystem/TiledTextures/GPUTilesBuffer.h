#pragma once
#include "DX12/Buffer.h"
import SIG;

class GPUTilesBuffer
{
	bool tiles_updated = false;
	
	grid<ivec3, int> tile_positions;
	ivec3 shape;
public:

	std::vector<ivec3> used_tiles;
	using ptr = std::shared_ptr<GPUTilesBuffer>;
	Render::StructuredBuffer<ivec3>::ptr buffer;
	Render::StructuredBuffer<DispatchArguments>::ptr dispatch_buffer;
	uint32_t size();
	void set_size(ivec3 size, ivec3 shape);

	void insert(ivec3 pos);
	void erase(ivec3 pos);
	void clear();

	void update(Render::CommandList::ptr list);
};