#include "pch_render.h"
#include "DynamicTileGenerator.h"


bool TileDynamicGenerator::make_tile(uint3 pos)
{
	auto &v = all_tiles[pos];
	
	if (!v)
	{
		tiles_to_load.emplace_back(pos);
	}

	if(v!= current_counter)
		dynamic_tiles.emplace_back(pos);

	v = current_counter;
	
	return v;
}

void TileDynamicGenerator::remove_all()
{
	for (auto& e : all_tiles)
	{
		e = 0;
	}

	current_counter = 0;

	dynamic_tiles.clear();
	tiles_to_remove.clear();
	tiles_to_load.clear();
	old_tiles.clear();
}

TileDynamicGenerator::TileDynamicGenerator(uint3 tile_count) :tile_count(tile_count)
{
	all_tiles.resize(tile_count);
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
	}

void TileDynamicGenerator::end()
{
	for (auto e : old_tiles)
	{
		if (all_tiles[e] != current_counter)
		{
			tiles_to_remove.emplace_back(e);
			all_tiles[e] = 0;
		}
	}
}
