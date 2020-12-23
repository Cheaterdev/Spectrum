#pragma once

class GPUTilesBuffer
{
	bool tiles_updated = false;
	std::vector<ivec3> used_tiles;
	grid<ivec3, int> tile_positions;
public:
	using ptr = std::shared_ptr<GPUTilesBuffer>;
	Render::StructuredBuffer<ivec3>::ptr buffer;

	uint32_t size();
	void set_size(ivec3 size);

	void insert(ivec3 pos);
	void erase(ivec3 pos);
	void clear();

	void update(Render::CommandList::ptr list);
};