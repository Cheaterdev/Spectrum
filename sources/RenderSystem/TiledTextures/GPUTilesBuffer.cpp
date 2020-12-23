#include "pch.h"

void GPUTilesBuffer::set_size(ivec3 size)
{
	tile_positions.resize(size, -1);
	buffer.reset(new Render::StructuredBuffer<ivec3>(size.x * size.y * size.z));
}

void GPUTilesBuffer::clear()
{
	tiles_updated = true;
	used_tiles.clear();
	tile_positions.fill(-1);
}

uint32_t GPUTilesBuffer::size()
{
	return (uint32_t)used_tiles.size();
}

void GPUTilesBuffer::insert(ivec3 pos)
{
	int tile_pos = tile_positions[pos];
	if (tile_pos == -1)
	{
		used_tiles.emplace_back(pos);
		tiles_updated = true;
		tile_positions[pos] = used_tiles.size() - 1;
	}
}

void GPUTilesBuffer::erase(ivec3 pos)
{
	int tile_pos = tile_positions[pos];
	if (tile_pos!=-1)
	{
		used_tiles[tile_pos] = used_tiles.back();
		tile_positions[used_tiles[tile_pos]] = tile_pos;
		used_tiles.resize(used_tiles.size() - 1);
		tile_positions[pos] = -1;
		tiles_updated = true;
	}
}

void GPUTilesBuffer::update(Render::CommandList::ptr list)
{
	if (tiles_updated) {
		buffer->set_data(list, 0, used_tiles);
	}
	tiles_updated = false;
}
