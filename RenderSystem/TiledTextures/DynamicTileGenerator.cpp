#include "pch.h"



bool TileDynamicGenerator::make_tile(ivec3 pos)
{
	int index = pos.x + pos.y * tile_count.x + pos.z *tile_count.x * tile_count.y;
	auto &v = all_tiles[index];


	
	if (!v)
	{
		// .size();
		tiles_to_load.emplace(index);
	}

	if(v!= current_counter)
		dynamic_tiles.emplace(index);

	v = current_counter;
	
	return v;
}

void TileDynamicGenerator::remove_all()
{
	for (auto& e : all_tiles)
	{
		e = 0;
	//	all_tiles[e] = false;
	}
	current_counter = 0;

	dynamic_tiles.clear();
	tiles_to_remove.clear();
	tiles_to_load.clear();
	old_tiles.clear();
}

TileDynamicGenerator::TileDynamicGenerator(ivec3 tile_count) :tile_count(tile_count)
{
	all_tiles.resize(tile_count.x*tile_count.y* tile_count.z, false);
}

ivec3 TileDynamicGenerator::get_pos(int i)
{
	int x = i % tile_count.x;
	int z = i / (tile_count.y*tile_count.x);
	int y = ((i - x) / tile_count.x) % tile_count.y;

	return{ x,y,z };
}

void TileDynamicGenerator::set_scene_volume(vec3 min, vec3 max)
{
	this->min = min;
	this->max = max;

	one_tile_size = (max - min) / tile_count;
}

void TileDynamicGenerator::begin(vec3 min, vec3 max)
{
	current_counter++;
	set_scene_volume(min, max);
	//	memset(&all_tiles.front(), 0, all_tiles.size());

//	for (auto& e : dynamic_tiles)
//	{
//		all_tiles[e] = false;
//	}

	std::swap(old_tiles, dynamic_tiles);

	dynamic_tiles.clear();
	tiles_to_remove.clear();
	tiles_to_load.clear();
}

void TileDynamicGenerator::add(vec3 min, vec3 max)
{
	ivec3 from = (min - this->min) / one_tile_size;
	ivec3 to = (max - this->min + one_tile_size ) / one_tile_size;


	for (int x = from.x; x < to.x; x++)
		for (int y = from.y; y < to.y; y++)
			for (int z = from.z; z < to.z; z++)
			{
				make_tile({ x,y,z });
			}

//	Log::get() << "count " << (to.x - from.x) << " " << (to.y - from.y) << " " << (to.z - from.z) << Log::endl;
}

void TileDynamicGenerator::end()
{
	/*for (auto e : dynamic_tiles)
	{
		if (old_tiles.find(e) == old_tiles.end())
			tiles_to_load.emplace(e);
	}

	*/
	/*for (auto e : old_tiles)
	{
		if (dynamic_tiles.find(e) == dynamic_tiles.end())
			tiles_to_remove.emplace(e);
	}*/

	for (auto e : old_tiles)
	{
		if (all_tiles[e] != current_counter)
		{
			tiles_to_remove.emplace(e);
			all_tiles[e] = 0;
		}
	}
}
