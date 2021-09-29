#pragma once
#include "Math/Math.h"

class TileDynamicGenerator
{
	uint3 tile_count;
	vec3 min, max;

	uint3 one_tile_size;
	grid<uint3, UINT64> all_tiles;
	std::list<uint3> old_tiles;

	bool make_tile(uint3 pos);
	UINT64 current_counter = 0;
public:

	std::list<uint3> tiles_to_remove;
	std::list<uint3> tiles_to_load;
	std::list<uint3> dynamic_tiles;

	void remove_all();
	TileDynamicGenerator() = default;
	TileDynamicGenerator(uint3 tile_count);
	void set_scene_volume(vec3 min, vec3 max);
	void begin(vec3 min, vec3 max);
	void add(vec3 min, vec3 max);
	void end();
};