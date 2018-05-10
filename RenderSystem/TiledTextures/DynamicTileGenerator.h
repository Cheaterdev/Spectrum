#pragma once

class TileDynamicGenerator
{
	ivec3 tile_count;
	vec3 min, max;

	vec3 one_tile_size;
	std::vector<UINT64> all_tiles;
	std::set<int> old_tiles;



	bool make_tile(ivec3 pos);
	UINT64 current_counter = 0;
public:

	std::set<int> tiles_to_remove;
	std::set<int> tiles_to_load;
	std::set<int> dynamic_tiles;

	void remove_all();
	TileDynamicGenerator() = default;
	TileDynamicGenerator(ivec3 tile_count);

	ivec3 get_pos(int i);

	void set_scene_volume(vec3 min, vec3 max);

	void begin(vec3 min, vec3 max);


	void add(vec3 min, vec3 max);

	void end();
};