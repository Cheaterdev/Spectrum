export module Graphics:TileDynamicGenerator;

import Core;
export {

	class TileDynamicGenerator
	{
		uint3 tile_count;
		vec3 min, max;

		// World-space tile extent. MUST stay float: scene_size/tile_count is
		// fractional (e.g. 200/16 = 12.5) and integer truncation makes the
		// tile ranges in add() drift with distance from the scene min —
		// dynamic objects far from the origin mark the wrong tiles.
		vec3 one_tile_size;
		grid<uint3, uint64> all_tiles;
		std::list<uint3> old_tiles;

		bool make_tile(uint3 pos);
		uint64 current_counter = 0;
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
}