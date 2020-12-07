#include "pch.h"



void GPUTilesBuffer::set_size(uint32_t max_count)
{
	buffer.reset(new Render::StructuredBuffer<ivec3>(max_count));
}

void GPUTilesBuffer::clear()
{
	tiles_updated = true;
	used_tiles.clear();
}

uint32_t GPUTilesBuffer::size()
{
	return (uint32_t)used_tiles.size();
}

void GPUTilesBuffer::insert(ivec3 pos)
{
	tiles_updated = true;
	tile_mutex.lock(); //fix this
	auto it = std::find(used_tiles.begin(), used_tiles.end(), pos);
	if (it == used_tiles.end())
		used_tiles.emplace_back(pos);
	else
		Log::get() << "wtf" << Log::endl;
	tile_mutex.unlock();
}

void GPUTilesBuffer::erase(ivec3 pos)
{
	tiles_updated = true;
	tile_mutex.lock(); //fix this
	auto it = std::find(used_tiles.begin(), used_tiles.end(), pos);
	if (it != used_tiles.end())
		used_tiles.erase(it);
	else
		Log::get() << "----------" << Log::endl;
	tile_mutex.unlock();
}


void GPUTilesBuffer::update(Render::CommandList::ptr list)
{
	if (tiles_updated) {
		buffer->set_data(list, 0, used_tiles);
	}
	tiles_updated = false;
}
