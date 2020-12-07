#pragma once

class GPUTilesBuffer
{
	bool tiles_updated = false;
	std::mutex tile_mutex;
	std::vector<ivec3> used_tiles;

public:
	using ptr = std::shared_ptr<GPUTilesBuffer>;
	void set_size(uint32_t max_count);
	Render::StructuredBuffer<ivec3>::ptr buffer;

	void clear();

	uint32_t size();

	void insert(ivec3 pos);

	void erase(ivec3 pos);

	
	void update(Render::CommandList::ptr list);

};