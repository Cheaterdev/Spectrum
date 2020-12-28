#pragma once

class TileDynamicGenerator
{
	ivec3 tile_count;
	vec3 min, max;

	vec3 one_tile_size;
	grid<ivec3, UINT64> all_tiles;
	std::list<ivec3> old_tiles;

	bool make_tile(ivec3 pos);
	UINT64 current_counter = 0;
public:

	std::list<ivec3> tiles_to_remove;
	std::list<ivec3> tiles_to_load;
	std::list<ivec3> dynamic_tiles;

	void remove_all();
	TileDynamicGenerator() = default;
	TileDynamicGenerator(ivec3 tile_count);
	void set_scene_volume(vec3 min, vec3 max);
	void begin(vec3 min, vec3 max);
	void add(vec3 min, vec3 max);
	void end();
};